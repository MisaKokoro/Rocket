#ifndef ROCKET_NET_TCP_TCP_CLIENT_H
#define ROCKET_NET_TCP_TCP_CLIENT_H
#include <memory>
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_buffer.h"
#include "rocket/net/tcp/tcp_connection.h"
#include "rocket/net/fd_event.h"
#include "rocket/net/abstract_protocol.h"

namespace rocket {

class TcpClient {
public:
    TcpClient(NetAddr::s_ptr peer_addr);

    ~TcpClient();

    //异步的进行connect
    // connect成功会执行回调函数
    void connect(std::function<void()> done);

    // 异步的发送message
    // 发送message成功则会执行回调函数
    void writeMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);

    // 异步的读取message
    // 读取=message成功则会执行回调函数
    void readMessage(const std::string&, std::function<void(AbstractProtocol::s_ptr)> done);

private:
    NetAddr::s_ptr m_peer_addr;
    EventLoop* m_event_loop {nullptr};

    int m_fd {-1};
    FdEvent* m_fd_event {nullptr};

    TcpConnection::s_ptr m_connection;
};

}
#endif
