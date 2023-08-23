#ifndef ROCKET_NET_RPC_RPC_DISPATCHER_H
#define ROCKET_NET_RPC_RPC_DISPATCHER_H

#include <string>
#include <memory>
#include <map>
#include <google/protobuf/service.h>
// #include "rocket/net/tcp/tcp_connection.h"
#include "rocket/net/coder/abstract_protocol.h"
#include "rocket/net/coder/tinypb_protocol.h"

namespace rocket {
class TcpConnection; // tcp_connection.h与这个头文件相互依赖了，需要一个前置定义
class RpcDispatcher {
public:
    using service_s_ptr = std::shared_ptr<google::protobuf::Service>;

    void dispatcher(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr response, TcpConnection* connection);

    void registerService(service_s_ptr service);

    bool parseServiceFullName(const std::string &full_name, std::string& service_name, std::string& method_name);

    void setTinyPBError(std::shared_ptr<TinyPBProtocol> msg, int32_t err_code, const std::string err_info);
private:
    std::map<std::string, service_s_ptr> m_service_map;
};

}
#endif