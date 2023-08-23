#ifndef ROCKET_NET_TCP_TCP_CONNECTION_H
#define ROCKET_NET_TCP_TCP_CONNECTION_H

#include <memory>
#include <queue>
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_buffer.h"
#include "rocket/net/tcp/tcp_acceptor.h"
#include "rocket/net/io_thread.h"
#include "rocket/net/fd_event.h"
#include "rocket/net/coder/tinypb_coder.h"
#include "rocket/net/coder/tinypb_protocol.h"
#include "rocket/net/rpc/rpc_dispatcher.h"

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
    TcpConnection(EventLoop* event_loop, int fd, int buffer_size, NetAddr::s_ptr local_addr ,NetAddr::s_ptr peer_addr, TcpConnectionType type = TcpConnectionByServer);

    ~TcpConnection();

    void onRead();

    void onWrite();

    void execute();

    void setState(const TcpState state);

    void clear();

    void shutdown();

    void setConnectionType(TcpConnectionType type);

    void listenWrite(); // 监听可写事件

    void listenRead(); // 监听可写事件

    TcpState getState();

    void pushSendMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);

    void pushReadMessage(const std::string &req_id, std::function<void(AbstractProtocol::s_ptr)> done);

    NetAddr::s_ptr getLocalAddr();

    NetAddr::s_ptr getPeerAddr();

private:
    EventLoop* m_event_loop {nullptr};

    int m_fd {-1};
    
    NetAddr::s_ptr m_local_addr;
    NetAddr::s_ptr m_peer_addr;

    TcpBuffer::s_ptr m_in_buffer; // 负责写入对端发送过来的内容
    TcpBuffer::s_ptr m_out_buffer; // 负责存将要发送给对端的内容

    FdEvent* m_fd_event {nullptr};

    TcpState m_state;

    TcpConnectionType m_connection_type {TcpConnectionByServer};

    std::vector<std::pair<AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)>>> m_write_dones; 

    std::map<std::string, std::function<void(AbstractProtocol::s_ptr)>> m_read_dones;

    AbstractCoder* m_coder {nullptr};

    std::shared_ptr<RpcDispatcher> m_dispatcher;

};

}
#endif 