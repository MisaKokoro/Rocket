#ifndef ROCKET_NET_TCP_TCP_SERVER_H
#define ROCKET_NET_TCP_TCP_SERVER_H
#include <set>
#include "rocket/net/io_thread_group.h"
#include "rocket/net/tcp/tcp_acceptor.h"
#include "rocket/net/tcp/tcp_connection.h"

namespace rocket {
// TcpSever 是一个全局的单例对象，只能在主线程里面构建
class TcpServer {
public:
    TcpServer(NetAddr::s_ptr local_addr);

    ~TcpServer();
    
    void start();
private:
    void init();

    void onAccept();

private:
    TcpAcceptor::s_ptr m_acceptor; // acceptor 负责建立连接

    NetAddr::s_ptr m_local_addr; // 本地监听的地址

    EventLoop* m_main_event_loop {nullptr}; // main Reactor

    IOThreadGroup* m_io_thread_group {nullptr}; // subReactor

    FdEvent *m_listen_fd_event {nullptr};

    int m_client_counts {0};

    std::set<TcpConnection::s_ptr> m_client;
};
}
#endif 