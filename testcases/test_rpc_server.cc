#include <unistd.h>
#include <string.h>
#include <google/protobuf/service.h>
#include "rocket/net/tcp/net_addr.h"
#include "rocket/common/log.h"
#include "rocket/common/config.h"
#include "rocket/net/tcp/tcp_acceptor.h"
#include "rocket/net/tcp/tcp_server.h"
#include "rocket/net/fd_event_group.h"
#include "rocket/net/tcp/tcp_connection.h"
#include "rocket/net/tcp/tcp_client.h"
#include "rocket/net/coder/string_coder.h"
#include "rocket/net/coder/tinypb_coder.h"
#include "rocket/net/coder/tinypb_protocol.h"
#include "rocket/net/rpc/rpc_dispatcher.h"

#include "order.pb.h"

class OrderImpl : public Order {
    void makeOrder(google::protobuf::RpcController* controller,
                       const ::makeOrderRequest* request,
                       ::makeOrderResponse* response,
                       ::google::protobuf::Closure* done) {
        if (request->price() < 10) {
            response->set_ret_code(-1);
            response->set_res_info("short balance");
        }
        response->set_order_id("20230514");

    }
};


void test_tcp_server() {
    rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 12345);
    rocket::TcpServer tcp_server(addr);

    auto service = std::make_shared<OrderImpl>();
    rocket::RpcDispatcher::GetRpcDispatcher()->registerService(service);

    tcp_server.start();
}

int main() {
    rocket::Config::InitGlobalConfig("/home/yanxiang/Desktop/MyProject/rocket/conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();

    rocket::IPNetAddr addr("127.0.0.1:12345");
    DEBUGLOG("create addr %s", addr.toString().c_str());


    test_tcp_server();


    return 0;
}

