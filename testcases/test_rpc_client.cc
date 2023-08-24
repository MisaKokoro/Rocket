#include <unistd.h>
#include <string.h>
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
#include "rocket/net/rpc/rpc_channel.h"
#include "order.pb.h"


void test_tcp_client() {
  rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1",12345);
  rocket::TcpClient client(addr);

  client.connect([addr,&client]() {
    DEBUGLOG("success connect [%s]", addr->toString().c_str());
    auto message = std::make_shared<rocket::TinyPBProtocol>();
    // message->info = "hello rocket";
    message->m_msg_id = "99998888";
    message->m_pb_data = "test pb data";

    makeOrderRequest request;
    request.set_price(100);
    request.set_goods("apple");
    DEBUGLOG("send message success, request [%s]", request.ShortDebugString().c_str());
    
    if (!request.SerializeToString(&message->m_pb_data)) {
        ERRORLOG("serilize failed");
        return;
    }   
    DEBUGLOG("send message success, request [%s]", request.ShortDebugString().c_str());

    message->m_method_name = "Order.makeOrder";

    // 为什么这里[&request]就会出错？
    // 因为回调函数执行的时候test_tcp_client()函数以及执行完了，request对象已经被销毁了
    client.writeMessage(message, [request](rocket::AbstractProtocol::s_ptr msg_ptr){
      DEBUGLOG("send message success, request [%s]", request.ShortDebugString().c_str());
    });

    client.readMessage("99998888", [](rocket::AbstractProtocol::s_ptr msg_ptr) {
      auto message = std::dynamic_pointer_cast<rocket::TinyPBProtocol>(msg_ptr);
      DEBUGLOG("msg_id[%s], get response [%s]", message->getReqId().c_str(), message->m_pb_data.c_str());

      makeOrderResponse response;
      if (!response.ParseFromString(message->m_pb_data)) {
        ERRORLOG("deserilaze failed");
        return;
      }

      DEBUGLOG("get response success , response [%s]",response.ShortDebugString().c_str());

    });
  });
}

int main() {
    rocket::Config::InitGlobalConfig("/home/yanxiang/Desktop/MyProject/rocket/conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();


    // test_connect();
    test_tcp_client();
    return 0;
}
