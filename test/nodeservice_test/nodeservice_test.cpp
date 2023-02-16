#include <iostream>
#include "grok/grok.h"
#include "grok/nodeService.h"
#include "nodemsg_test.pb.h"

using namespace std;
using namespace boost;
using boost::asio::ip::tcp;
using namespace grok;


// client1 send msg_test_1<nodemsg_test::TestReq1> to client2
// client2 response msg_test_1<nodemsg_test::TestRsp1> to client1
class TestPb {
public:
    void regster_client1(MsgCenterSPtr msgcenter) {
        AUTO_REGISTER_PBMSG_1(msgcenter, nodemsg_test::TestRsp1, on_client1);
    }
    void regster_client2(MsgCenterSPtr msgcenter) {
        AUTO_REGISTER_PBMSG_2(msgcenter, nodemsg_test::TestReq1, nodemsg_test::TestRsp1, on_client2);
    }
    
    void on_client1(MsgPackSPtr p, nodemsg_test::TestRsp1* rsp) {
        rsp->PrintDebugString();
    }
    void on_client2(MsgPackSPtr p, nodemsg_test::TestReq1* req,nodemsg_test::TestRsp1* rsp) {
        req->PrintDebugString();
        rsp->set_rsp("world");
    }
};

int main(int argc, char** argv) {
    asio::io_service iov;
    const char* ip = "127.0.0.1";
    int port = 9595;
    auto center = grok::NodeCenter::Create(iov, port);
    TestPb test;

    std::thread t1([&](){
        asio::io_service iov;
        auto mc_client = grok::MsgCenter::Create();
        auto client = grok::NodeClient::Create(iov, ip, port, "test1");
        client->evMsgCome += delegate(mc_client, &MsgCenter::msg_operator);
        mc_client->imWriteMsgPack = make_function_wrapper(client, &grok::NodeClient::write_msgpack);
        test.regster_client1(mc_client);
        mc_client->start(1);

        asio::basic_waitable_timer<std::chrono::steady_clock> w(iov);
        w.expires_from_now(std::chrono::seconds(2));
        w.async_wait([client](boost::system::error_code ec){
            nodemsg_test::TestReq1 req;
            req.set_req("hello");
            client->send_request_pb("test2", req);
        });
        iov.run();
        return ;
    });
    std::thread t2([&](){
        asio::io_service iov;
        auto mc_client = grok::MsgCenter::Create();
        auto client = grok::NodeClient::Create(iov, ip, port, "test2");
        client->evMsgCome += delegate(mc_client, &MsgCenter::msg_operator);
        test.regster_client2(mc_client);
        mc_client->imWriteMsgPack = make_function_wrapper(client, &grok::NodeClient::write_msgpack);
        mc_client->start(1);

        iov.run();
        return ;
    });
    iov.run();
    t1.join();
    t2.join();
    return 0;
}