#include "grokSimpleNet.h"
#include <atomic>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <thread>
#include <vector>


#define ASSERT_SYSERROR(ec) if(ec) {DBG("SYSERROR %s", ec.message().c_str());assert(false);}
using namespace grok;
using namespace boost;
using boost::asio::ip::tcp;

static std::uint64_t MakeToken()
{
    static std::atomic_int64_t gID{0};
    return gID.fetch_add(1);
}

grok::Session::Session(asio::io_service& iosvr):
m_sock(iosvr),m_write_buf_idx(-1) {
}

grok::Session::~Session() {
    DBG("SESSIONCLOSE %s", to_string().c_str());
}

Session::Ptr grok::Session::Connect(boost::asio::io_service&iosvr,boost::asio::ip::tcp::endpoint ep,PacketProtocolBase::SPtr protocl)
{
    Session::Ptr session = std::make_shared<Session>(iosvr);
    session->m_packptr = protocl;

    boost::system::error_code ec;
    session->m_sock.connect(ep, ec);
    if(ec) {
        DBG("CONNECT %s", ec.message().c_str());
        return nullptr;
    }

    return session;
}

std::string grok::Session::to_string()
{
    if(!m_sock.is_open()) {
        return "unopen";
    }
    auto ep = m_sock.remote_endpoint();
    auto addr_port = ep.address().to_string();
    addr_port = addr_port + ":" + std::to_string(ep.port());
    return addr_port;
}

void Session::start()
{
    if(!m_packptr) {
        m_packptr = std::make_shared<PacketProtocolBase>();
    }
    boost::system::error_code ec;
    m_sock.non_blocking(true, ec);
    ASSERT_SYSERROR(ec);
    m_sock.set_option(tcp::no_delay(true), ec);
    ASSERT_SYSERROR(ec);

    do_read();
}

void grok::Session::write(const char *s, std::size_t n)
{
    auto self = shared_from_this();
    std::shared_ptr<std::string> sptr = std::make_shared<std::string> (s, n);
    dispatch([self,sptr,this](){
        if(m_write_buf_idx == -1) {
            m_write_buf_idx = 0;
            m_packptr->pack(m_writebuf[m_write_buf_idx], sptr->data(), sptr->size());
            do_write();
        } else {
            // 先缓存起来
            int cache_write_idx = 0;
            if (m_write_buf_idx == 0) {
                cache_write_idx = 1;
            }
            m_packptr->pack(m_writebuf[cache_write_idx], sptr->data(), sptr->size());
        }
    });
}

void grok::Session::do_write() {
    auto self = shared_from_this();
    asio::async_write(m_sock, m_writebuf[m_write_buf_idx].bufferdata(), 
        [self, this](boost::system::error_code ec, std::size_t sz){
            if(ec) {
                DBG("Read Error:%s", ec.message().c_str());
                if(ec == asio::error::interrupted
                || ec == asio::error::eof) {
                    evClose(self, ec);
                    return;
                }
            }
            if(sz > 0) {
                m_writebuf[m_write_buf_idx].consume(sz);
            }
            // 当前的buffer还有未发送完毕的数据
            if(m_writebuf[m_write_buf_idx].buffersize() > 0) {
                do_write();
                return;
            }
            // 查看另外一个Buffer是否有还需要发送的数据
            // 先缓存起来
            m_write_buf_idx = m_write_buf_idx == 0 ? 1 : 0;
            if(m_writebuf[m_write_buf_idx].buffersize() > 0) {
                do_write();
                return;
            }
            // 所有的数据都已经写完了
            m_write_buf_idx = -1;
    });
}

void grok::Session::do_read() {
    auto self = shared_from_this();
    asio::async_read(m_sock, m_readbuf.prepare(4096), asio::transfer_at_least(4), 
        [this, self](boost::system::error_code ec, std::size_t sz) {
            if(ec) {
                DBG("Read Error:%s", ec.message().c_str());
                if(ec == asio::error::interrupted
                || ec == asio::error::eof) {
                    evClose(self, ec);
                    return;
                }
            }
            if(sz > 0) {
                m_readbuf.commit(sz);
                while(true) {
                    std::size_t n = 0;
                    if(m_packptr->parse(m_readbuf, n)) {
                        auto buf = m_readbuf.bufferdata(n);
                        evMsg(self, asio::buffer_cast<const char*>(buf), n);
                        m_readbuf.consume(n);
                    } else {
                        break;
                    }
                }
            }
            do_read(); 
        }
    );
}

grok::NetServer::NetServer(boost::asio::io_service& iov):m_iosvr(iov), m_accepter(iov) {

}

grok::NetServer::~NetServer()
{
    DBG("~");
}

void grok::NetServer::set_packet_protocol(PacketProtocolBase::SPtr p)
{
    if(m_running.load()) {
        // 一旦运行起来之后，就不允许修改解析协议了！
        return;
    }
    if(p) {
        m_packptr.swap(p);
    }
}

void grok::NetServer::start(int port, int thread)
{
    bool r = false;
    if (!m_running.compare_exchange_strong(r, true))
    {
        return;
    }
    try {
        if(!m_packptr) {
            m_packptr = std::make_shared<PacketProtocolBase>();
        }
        
        m_accepter = tcp::acceptor(m_iosvr, tcp::endpoint(tcp::v4(), port));

        m_thread_pools.resize(thread);
        for (int i = 0; i < thread; ++i)
        {
            m_iosvr_pools.push_back(std::make_shared<asio::io_service>(1));
            m_thread_pools[i] = std::thread([this, i]()
                                            {
                asio::io_service::work w(*m_iosvr_pools[i]);
                m_iosvr_pools[i]->run(); });
        }
        do_accept();
    } catch(boost::system::error_code ec) {
        DBG("error %s", ec.message().c_str());
    }
}

void grok::NetServer::stop()
{
    m_iosvr.stop();
    for (int i = 0; i < m_iosvr_pools.size(); ++i)
    {
        m_iosvr_pools[i]->stop();
    }
    for (int i = 0; i < m_thread_pools.size(); ++i)
    {
        m_thread_pools[i].join();
    }
    m_accepter.cancel();
    m_iosvr_pools.clear();
    m_running.exchange(false);
}

void grok::NetServer::do_accept()
{
    auto self = shared_from_this();
    auto idx = next_ioidx();
    auto iosvr = iosvr_byidx(idx);
    auto session = std::make_shared<Session>(*iosvr);
    session->m_packptr = m_packptr;
    session->evClose += grok::delegate(self, &NetServer::on_closesession);   
    session->evMsg += grok::delegate(self, &NetServer::on_msg);

    m_accepter.async_accept(session->m_sock, 
    [self, session, this](boost::system::error_code ec) {
            if(ec) {
                DBG("accept error:%s", ec.message().c_str());
            } else {
                session->m_sock.non_blocking(true);
                session->m_sock.set_option(tcp::no_delay(true));
                on_newsession(session);
                session->start();
            }
            do_accept(); 
        }
    );
}

std::uint32_t grok::NetServer::next_ioidx() {
    auto idx = m_cur.fetch_add(1) % m_iosvr_pools.size();
    return idx;
}

asio::mutable_buffers_1 grok::SimpleStream::prepare(size_type n)
{
    auto space = this->size() - pos_;
    if(space < n) {
        resize(size() + n - space);
    }
    char *p = (char*)data() + pos_;
    return asio::buffer(p, n);
}

asio::const_buffers_1 grok::SimpleStream::bufferdata()
{
    return asio::buffer(data(), pos_);
}

asio::const_buffers_1 grok::SimpleStream::bufferdata(size_type n)
{
    return asio::buffer(data(), std::min(pos_, n));
}

std::string::size_type grok::SimpleStream::buffersize()
{
    return pos_;
}

void grok::SimpleStream::commit(size_type n)
{
    pos_ += n;
    pos_ = std::min(size(), pos_);
}

void grok::SimpleStream::consume(size_type n)
{
    erase(0, n);
    pos_ -= n;
}

bool grok::PacketProtocolBase::pack(SimpleStream &ss, const char *s, std::size_t n)
{
    auto l = boost::asio::detail::socket_ops::host_to_network_long(n);
    auto buf = ss.prepare(sizeof(l));
    asio::buffer_copy(buf, asio::buffer(&l, sizeof(l)));
    ss.commit(sizeof(l));

    buf = ss.prepare(n);
    asio::buffer_copy(buf, asio::buffer(s, n));
    ss.commit(n);
    return true;
}

bool grok::PacketProtocolBase::parse(SimpleStream &ss, std::size_t &n)
{
    auto buf = ss.bufferdata();
    auto sz = asio::buffer_size(buf);
    if(sz < sizeof(asio::detail::u_long_type)) {
        return false;
    }
    const char* p = asio::buffer_cast<const char*>(buf);
    asio::detail::u_long_type l;
    memcpy(&l, p, sizeof(l));
    l = boost::asio::detail::socket_ops::network_to_host_long(l);

    if(sz < l + sizeof(l)) {
        return false;
    }
    n = l;
    ss.consume(sizeof(l));
    return true;
}
