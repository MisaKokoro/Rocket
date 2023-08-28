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
#include "rocket/net/rpc/rpc_closure.h"
#include "rocket/net/rpc/rpc_controller.h"
#include "rocket/common/msg_id_util.h"
#include "order.pb.h"


// void test_tcp_client() {
//   rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1",12345);
//   rocket::TcpClient client(addr);

//   client.connect([addr,&client]() {
//     DEBUGLOG("success connect [%s]", addr->toString().c_str());
//     auto message = std::make_shared<rocket::TinyPBProtocol>();
//     // message->info = "hello rocket";
//     message->m_msg_id = "99998888";
//     message->m_pb_data = "test pb data";

//     makeOrderRequest request;
//     request.set_price(100);
//     request.set_goods("apple");
//     DEBUGLOG("send message success, request [%s]", request.ShortDebugString().c_str());
    
//     if (!request.SerializeToString(&message->m_pb_data)) {
//         ERRORLOG("serilize failed");
//         return;
//     }   
//     DEBUGLOG("send message success, request [%s]", request.ShortDebugString().c_str());

//     message->m_method_name = "Order.makeOrder";

//     // 为什么这里[&request]就会出错？
//     // 因为回调函数执行的时候test_tcp_client()函数以及执行完了，request对象已经被销毁了
//     client.writeMessage(message, [request](rocket::AbstractProtocol::s_ptr msg_ptr){
//       DEBUGLOG("send message success, request [%s]", request.ShortDebugString().c_str());
//     });

//     client.readMessage("99998888", [](rocket::AbstractProtocol::s_ptr msg_ptr) {
//       auto message = std::dynamic_pointer_cast<rocket::TinyPBProtocol>(msg_ptr);
//       DEBUGLOG("msg_id[%s], get response [%s]", message->getReqId().c_str(), message->m_pb_data.c_str());

//       makeOrderResponse response;
//       if (!response.ParseFromString(message->m_pb_data)) {
//         ERRORLOG("deserilaze failed");
//         return;
//       }

//       DEBUGLOG("get response success , response [%s]",response.ShortDebugString().c_str());

//     });
//   });
// }

void test_rpc_channel() {
    rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1",12345);
    rocket::RpcChannel::s_ptr channel = std::make_shared<rocket::RpcChannel>(addr);

    INFOLOG("channel use count %d, ptr %p", channel.use_count(), channel.get());

    auto request = std::make_shared<makeOrderRequest>();
    auto response = std::make_shared<makeOrderResponse>();

    request->set_price(100);
    request->set_goods("apple");

    auto controller = std::make_shared<rocket::RpcController>();
    controller->SetMsgId(rocket::MsgIDUtil::GenMsgID());

    // INFOLOG("channel use count %d, ptr %p", channel.use_count(), channel.get());

    auto closure = std::make_shared<rocket::RpcClosure>([channel, request, response, controller]() mutable {
      // INFOLOG("channel use count %d, ptr %p", channel.use_count(), channel.get());
      if (controller->GetErrorCode() == 0) {
      INFOLOG("call rpc success, request [%s], response [%s]", 
            request->ShortDebugString().c_str(), response->ShortDebugString().c_str());
      } else {
        INFOLOG("call rpc failed request [%s], error code [%d], error info:%s",
          request->ShortDebugString().c_str(), controller->GetErrorCode(), controller->GetErrorInfo().c_str());
      }
      INFOLOG("Now exit eventLoop");
      channel->getTcpClient()->stop();
      channel.reset();
      // INFOLOG("channel use count %d, ptr %p", channel.use_count(), channel.get());
    });
    // INFOLOG("channel use count %d, ptr %p", channel.use_count(), channel.get());

    channel->Init(controller, request, response, closure);
    // INFOLOG("channel use count %d, ptr %p", channel.use_count(), channel.get());
    
    Order_Stub stub(channel.get());

    controller->SetTimeout(1000);
    
    stub.makeOrder(controller.get(), request.get(), response.get(), closure.get());

    // CALLRPC("127.0.0.1:12345", Order_Stub, makeOrder, controller, request, response, closure);
    
    INFOLOG("test_rpc_channel finished");
    INFOLOG("channel use count %d, ptr %p", channel.use_count(), channel.get());
}

int main() {
    rocket::Config::InitGlobalConfig(nullptr);
    rocket::Logger::InitGlobalLogger(rocket::Logger::SYNC);


    // test_connect();
    // test_tcp_client();
    test_rpc_channel();
    return 0;
}
