#include "nodeService.h"
#include <thread>

using namespace grok;
static std::string gNodeCenterName = "NodeCenter";

struct MsgPackHelper {
    template<typename PB>
    static void Send(Session::Ptr s, nodeService::MsgPack* p, PB& pb) {
        if(pb.SerializeToString(p->mutable_pbdata())) {
            // TODO: asio升级后，可以通过io_context 判断如果是在session绑定的线程
            // 可以直接使用session的writebuf 减少一次拷贝
            std::string data;
            if(p->SerializeToString(&data)) {
                s->write(data.c_str(), data.size());
            }
        }
    }
    static void Send(Session::Ptr s, nodeService::MsgPack* p) {
        // TODO: asio升级后，可以通过io_context 判断如果是在session绑定的线程
        // 可以直接使用session的writebuf 减少一次拷贝
        std::string data;
        if(p->SerializeToString(&data)) {
            s->write(data.c_str(), data.size());
        }
    }
    template<typename PB>
    static std::unique_ptr<PB> Parse(nodeService::MsgPack* p) {
        std::unique_ptr<PB> r = std::make_unique<PB>();
        if(r->ParseFromString(p->pbdata())) {
            return r;
        }
        return nullptr;
    }
};

grok::NodeCenter::SPtr grok::NodeCenter::Create(boost::asio::io_service& iov, boost::asio::ip::tcp::endpoint ep)
{
    auto t = std::thread::hardware_concurrency() * 2 + 1;

    auto sptr = std::make_shared<NodeCenter>();
    sptr->m_net_server = std::make_shared<NetServer>(iov);
    sptr->m_net_server->evClose += delegate(sptr, &NodeCenter::on_closesession);
    sptr->m_net_server->evConnect += delegate(sptr, &NodeCenter::on_newsession);
    sptr->m_net_server->evMsg += delegate(sptr, &NodeCenter::on_puremsg);


    sptr->m_net_server->start(ep, t);
    return sptr;
}

void grok::NodeCenter::on_newsession(Session::Ptr s)
{
    // 新连接
    // TODO: 启动一个定时，超时后还未进行register，那么就主动给他踢掉
}

void grok::NodeCenter::on_closesession(Session::Ptr s, boost::system::error_code ec)
{
    std::string name;
    auto key = s->to_string();
    {
        auto r = m_data.readGuard();
        auto it = r->key2name.find(key);
        if (it == r->key2name.end()) {
            // 还未注册过呢
            return;
        }
        name = it->second;
    }
    if(name.empty()) {
        auto w = m_data.writeGuard();
        auto it = w->name2node.find(name);
        if(it == w->name2node.end()) {
            // 相同的ip,port 快速注册和close了多次？
            return;
        }
        // 找到了node，也要看看token是否一致，以防同一个ip,port上一次的close把新的connect给删除了
        if(it->second.s->m_token != s->m_token) {
            return;
        }
        assert(it->second.s == s);
        w->key2name.erase(key);
        w->name2node.erase(it);
    }
}

void grok::NodeCenter::on_puremsg(Session::Ptr s, const char *d, std::size_t n)
{
    std::unique_ptr<nodeService::MsgPack> pack = std::make_unique<nodeService::MsgPack>();
    if(!pack->ParseFromArray(d, n)) {
        // 格式不对
        return ;
    }
    if(pack->msgid() <= 0) {
        return ;
    }

    if(pack->msgid() == NODE_SERVICE_REGISTER) {
        on_registernode(pack.get(), s);
        return;
    }

    on_nodemsg(pack.get(), s);
    return;
}

void grok::NodeCenter::on_registernode(nodeService::MsgPack *p, Session::Ptr s)
{
    nodeService::RspNodeRegistor rsp;
    rsp.set_status(nodeService::eRegister_OK);
    if(p->source().empty()) {
        // 参数错误
        rsp.set_status(nodeService::eRegister_ParamErr);
        p->set_msgtype(nodeService::eMsg_response);
        MsgPackHelper::Send(s, p, rsp);
        return;
    }

    if(p->source() == gNodeCenterName) {
        // 参数错误
        rsp.set_status(nodeService::eRegister_ParamErr);
        p->set_msgtype(nodeService::eMsg_response);
        MsgPackHelper::Send(s, p, rsp);
        return;
    }

    NodeCtx ctx;
    ctx.name = p->source();
    ctx.s = s;

    Session::Ptr s_old;
    {
        auto w = m_data.writeGuard();
        auto it = w->name2node.find(p->source());
        if (it != w->name2node.end()) {
            // 后面注册的会直接将前面的覆盖掉！
            // 交换session
            s_old = it->second.s;
            it->second.s = s;
            w->key2name.erase(s_old->to_string());
            w->key2name[s->to_string()] = ctx.name;
        } else {
            w->key2name[s->to_string()] = ctx.name;
            w->name2node[ctx.name] = ctx;
        }
    }
    
    // 旧注册的session干掉！
    if(s_old) {
        s_old->stop();
    }

    // 这里，还需要对注册的client进行返回成功的消息
    MsgPackHelper::Send(s, p, rsp);
}

void grok::NodeCenter::on_nodemsg(nodeService::MsgPack *p, Session::Ptr s)
{
    if(p->msgtype() == nodeService::eMsg_response) {
        // 找到
        auto source_s = get_session(p->source());
        if(!source_s) {
            // response的节点已经挂了
            return;
        }
        MsgPackHelper::Send(source_s, p);
        return;
    }

    // 转发消息
    auto dest_s = get_session(p->dest());
    if(!dest_s) {
        return;
    }
    MsgPackHelper::Send(dest_s, p);
}

Session::Ptr grok::NodeCenter::get_session(const std::string& name)
{
    auto r = m_data.readGuard();
    auto it = r->name2node.find(name);
    if(it != r->name2node.end()) {
        return it->second.s;
    }
    return nullptr;
}

grok::NodeClient::SPtr grok::NodeClient::Create(boost::asio::io_service &iosvr, boost::asio::ip::tcp::endpoint ep, std::string name)
{
    SPtr r = std::make_shared<NodeClient>();
    r->m_ctx.s = Session::Connect(iosvr, ep);
    if(!r->m_ctx.s) {
        return nullptr;
    }
    r->m_ctx.s->evMsg += delegate(r, &NodeClient::on_puremsg);
    r->m_ctx.name = name;
    r->m_ctx.s->start();
    // 发送注册
    nodeService::MsgPack pack;
    pack.set_msgid(NODE_SERVICE_REGISTER);
    pack.set_source(name);
    pack.set_msgtype(nodeService::eMsg_request);
    MsgPackHelper::Send(r->m_ctx.s, &pack);
    return r;
}

void grok::NodeClient::register_msgid(std::int32_t msgid, MsgOpCall &&cb)
{
    auto w = m_data.writeGuard();
    w->m_msg_opmap[msgid] = std::move(cb);
}

Session::Ptr grok::NodeClient::get_session()
{
    return m_ctx.s;
}

void grok::NodeClient::send_notify(std::int32_t msgid, const std::string &dest, const char *data, std::size_t n)
{
    std::shared_ptr<nodeService::MsgPack> p = std::make_shared<nodeService::MsgPack>();
    p->set_dest(dest);
    p->mutable_pbdata()->append(data, n);
    p->set_msgid(msgid);
    p->set_msgtype(nodeService::eMsg_notify);
    //TODO: 当lambda支持move uniqeptr时可以修改,可以把shared_ptr改成uniqe_ptr

    auto self = this->shared_from_this();
    dispatch([p, self, this](){
        p->set_source(m_ctx.name);
        MsgPackHelper::Send(m_ctx.s, p.get());
    });
}

void grok::NodeClient::send_request(std::int32_t msgid, const std::string &dest, const char *data, std::size_t n)
{
    std::shared_ptr<nodeService::MsgPack> p = std::make_shared<nodeService::MsgPack>();
    p->set_dest(dest);
    p->mutable_pbdata()->append(data, n);
    p->set_msgid(msgid);
    p->set_msgtype(nodeService::eMsg_request);
    p->set_sessionid(msg_msgnextidx());
    //TODO: 当lambda支持move uniqeptr时可以修改,可以把shared_ptr改成uniqe_ptr

    auto self = shared_from_this();
    dispatch([p, self, this](){
        p->set_source(m_ctx.name);
        MsgPackHelper::Send(m_ctx.s, p.get());
    });
}

void grok::NodeClient::send_response(nodeService::MsgPack *p, const char *data, std::size_t n)
{
    std::shared_ptr<nodeService::MsgPack> p_rsp = std::make_shared<nodeService::MsgPack>();
    p_rsp->set_msgid(p->msgid());
    p_rsp->set_dest(p->dest());
    p_rsp->set_source(p->source());
    p_rsp->set_msgtype(nodeService::eMsg_response);
    p_rsp->set_sessionid(p->sessionid());
    p_rsp->mutable_pbdata()->assign(data, n);
    //TODO: 当lambda支持move uniqeptr时可以修改,可以把shared_ptr改成uniqe_ptr

    auto self = shared_from_this();
    dispatch([p_rsp, self, this](){
        MsgPackHelper::Send(m_ctx.s, p_rsp.get());
    });
}

void grok::NodeClient::on_puremsg(Session::Ptr s, const char *d, std::size_t n)
{
    // 处理该函数的线程，一定是session->sock->io_service->run的thread
    std::unique_ptr<nodeService::MsgPack> pack = std::make_unique<nodeService::MsgPack>();
    if(!pack->ParseFromArray(d, n)) {
        // 格式不对
        return ;
    }

    MsgOpCall cb;
    {
        auto r = m_data.readGuard();
        auto it = r->m_msg_opmap.find(pack->msgid());
        if(it != r->m_msg_opmap.end()) {
            cb = it->second;
        }
    }
    if (cb) {
        auto self = shared_from_this();
        cb(self, pack.get());
    }
}

std::uint32_t grok::NodeClient::msg_msgnextidx()
{
    return m_req_sessionidx.fetch_add(1);
}
