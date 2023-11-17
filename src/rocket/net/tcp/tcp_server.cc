#include "rocket/net/tcp/tcp_server.h"
#include "rocket/common/log.h"
#include "rocket/net/tcp/tcp_connection.h"
#include "rocket/net/fd_event_group.h"

namespace rocket {
TcpServer::TcpServer(NetAddr::s_ptr local_addr, int io_thread_nums) : 
    m_local_addr(local_addr), m_io_thread_nums(io_thread_nums) {
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
    m_io_thread_group = new IOThreadGroup(m_io_thread_nums);
    INFOLOG("tcp server success listen on [%s]", m_local_addr->toString().c_str());

    m_timer_event = std::make_shared<TimerEvent>(60000, true, std::bind(&TcpServer::clearConnection, this));
    m_listen_fd_event = FdEventGroup::GetFdEventGroup()->getFdevent(m_acceptor->getListenFd());
    m_listen_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpServer::onAccept, this));
    m_main_event_loop->addEpollEvent(m_listen_fd_event);
    m_main_event_loop->addTimerEvent(m_timer_event);
}

void TcpServer::onAccept() {
    auto re = m_acceptor->accept();
    int client_fd = re.first;
    NetAddr::s_ptr peer_addr = re.second;

    m_client_counts++;

    // 把client_id添加到任意io线程里面
    IOThread* io_thread = m_io_thread_group->getIOThread();
    TcpConnection::s_ptr connection = std::make_shared<TcpConnection>(io_thread->getEventLoop(), client_fd, 128, m_local_addr, peer_addr);
    connection->setState(Connected);
    // TODO 缺少当连接关闭时的清理动作
    m_client.insert(connection);
    INFOLOG("TcpServer success get client fd = %d, peer addr = %s", client_fd, peer_addr->toString().c_str());
}

void TcpServer::clearConnection() {
    DEBUGLOG("client size = %lld, begin to clear connection", m_client.size());
    auto it = m_client.begin();
    // 删除set中的元素时要注意迭代器失效的问题，不要在循环里面自增
    for (; it != m_client.end(); ) {
        if ((*it)->getState() == Closed) {
            it = m_client.erase(it);
        } else {
            it++;
        }
    }
    DEBUGLOG("end to clear connection");
}
}