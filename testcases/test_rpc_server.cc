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
        APPDEBUGLOG("sleep 5s for rpc timeout test");
        // sleep(5);
        APPDEBUGLOG("sleep end");
        if (request->price() < 10) {
            response->set_ret_code(-1);
            response->set_res_info("short balance");
        }
        response->set_order_id("20230514");
        APPDEBUGLOG("call makeOrder success");

    }
};


int main(int argc, char* argv[]) {

  if (argc != 2) {
    printf("Start test_rpc_server error, argc not 2 \n");
    printf("Start like this: \n");
    printf("./test_rpc_server ../conf/rocket.xml \n");
    return 0;
  }

  rocket::Config::InitGlobalConfig(argv[1]);

  rocket::Logger::InitGlobalLogger();

  std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
  rocket::RpcDispatcher::GetRpcDispatcher()->registerService(service);

  rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", rocket::Config::GetGlobalConfig()->m_port);

  rocket::TcpServer tcp_server(addr);

  tcp_server.start();

  return 0;
}

