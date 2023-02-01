#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "boost/noncopyable.hpp"
#include "grokSimpleNet.h"
#include "grwtype.h"
#include "pb/nodeService.pb.h"
#include "locklist.h"

namespace grok
{
    struct NodeCtx {
        // node的注册信息
        std::string name;
        // node对应的session
        Session::Ptr s;
    };

    class NodeClient;
    class MsgCenter;
    using MsgPackSPtr = std::shared_ptr<nodeService::MsgPack>;
    using NodeClientSPtr = std::shared_ptr<NodeClient>;
    using MsgCenterSPtr = std::shared_ptr<MsgCenter>;

    class NodeCenter : boost::noncopyable, public std::enable_shared_from_this<NodeCenter> {
    public:
        using SPtr = std::shared_ptr<NodeCenter>;
        ImportFunctional<void(Session::Ptr c, MsgPackSPtr p)> imMsgOperator;

        static SPtr Create(boost::asio::io_service& iov, int port);

        void regsiter_msgcenter(MsgCenterSPtr msgcenter);

        NetServer::SPtr get_server() {
            return m_net_server;
        }
    protected:
        // 网络事件
        void on_newsession(Session::Ptr s);
        void on_closesession(Session::Ptr s, boost::system::error_code ec);
        void on_puremsg(Session::Ptr s, const char* d, std::size_t n);
    protected:
        // node 操作
        void on_registernode(MsgPackSPtr p, Session::Ptr s);
        void on_nodemsg(MsgPackSPtr p, Session::Ptr s);
        Session::Ptr get_session(const std::string& name);
    protected:
        struct Data {
            std::unordered_map<std::string, std::string> key2name;
            std::unordered_map<std::string, NodeCtx> name2node;
        };
        grwtype<Data> m_data;
        NetServer::SPtr m_net_server;
    };

    //TODO: 未来需要增加重连机制
    //TODO: 增加注册成功的状态
    class NodeClient : boost::noncopyable, public std::enable_shared_from_this<NodeClient> {
    public:
        using SPtr = std::shared_ptr<NodeClient>;
        ImportFunctional<void(Session::Ptr c, MsgPackSPtr p)> imMsgOperator;

        NodeClient() = default;
        static SPtr Create(boost::asio::io_service&iosvr, const char* ip, int port, std::string name);

        void regsiter_msgcenter(MsgCenterSPtr msgcenter);

        template<typename F>
        void dispatch(F&& f) {
            if (m_ctx.s) {
                m_ctx.s->dispatch(std::move(f));
            }
        }
        Session::Ptr get_session();

        void send_notify(std::string msgname, const std::string& dest, const char* data, std::size_t n);
        template<typename Pb>
        void send_notify_pb(const std::string& dest, Pb* pb) {
            std::string data;
            if(pb->SerializeToString(&data)) {
                send_notify(Pb::descriptor()->full_name(), dest, data.c_str(), data.size());
            }
        }

        void send_request(std::string msgname, const std::string& dest, const char* data, std::size_t n);
        template<typename Pb>
        void send_request_pb(const std::string& dest, Pb* pb) {
            std::string data;
            if(pb->SerializeToString(&data)) {
                send_request(Pb::descriptor()->full_name(), dest, data.c_str(), data.size());
            }
        }

        void send_response(nodeService::MsgPack* p, const char* data, std::size_t n);
        template<typename Pb>
        void send_response_pb(const std::string& dest, Pb* pb) {
            std::string data;
            if(pb->SerializeToString(&data)) {
                send_response(Pb::descriptor()->full_name(), dest, data.c_str(), data.size());
            }
        }
    protected:
        // 解析消息并分发
        void on_puremsg(Session::Ptr s, const char* d, std::size_t n);
        // 消息顺序id
        std::uint32_t msg_msgnextidx();
    protected:
        std::atomic_uint32_t m_req_sessionidx{0};
        NodeCtx m_ctx;
    };

    // 消息处理的线程池
    // nodeclient和nodeservice 作为socket主要处理IO读写，IO一旦处理完成拿到msgpack
    // 就把msgpack投递到msgcenter的线程池上进行并发的处理
    class MsgCenter : boost::noncopyable, public std::enable_shared_from_this<MsgCenter> {
    public:
        using MsgOpCall = std::function<void(Session::Ptr, MsgPackSPtr)>;

        static MsgCenterSPtr Create();

        MsgCenter() = default;
        ~MsgCenter();

        void start(int num);
        void stop();

        // 线程池的执行
        void post(GrokRunable* run);

        // msg注册和处理
        void register_msg(std::string msgname, MsgOpCall&& cb);

        template <typename PbRep, typename PbRsp, typename F>
        // F 充当函数角色
        // f type -> void (MsgPackSPtr p, PbReq* req, PbRsp* rsp);
        // 并且会自动回复rsp
        void regsiter_reqrsp(F &&f) {
            auto msgname = PbRep::descriptor()->full_name();
            std::function<void(MsgPackSPtr,PbRep*,PbRsp*)> cb(std::forward<F>(f));
            register_msg(msgname, [cb](Session::Ptr c, MsgPackSPtr p){
                PbRep req;
                if(req.ParseFromString(p->pbdata())) {
                    PbRsp rsp;
                    cb(p, &req, &rsp);
                    if(p->msgtype() == nodeService::eMsg_request) {
                        p->set_msgtype(nodeService::eMsg_response);
                        p->set_msgname(PbRsp::descriptor()->full_name());
                        if(rsp.SerializeToString(p->mutable_pbdata())) {
                            std::string data;
                            if(p->SerializeToString(&data)) {
                                c->write(data.c_str(), data.size());
                            }
                        }
                    }
                }
            });
        }
        
        template <typename PbReq, typename F>
        // F 充当函数角色
        // f type -> void (MsgPackSPtr p, PbReq* req);
        void regster_rspnotify(F &&f) {
            auto msgname = PbReq::descriptor()->full_name();
            std::function<void(MsgPackSPtr,PbReq*)> cb(std::forward<F>(f));
            register_msg(msgname, [cb](Session::Ptr c, MsgPackSPtr p){
                PbReq req;
                if(req.ParseFromString(p->pbdata())) {
                    cb(p, &req);
                }
            });
        }
        
        void msg_operator(Session::Ptr s, MsgPackSPtr p);
    protected:
        // 消息注册
        struct Data {
            std::unordered_map<std::string, MsgOpCall> msg2calls;
        };
        grwtype<Data> m_data;

    protected:
        // 消息分发执行
        void thread_func();

        LockList<GrokRunable > m_op_lists;
        std::vector<std::thread> m_op_threads;
    };

    #define AUTO_REGISTER_PBMSG_1(msgcenter,pbtype,func) \
            msgcenter->regster_rspnotify<pbtype>([this](MsgPackSPtr p,pbtype*pb){func(p,pb);});
    #define AUTO_REGISTER_PBMSG_2(msgcenter,pbreq,pbrsp,func) \
        msgcenter->regsiter_reqrsp<pbreq,pbrsp>([this](MsgPackSPtr p,pbreq*req,pbrsp*rsp){func(p,req,rsp);});

} // namespace grok
