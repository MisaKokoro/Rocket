#include "rocket/net/tcp/net_addr.h"
#include "rocket/common/log.h"
#include "rocket/common/config.h"
#include "rocket/net/tcp/tcp_acceptor.h"
#include "rocket/net/tcp/tcp_server.h"

void test_tcp_server() {
    rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 12345);
    rocket::TcpServer tcp_server(addr);

    tcp_server.start();
}

int main() {
    rocket::Config::InitGlobalConfig("/home/yanxiang/Desktop/MyProject/rocket/conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();

    rocket::IPNetAddr addr("127.0.0.1:12348");
    DEBUGLOG("create addr %s", addr.toString().c_str());


    test_tcp_server();


    return 0;
}
