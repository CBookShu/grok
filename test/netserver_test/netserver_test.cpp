#include <iostream>
#include "grok/grok.h"
#include <boost/asio.hpp>

using namespace std;

int main(int argc, char** argv) {
    boost::asio::io_service iov;

    grok::Session::Ptr client;
    auto ep = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 9595);
    auto server = std::make_shared<grok::NetServer>(iov);
    iov.post([&](){
        client = grok::Session::Connect(iov, ep);
        client->start();
        const char* s = "hello world";
        client->write(s, strlen(s));
        client->write(s, strlen(s));
        client->write(s, strlen(s));
        client->write(s, strlen(s));
        client->write(s, strlen(s));
    });
    server->start(ep, 8);
    iov.run();
    return 0;
}
