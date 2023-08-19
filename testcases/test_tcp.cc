#include "rocket/net/tcp/net_addr.h"
#include "rocket/common/log.h"
#include "rocket/common/config.h"
#include "rocket/net/tcp/tcp_acceptor.h"

int main() {
    rocket::Config::InitGlobalConfig("/home/yanxiang/Desktop/MyProject/rocket/conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();

    rocket::IPNetAddr addr("127.0.0.1:12348");
    DEBUGLOG("create addr %s", addr.toString().c_str());



    return 0;
}
