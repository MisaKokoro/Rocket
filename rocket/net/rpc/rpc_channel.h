#ifndef ROCKET_NET_RPC_RPC_CHANNEL_H
#define ROCKET_NET_RPC_RPC_CHANNEL_H

#include <memory>
#include <google/protobuf/service.h>
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_client.h"
#include "rocket/net/timer_event.h"
#include "rocket/net/timer.h"

#define NEWMESSAGE(type, name) \
    std::shared_ptr<type> name = std::make_shared<type>(); \

#define NEWRPCCONTROLLER(name) \
    std::shared_ptr<rocket::RpcController> name = std::make_shared<rocket::RpcController>(); \

#define NEWRPCCHANNEL(addr, name) \
    std::shared_ptr<rocket::RpcChannel> name = std::make_shared<rocket::RpcChannel>(std::make_shared<rocket::IPNetAddr>(addr));\

#define CALLRPC(addr, stub_name, method_name, controller, request, response, closure) \
do { \
    NEWRPCCHANNEL(addr, channel); \
    channel->Init(controller, request, response, closure); \
    stub_name(channel.get()).method_name(controller.get(), request.get(), response.get(), closure.get()); \
} while(0); \

namespace rocket {
class RpcChannel : public google::protobuf::RpcChannel, public std::enable_shared_from_this<RpcChannel> {
public:
    using s_ptr = std::shared_ptr<RpcChannel>;
    using controller_s_ptr = std::shared_ptr<google::protobuf::RpcController>;
    using message_s_ptr = std::shared_ptr<google::protobuf::Message>;
    using closure_s_ptr = std::shared_ptr<google::protobuf::Closure>;

public:
    RpcChannel(NetAddr::s_ptr peer_addr);
    ~RpcChannel();

    // 将用到的指针通过智能指针的方式存下来，防止在使用过程中析构
    void Init(controller_s_ptr controller, message_s_ptr req, message_s_ptr res, closure_s_ptr done);

    void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done);
    
    TcpClient* getTcpClient();

    TimerEvent::s_ptr getTimerEvent();

    google::protobuf::RpcController* getController();
    google::protobuf::Message* getRequest();
    google::protobuf::Message* getResponse();
    google::protobuf::Closure* getClosure();

private:
    NetAddr::s_ptr m_peer_addr;
    NetAddr::s_ptr m_local_addr;

    controller_s_ptr m_controller;
    message_s_ptr m_request;
    message_s_ptr m_response;
    closure_s_ptr m_closure;

    bool m_is_init {false};

    TcpClient::s_ptr m_client;
    TimerEvent::s_ptr m_timer_event;
};
}
#endif