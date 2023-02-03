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

struct MsgPackOperatorRun : GrokRunable {
    Session::Ptr client;
    MsgCenter::MsgOpCall cb;
    MsgPackSPtr pack;

    virtual void run() override {
        cb(client, pack);
    }
};

grok::NodeCenter::SPtr grok::NodeCenter::Create(boost::asio::io_service& iov, int port)
{
    auto t = std::thread::hardware_concurrency() + 1;

    auto sptr = std::make_shared<NodeCenter>();
    sptr->m_net_server = std::make_shared<NetServer>(iov);
    sptr->m_net_server->evClose += delegate(sptr, &NodeCenter::on_closesession);
    sptr->m_net_server->evConnect += delegate(sptr, &NodeCenter::on_newsession);
    sptr->m_net_server->evMsg += delegate(sptr, &NodeCenter::on_puremsg);


    sptr->m_net_server->start(port, t);
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
    auto pack = std::make_shared<nodeService::MsgPack>();
    if(!pack->ParseFromArray(d, n)) {
        // 格式不对
        return ;
    }

    if(pack->msgname() == nodeService::ReqNodeRegistor::descriptor()->full_name()) {
        on_registernode(pack, s);
        return;
    }

    on_nodemsg(pack, s);
    return;
}

void grok::NodeCenter::on_registernode(MsgPackSPtr p, Session::Ptr s)
{
    nodeService::RspNodeRegistor rsp;
    rsp.set_status(nodeService::eRegister_OK);
    if(p->source().empty()) {
        // 参数错误
        rsp.set_status(nodeService::eRegister_ParamErr);
        p->set_msgtype(nodeService::eMsg_response);
        MsgPackHelper::Send(s, p.get(), rsp);
        return;
    }

    if(p->source() == gNodeCenterName) {
        // 参数错误
        rsp.set_status(nodeService::eRegister_ParamErr);
        p->set_msgtype(nodeService::eMsg_response);
        MsgPackHelper::Send(s, p.get(), rsp);
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
    MsgPackHelper::Send(s, p.get(), rsp);
}

void grok::NodeCenter::on_nodemsg(MsgPackSPtr p, Session::Ptr s)
{
    if(p->msgtype() == nodeService::eMsg_response) {
        // 找到
        auto source_s = get_session(p->source());
        if(!source_s) {
            // response的节点已经挂了
            return;
        }
        MsgPackHelper::Send(source_s, p.get());
        return;
    }

    if(p->dest() == gNodeCenterName) {
        auto source_s = get_session(p->source());
        if(!source_s) {
            return;
        }
        // 分发处理
        evMsgCome(source_s, p);
        return;
    }

    // 转发消息
    auto dest_s = get_session(p->dest());
    if(!dest_s) {
        return;
    }
    MsgPackHelper::Send(dest_s, p.get());
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

grok::NodeClient::SPtr grok::NodeClient::Create(boost::asio::io_service &iosvr, const char *ip, int port, std::string name)
{
    SPtr r = std::make_shared<NodeClient>();
    r->m_ctx.s = Session::Connect(iosvr, ip, port);
    if(!r->m_ctx.s) {
        return nullptr;
    }
    r->m_ctx.s->evMsg += delegate(r, &NodeClient::on_puremsg);
    r->m_ctx.name = name;
    r->m_ctx.s->start();
    // 发送注册
    nodeService::MsgPack pack;
    pack.set_msgname(nodeService::ReqNodeRegistor::descriptor()->full_name());
    pack.set_source(name);
    pack.set_msgtype(nodeService::eMsg_request);
    MsgPackHelper::Send(r->m_ctx.s, &pack);
    return r;
}

Session::Ptr grok::NodeClient::get_session()
{
    return m_ctx.s;
}

void grok::NodeClient::send_notify(std::string msgname, const std::string &dest, const char *data, std::size_t n)
{
    std::shared_ptr<nodeService::MsgPack> p = std::make_shared<nodeService::MsgPack>();
    p->set_dest(dest);
    p->mutable_pbdata()->append(data, n);
    p->set_msgname(msgname);
    p->set_msgtype(nodeService::eMsg_notify);
    //TODO: 当lambda支持move uniqeptr时可以修改,可以把shared_ptr改成uniqe_ptr

    auto self = this->shared_from_this();
    dispatch([p, self, this](){
        p->set_source(m_ctx.name);
        MsgPackHelper::Send(m_ctx.s, p.get());
    });
}

void grok::NodeClient::send_request(std::string msgname, const std::string &dest, const char *data, std::size_t n)
{
    std::shared_ptr<nodeService::MsgPack> p = std::make_shared<nodeService::MsgPack>();
    p->set_dest(dest);
    p->mutable_pbdata()->append(data, n);
    p->set_msgname(msgname);
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
    p_rsp->set_msgname(p->msgname());
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
    auto pack = std::make_shared<nodeService::MsgPack>();
    if(!pack->ParseFromArray(d, n)) {
        // 格式不对
        return ;
    }

    evMsgCome(m_ctx.s, pack);
}

std::uint32_t grok::NodeClient::msg_msgnextidx()
{
    return m_req_sessionidx.fetch_add(1);
}

MsgCenterSPtr grok::MsgCenter::Create()
{
    auto sptr = std::make_shared<MsgCenter>();
    auto thread = std::thread::hardware_concurrency() * 2;
    sptr->start(thread);
    return sptr;
}

grok::MsgCenter::~MsgCenter()
{
    stop();
}

void grok::MsgCenter::start(int num)
{
    m_op_threads.resize(num);
    for (int i = 0; i < num; ++i) {
        m_op_threads[i] = std::thread([this](){
            thread_func();
        });
    }
}

void grok::MsgCenter::stop()
{
    for (int i = 0; i < m_op_threads.size(); ++i) {
        m_op_lists.Give(nullptr);
    }

    for (int i = 0; i < m_op_threads.size(); ++i) {
        m_op_threads[i].join();
    }
}

void grok::MsgCenter::post(GrokRunable *run)
{
    m_op_lists.Give(run);
}

void grok::MsgCenter::register_msg(std::string msgname, MsgOpCall &&cb)
{
    auto w = m_data.writeGuard();
    w->msg2calls[msgname] = std::move(cb);
}

void grok::MsgCenter::msg_operator(Session::Ptr s, MsgPackSPtr p)
{
    MsgOpCall cb;
    {
        auto r = m_data.readGuard();
        auto it = r->msg2calls.find(p->msgname());
        if (it != r->msg2calls.end()) {
            cb = it->second;
        }
    }
    if(cb) {
        auto* run = new MsgPackOperatorRun;
        run->cb = cb;
        run->client = s;
        run->pack = p;
        m_op_lists.Give(run);
    }
}

void grok::MsgCenter::thread_func()
{
    for(;;) {
        auto *p = m_op_lists.Get();
        if(!p) {
            // 收到空说明要结束了
            break;
        }
        p->run();
        delete p;
    }
}
