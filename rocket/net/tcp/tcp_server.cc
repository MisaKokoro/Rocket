#include "rocket/net/tcp/tcp_server.h"
#include "rocket/common/log.h"
#include "rocket/net/tcp/tcp_connection.h"
#include "rocket/net/fd_event_group.h"

namespace rocket {
TcpServer::TcpServer(NetAddr::s_ptr local_addr) : m_local_addr(local_addr) {
   init();
}

TcpServer::~TcpServer() {
    if (m_main_event_loop) {
        delete m_main_event_loop;
        m_main_event_loop = nullptr;
    }
}

// 启动io线程组的循环和主reactor的循环
void TcpServer::start() {
    m_io_thread_group->start();
    m_main_event_loop->loop();
}

void TcpServer::init() {
    m_acceptor = std::make_shared<TcpAcceptor>(m_local_addr);
    m_main_event_loop = EventLoop::GetCurrentEventLoop();
    m_io_thread_group = new IOThreadGroup(2);
    INFOLOG("tcp server success listen on [%s]", m_local_addr->toString().c_str());

    m_listen_fd_event = FdEventGroup::GetFdEventGroup()->getFdevent(m_acceptor->getListenFd());
    m_listen_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpServer::onAccept, this));
    m_main_event_loop->addEpollEvent(m_listen_fd_event);
}

void TcpServer::onAccept() {
    auto re = m_acceptor->accept();
    int client_fd = re.first;
    NetAddr::s_ptr peer_addr = re.second;

    m_client_counts++;

    // 把client_id添加到任意io线程里面
    IOThread* io_thread = m_io_thread_group->getIOThread();
    TcpConnection::s_ptr connection = std::make_shared<TcpConnection>(io_thread->getEventLoop(), client_fd, 128, peer_addr, m_local_addr);
    connection->setState(Connected);
    m_client.insert(connection);
    INFOLOG("TcpServer success get client fd = %d, peer addr = %s", client_fd, peer_addr->toString().c_str());
}

}