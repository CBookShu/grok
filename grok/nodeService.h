#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "boost/noncopyable.hpp"
#include "grokSimpleNet.h"
#include "utils.h"
#include "nodeMsgid.h"
#include "pb/nodeService.pb.h"

namespace grok
{
    struct NodeCtx {
        // node的注册信息
        std::string name;
        // node对应的session
        Session::Ptr s;
    };

    class NodeCenter : boost::noncopyable, std::enable_shared_from_this<NodeCenter> {
    public:
        using SPtr = std::shared_ptr<NodeCenter>;
        
        static SPtr Create(boost::asio::io_service& iov, boost::asio::ip::tcp::endpoint ep);
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
        void on_registernode(nodeService::MsgPack* p, Session::Ptr s);
        void on_nodemsg(nodeService::MsgPack* p, Session::Ptr s);
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
        using MsgOpCall = std::function<void(NodeClient::SPtr, nodeService::MsgPack*)>;
        template <typename Pb>
        using MsgOpCallPb = std::function<void(NodeClient::SPtr, nodeService::MsgPack*, Pb&)>;

        NodeClient() = default;
        static SPtr Create(boost::asio::io_service&iosvr, boost::asio::ip::tcp::endpoint ep, std::string name);
    
        void register_msgid(std::int32_t msgid, MsgOpCall&& cb);
        template <typename Pb>
        void register_msgid_withpb(std::int32_t msgid, MsgOpCallPb<Pb>&& cb) {
            register_msgid(msgid, [cb](NodeClient::SPtr c, nodeService::MsgPack* p){
                Pb pb;
                if(pb.ParseFromString(p->pbdata())) {
                    cb(c, p, pb);
                }
            });
        }

        template<typename F>
        void dispatch(F&& f) {
            if (m_ctx.s) {
                m_ctx.s->dispatch(std::move(f));
            }
        }
        Session::Ptr get_session();

        void send_notify(std::int32_t msgid, const std::string& dest, const char* data, std::size_t n);
        void send_request(std::int32_t msgid, const std::string& dest, const char* data, std::size_t n);
        void send_response(nodeService::MsgPack* p, const char* data, std::size_t n);
    protected:
        // 解析消息并分发
        void on_puremsg(Session::Ptr s, const char* d, std::size_t n);
        // 消息顺序id
        std::uint32_t msg_msgnextidx();
    protected:
        struct Data {
            std::unordered_map<std::int32_t, MsgOpCall> m_msg_opmap;
        };
        grwtype<Data> m_data;
        std::atomic_uint32_t m_req_sessionidx{0};
        NodeCtx m_ctx;
    };


    #define NODE_SERVICE_REGISTER_MSG(client,msgid,pbtype,cb)    \
        client->register_msgid_withpb<pbtype>(msgid, \
        [this](NodeClient::SPtr p1, nodeService::MsgPack*p2, pbtype& p3){\
            this->cb(p1,p2,p3); \
        });
} // namespace grok
