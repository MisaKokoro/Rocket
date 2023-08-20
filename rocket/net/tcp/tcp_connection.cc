#include <unistd.h>
#include <string.h>
#include "rocket/net/tcp/tcp_connection.h"
#include "rocket/net/fd_event_group.h"
#include "rocket/common/log.h"

namespace rocket {

TcpConnection::TcpConnection(IOThread* io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr) 
    : m_io_thread(io_thread), m_peer_addr(peer_addr), m_state(NotConnected), m_fd(fd) {
    m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
    m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

    // 将对端的套接字加入fdgroup ，并为其注册读回调函数
    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdevent(fd);
    m_fd_event->setNonBlock();
    m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::onRead, this));

    m_io_thread->getEventLoop()->addEpollEvent(m_fd_event);
}

TcpConnection::~TcpConnection() {
    DEBUGLOG("addr TcpConnection %s destory", m_peer_addr->toString().c_str());
}

// 当一个连接上有可读请求时，要把该连接上的数据读出来
void TcpConnection::onRead() {
    if (m_state != Connected) {
        ERRORLOG("onRead CallBack Error, Client not in connected state, addr[%s], clientfd[%d]",
        m_peer_addr->toString().c_str(), m_fd);
        return;
    }

    bool is_read_all = false;
    bool is_close = false;
    while (!is_read_all) {
        // 当buffer已经写满时对其进行扩容
        if (m_in_buffer->writeAble() == 0) {
            m_in_buffer->resizeBuffer(2 * m_in_buffer->m_buffer.size());
        }

        int read_count = m_in_buffer->writeAble(); // 可以读入多少数据
        int write_index = m_in_buffer->writeIndex();

        int rt = read(m_fd, &(m_in_buffer->m_buffer[write_index]), read_count);
        if (rt > 0) {
            m_in_buffer->moveWriteIndex(rt);
            // 读的数据全部写到buffer里面了，但是数据可能还没有读完
            if (rt == read_count) {
                continue;
            } else if (rt < read_count) {
                is_read_all = true;
                break;
            } 
        } else if (rt == 0) {
            // rt == 0代表对端关闭了连接
            is_close = true;
            break;
        } else if (rt == -1 && errno == EAGAIN) {
            // TODO 表示已经没有数据可读了?? read函数在面对非阻塞fd时的表现
            is_read_all = true;
            break;
        }
    }

    if (is_close) {
        // TODO 处理关闭连接
        INFOLOG("peer closed, peer addr [%s], client fd [%d]", m_peer_addr->toString().c_str(), m_fd);
        clear();
        return;
    }

    if (!is_read_all) {
        ERRORLOG("data not read all");
    }

    // TODO 处理接收到的信息
    execute();
}

// 将当前outbuffer中的数据全部发送给client
void TcpConnection::onWrite() {
    if (m_state != Connected) {
        ERRORLOG("onWrite CallBack Error, Client not in connected state, addr[%s], clientfd[%d]",
        m_peer_addr->toString().c_str(), m_fd);
        return;
    }

    // 开始发送数据
    bool is_write_all = false;
    while (true) {
        if (m_out_buffer->readAble() == 0) {
            is_write_all = true;
            DEBUGLOG("No Data Need To Send To Client[%s]", m_peer_addr->toString().c_str());
            break;
        }

        int write_size = m_out_buffer->readAble();
        int read_index = m_out_buffer->readIndex();
        int rt = write(m_fd, &m_out_buffer->m_buffer[read_index], write_size);
        if (rt >= write_size) {
            m_out_buffer->moveReadIndex(rt);
            is_write_all = true;
            DEBUGLOG("No Data Need To Send To Client[%s]", m_peer_addr->toString().c_str());
            break;
        } 
        if (rt < 0 && errno == EAGAIN) {
            // 发送缓存区已经满了，无法再次发送
            // 这种情况等待下次fd可写时发送数据即可
            ERRORLOG("write data error errno = EAGIN, error:%s", strerror(errno));
            break;
        }
    }

    // 如果已经写完的话，就不要继续监听写事件了
    if (is_write_all) {
        m_fd_event->cancel(FdEvent::OUT_EVENT);
        m_io_thread->getEventLoop()->addEpollEvent(m_fd_event);
    }
}

// 将rpc请求执行业务逻辑， 获取RPC响应，再把RPC响应发送出去
void TcpConnection::execute() {
    std::vector<char> tmp;
    m_in_buffer->readFromBuffer(tmp, m_in_buffer->readAble());
    std::string msg(tmp.begin(), tmp.end());

    INFOLOG("success get requst [%s] from client [%s]", msg.c_str(),m_peer_addr->toString().c_str());
    
    m_out_buffer->wirteToBuffer(&tmp[0], tmp.size());
    m_fd_event->listen(FdEvent::OUT_EVENT, std::bind(&TcpConnection::onWrite, this));
    m_io_thread->getEventLoop()->addEpollEvent(m_fd_event);
}

void TcpConnection::setState(const TcpState state) {
    m_state = state;
}


TcpState TcpConnection::getState() {
    return m_state;
}

// 处理服务器关闭连接后的清理动作
void TcpConnection::clear() {
    if (m_state == Closed) {
        return;
    }

    DEBUGLOG("now will delete fd event fd = %d", m_fd_event->getFd());
    m_io_thread->getEventLoop()->deleteEpollEvent(m_fd_event); // 从eventloop中将这个fd删除，不再监听
    m_state = Closed;
}

void TcpConnection::shutdown() {
    if (m_state == Closed || m_state == NotConnected) {
        return;
    }
    // TODO 将状态设置为半关闭状态？
    m_state = HalfClosing;

    // 调用 shutdown 关闭读写， 服务器就不会对这个fd进行读写操作
    // 发送 FIN 报文， 触发四次挥手
    // 当 fd 发生可读事件，但是可读数据为0， 对端发送了 FIN 报文
    ::shutdown(m_fd, SHUT_RDWR);
}

}