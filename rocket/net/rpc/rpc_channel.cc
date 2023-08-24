#include <google/protobuf/service.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include "rocket/net/rpc/rpc_channel.h"
#include "rocket/net/coder/tinypb_protocol.h"
#include "rocket/common/msg_id_util.h"
#include "rocket/net/rpc/rpc_controller.h"
#include "rocket/common/log.h"
#include "rocket/common/error_code.h"
#include "rocket/net/tcp/tcp_client.h"

namespace rocket {
    RpcChannel::RpcChannel(NetAddr::s_ptr peer_addr) : m_peer_addr(peer_addr) {

    }

    RpcChannel::~RpcChannel() {

    }

    void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                                google::protobuf::RpcController* controller, 
                                const google::protobuf::Message* request,
                                google::protobuf::Message* response, 
                                google::protobuf::Closure* done) {
        
        // 发送的请求message
        std::shared_ptr<TinyPBProtocol> req_protocol = std::make_shared<TinyPBProtocol>();
        RpcController* my_controller = dynamic_cast<RpcController*>(controller);

        if (my_controller == nullptr) {
            ERRORLOG("controller dynamic cast failed");
            return;
        }

        // 获取msgID
        if (my_controller->GetMsgId().empty()) {
            req_protocol->m_msg_id = MsgIDUtil::GenMsgID();
            my_controller->SetMsgId(req_protocol->m_msg_id);
        } else {
            req_protocol->m_msg_id = my_controller->GetMsgId();
        }

        // 获取method name
        req_protocol->m_method_name = method->full_name();
        INFOLOG("%s | call method name [%s]", req_protocol->m_msg_id.c_str(), req_protocol->m_method_name.c_str());

        // 将request序列化
        if (!request->SerializeToString(&req_protocol->m_pb_data)) {
            std::string err_info = "failed to serialize";
            my_controller->SetError(ERROR_FAILED_SERIALIZE, err_info);
            ERRORLOG("%s | error : %s, origin request [%s] ", req_protocol->m_msg_id.c_str(), 
                        err_info.c_str(), request->ShortDebugString().c_str());
            return;
        }

        // 创建客户端，将message发送
        TcpClient client(m_peer_addr);
        client.connect([&client, req_protocol, done]() {
            // 发送message
            client.writeMessage(req_protocol, [&client, req_protocol, done](AbstractProtocol::s_ptr) {
                INFOLOG("%s |, send request success: call method name [%s]", req_protocol->m_msg_id.c_str(),
                        req_protocol->m_method_name.c_str());
                // 拿到回包后进行解析
                client.readMessage(req_protocol->m_msg_id, [done](AbstractProtocol::s_ptr msg) {
                    auto rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(msg);
                    INFOLOG("%s | success get response method name [%s]", rsp_protocol->m_msg_id.c_str(), rsp_protocol->m_method_name.c_str());
                    // 成功收到回包，执行对应的回调函数
                    if (done) {
                        done->Run();
                    }
                });
            });
        });
        

    }
}