#pragma once
#include <memory>
#include <atomic>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <thread>
#include <vector>

#include "event.h"
#include "groklog.h"

namespace grok {
    struct NetServer;

    // 一段连续的缓存  可用的buf + 空闲buf 组成
    struct SimpleStream : public std::string {
        size_type pos_ = 0;

        SimpleStream() = default;

        // 获取空间
        boost::asio::mutable_buffers_1 prepare(size_type n);
        // 获取数据
        boost::asio::const_buffers_1 bufferdata();
        // 获取数据
        boost::asio::const_buffers_1 bufferdata(size_type n);
        // 可用的数据
        size_type buffersize();
        // 提交数据
        void commit(size_type n);
        // 释放数据
        void consume(size_type n);
    };

    // 默认的一套协议格式
    // head[4bit] + body
    // head转换成 body 长度
    struct PacketProtocolBase {
        using SPtr = std::shared_ptr<PacketProtocolBase>;
        // 将长度为n的s字符串，打包到ss中
        virtual bool pack(SimpleStream& ss, const char* s, std::size_t n);
        // 将ss中的数据解析出n,然后把body的begin指针设置好
        virtual bool parse(SimpleStream& ss, std::size_t& n);    
    };

    struct Session : boost::noncopyable, public std::enable_shared_from_this<Session> {
        using Ptr = std::shared_ptr<Session>;

        EventNoMutex<Ptr, boost::system::error_code> evClose;
        EventNoMutex<Ptr, const char*, std::size_t> evMsg;

        std::uint64_t m_token = -1;
        PacketProtocolBase::SPtr m_packptr;
        boost::asio::ip::tcp::socket m_sock;
        SimpleStream m_readbuf;
        SimpleStream m_writebuf[2];
        int m_write_buf_idx;
        
        Session(boost::asio::io_service& iosvr);
        ~Session();

        static Ptr Connect(boost::asio::io_service&iosvr, const char *ip, int port, PacketProtocolBase::SPtr protocl = nullptr);

        std::string to_string();

        void start();
        void stop();
        void write(const char*s, std::size_t n);
        template<typename F>
        void dispatch(F &&f) {
            m_sock.get_io_service().dispatch(std::move(f));
        }
    protected:
        void do_write();
        void do_read();
        void do_close(boost::system::error_code ec = {});
    };

    struct NetServer : boost::noncopyable, public std::enable_shared_from_this<NetServer> {
        using SPtr = std::shared_ptr<NetServer>;

        std::atomic_uint32_t m_cur{0};
        std::atomic_bool m_running{false};
        boost::asio::io_service& m_iosvr;
        std::vector<std::thread> m_thread_pools;
        std::vector<std::shared_ptr<boost::asio::io_service>> m_iosvr_pools;
        boost::asio::ip::tcp::acceptor m_accepter;
        PacketProtocolBase::SPtr m_packptr;

        EventNoMutex<Session::Ptr> evConnect;
        EventNoMutex<Session::Ptr, boost::system::error_code> evClose;
        EventNoMutex<Session::Ptr, const char*, std::size_t> evMsg;

        NetServer(boost::asio::io_service& iov);
        ~NetServer();

        void set_packet_protocol(PacketProtocolBase::SPtr p);

        void start(int port, int thread);

        void stop();

        boost::asio::io_service& iosvr_byidx(std::uint32_t idx) {
            return *m_iosvr_pools[idx];
        }

        boost::asio::io_service& iosvr_rnd();

        boost::asio::io_service& iosvr_main() {
            return m_iosvr;
        }

        void on_newsession(Session::Ptr session) {
            auto key = session->to_string();
            DBG("NEWSESSION %s", key.c_str());
            evConnect(session);
        }
        void on_closesession(Session::Ptr session, boost::system::error_code ec) {
            auto key = session->to_string();
            DBG("CLOSESESSION %s[%s]", key.c_str(), ec.message().c_str());
            evClose(session, ec);
        }
        void on_msg(Session::Ptr session, const char* s, std::size_t n) {
            auto key = session->to_string();
            // DBG("MSG %s[%s]",key.c_str(), std::string(s, n).c_str());
            evMsg(session, s, n);
        }
    protected:
        void do_accept();
        std::uint32_t next_ioidx();
    };
}