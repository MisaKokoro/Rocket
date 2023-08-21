#ifndef ROCKET_NET_TCP_TCP_CONNECTION_H
#define ROCKET_NET_TCP_TCP_CONNECTION_H

#include <memory>
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_buffer.h"
#include "rocket/net/tcp/tcp_acceptor.h"
#include "rocket/net/io_thread.h"
#include "rocket/net/fd_event.h"

namespace rocket {
enum TcpState {
    NotConnected = 1,
    Connected = 2,
    HalfClosing = 3,
    Closed = 4,
};

enum TcpConnectionType {
    TcpConnectionByServer = 1, // 作为服务端使用， 代表对端客户端连接
    TcpConnectionByClient = 2, // 作为客户端使用， 代表对端服务器连接
};
class TcpConnection {
public:
    using s_ptr = std::shared_ptr<TcpConnection>;
public:
    TcpConnection(EventLoop* event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr);

    ~TcpConnection();

    void onRead();

    void onWrite();

    void execute();

    void setState(const TcpState state);

    void clear();

    void shutdown();

    void setConnectionType(TcpConnectionType type);

    TcpState getState();

private:
    NetAddr::s_ptr m_local_addr;
    NetAddr::s_ptr m_peer_addr;

    TcpBuffer::s_ptr m_in_buffer; // 负责写入对端发送过来的内容
    TcpBuffer::s_ptr m_out_buffer; // 负责存将要发送给对端的内容

    EventLoop* m_event_loop {nullptr};

    FdEvent* m_fd_event {nullptr};

    TcpState m_state;

    int m_fd {-1};

    TcpConnectionType m_connection_type {TcpConnectionByServer};

};

}
#endif 