#include <iostream>
#include "grok/grok.h"
#include "grok/nodeService.h"
#include "nodemsg_test.pb.h"

using namespace std;
using namespace boost;
using boost::asio::ip::tcp;
using namespace grok;


static int msg_test_1 = NODE_SERVICE_CUSTOM + 1;
// client1 send msg_test_1<nodemsg_test::TestReq1> to client2
// client2 response msg_test_1<nodemsg_test::TestRsp1> to client1
class TestPb {
public:
    void regster_client1(NodeClient::SPtr client) {
        NODE_SERVICE_REGISTER_MSG(client, msg_test_1, nodemsg_test::TestRsp1, on_client1);
    }
    void regster_client2(NodeClient::SPtr client) {
        NODE_SERVICE_REGISTER_MSG(client, msg_test_1, nodemsg_test::TestReq1, on_client2);
    }
    
    void on_client1(NodeClient::SPtr c, nodeService::MsgPack* p, nodemsg_test::TestRsp1& rsp) {
        rsp.PrintDebugString();
    }
    void on_client2(NodeClient::SPtr c, nodeService::MsgPack* p, nodemsg_test::TestReq1& req) {
        req.PrintDebugString();

        // 返回rsp
        nodemsg_test::TestRsp1 rsp;
        rsp.set_rsp("world");
        auto s = rsp.SerializeAsString();
        c->send_response(p, s.data(), s.size());
    }
};

int main(int argc, char** argv) {
    asio::io_service iov;
    auto ep = tcp::endpoint(tcp::v4(), 9595);
    auto center = grok::NodeCenter::Create(iov, ep);
    TestPb test;

    std::thread t1([ep,&test](){
        asio::io_service iov;
        auto client = grok::NodeClient::Create(iov, ep, "test1");
        test.regster_client1(client);
        asio::basic_waitable_timer<std::chrono::steady_clock> w(iov);
        w.expires_from_now(std::chrono::seconds(2));
        w.async_wait([client](boost::system::error_code ec){
            nodemsg_test::TestReq1 req;
            req.set_req("hello");
            auto s = req.SerializeAsString();
            client->send_notify(msg_test_1, "test2", s.data(), s.size());
        });
        iov.run();
        return ;
    });
    std::thread t2([ep,&test](){
        asio::io_service iov;
        auto client = grok::NodeClient::Create(iov, ep, "test2");
        test.regster_client2(client);
        iov.run();
        return ;
    });
    iov.run();
    t1.join();
    t2.join();
    return 0;
}