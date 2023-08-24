#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "rocket/common/log.h"
#include "rocket/net/rpc/rpc_dispatcher.h"
#include "rocket/net/coder/tinypb_protocol.h"
#include "rocket/common/error_code.h"
#include "rocket/net/rpc/rpc_controller.h"
#include "rocket/net/tcp/tcp_connection.h"

namespace rocket {

static RpcDispatcher* g_rpc_dispatcher = nullptr;

RpcDispatcher* RpcDispatcher::GetRpcDispatcher() {
    if (g_rpc_dispatcher) {
        return g_rpc_dispatcher;
    }
    g_rpc_dispatcher = new RpcDispatcher();
    return g_rpc_dispatcher;
}

void RpcDispatcher::dispatcher(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr response, TcpConnection* connection) {
    auto req_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(request);
    auto rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(response);
    std::string method_full_name = req_protocol->m_method_name;
    std::string service_name, method_name;

    rsp_protocol->m_req_id = req_protocol->m_req_id;
    rsp_protocol->m_method_name = req_protocol->m_method_name;


    if (!parseServiceFullName(method_full_name, service_name, method_name)) {
        setTinyPBError(rsp_protocol, ERROR_PARSE_SERVICE_NAME, "parse service name error");
        return;
    }

    // 判断当前service是否注册
    auto it = m_service_map.find(service_name);
    if (it == m_service_map.end()) {
        ERRORLOG("%s | service name[%s] not found", req_protocol->m_req_id.c_str(),service_name.c_str());
        setTinyPBError(rsp_protocol, ERROR_SERVICE_NOT_FOUND, "service not found");
        return;
    }

    // 从service中寻找method
    service_s_ptr service = it->second;
    const google::protobuf::MethodDescriptor* method = service->GetDescriptor()->FindMethodByName(method_name);
    if (method == nullptr) {
        ERRORLOG("%s | method name [%d] not found in service [%s]", req_protocol->m_req_id.c_str(),method_name.c_str(),service_name.c_str());
        setTinyPBError(rsp_protocol, ERROR_METHOD_NOT_FOUND, "service not found");
        return;
    }

    google::protobuf::Message* req_msg = service->GetRequestPrototype(method).New();

    // 反序列化处理， 将pb_data反序列化为req_msg
    if (!req_msg->ParseFromString(req_protocol->m_pb_data)) {
        ERRORLOG("%s | deserilize error", req_protocol->m_req_id.c_str());
        setTinyPBError(rsp_protocol, ERROR_FAILED_DESERIALIZE, "deserilize error");
        if (req_msg) {
            delete req_msg;
            req_msg = nullptr;
        }
        return;
    }

    INFOLOG("req_id[%s], get rpc request [%s]", req_protocol->m_req_id.c_str(), req_msg->ShortDebugString().c_str());

    google::protobuf::Message* rsp_msg = service->GetResponsePrototype(method).New();

    // 调用response函数，将响应消息存到rsp_msg
    RpcController rpcController;
    rpcController.SetLocalAddr(connection->getLocalAddr());
    rpcController.SetPeerAddr(connection->getPeerAddr());
    rpcController.SetReqId(req_protocol->m_req_id);

    service->CallMethod(method, &rpcController, req_msg, rsp_msg, nullptr);

    rsp_protocol->m_req_id = req_protocol->m_req_id;
    rsp_protocol->m_method_name = req_protocol->m_method_name;

    if (!rsp_msg->SerializeToString(&(rsp_protocol->m_pb_data))) {
        ERRORLOG("%s | serilize error, origin message [%s]", req_protocol->m_req_id.c_str(), rsp_msg->ShortDebugString().c_str());
        setTinyPBError(rsp_protocol, ERROR_FAILED_SERIALIZE, "serilize error");
        if (req_msg) {
            delete req_msg;
            req_msg = nullptr;
        }
        if (rsp_msg) {
            delete rsp_msg;
            rsp_msg = nullptr;
        }
        return;
    }

    rsp_protocol->m_err_code = 0;

    INFOLOG("%s | dispatch success requset [%s], response [%s]", req_protocol->m_req_id.c_str(),
        req_msg->ShortDebugString().c_str(), rsp_msg->ShortDebugString().c_str());

    delete req_msg;
    delete rsp_msg;
    req_msg = nullptr;
    rsp_msg = nullptr;
}

// 将一个服务注册 
void RpcDispatcher::registerService(service_s_ptr service) {
    std::string service_name = service->GetDescriptor()->full_name();
    DEBUGLOG("service full name [%s]", service_name.c_str());
    m_service_map[service_name] = service;
}

// 解析服务的名字，因为一个方法通常是以servic.method的形式出现的，因此要正确的解析
bool RpcDispatcher::parseServiceFullName(const std::string &full_name, std::string& service_name, std::string& method_name) {
    size_t i = 0;
    i = full_name.find_first_of(".");
    if (i == std::string::npos) {
        ERRORLOG("not find . in full name [%s], full name error", full_name.c_str());
        return false;
    }

    service_name = full_name.substr(0,i);
    method_name = full_name.substr(i + 1);
    DEBUGLOG("parse service name [%s], method name [%s]", service_name.c_str(), method_name.c_str());
    return true;
}

void RpcDispatcher::setTinyPBError(std::shared_ptr<TinyPBProtocol> msg, int32_t err_code, const std::string err_info) {
    msg->m_err_code = err_code;
    msg->m_err_info = err_info;
    msg->m_err_info_len = err_info.size();
}



}