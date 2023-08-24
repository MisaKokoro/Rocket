#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "rocket/net/tcp/tcp_client.h"
#include "rocket/common/log.h"
#include "rocket/net/fd_event_group.h"
namespace rocket {
TcpClient::TcpClient(NetAddr::s_ptr peer_addr) : m_peer_addr(peer_addr) {
    m_event_loop = EventLoop::GetCurrentEventLoop();
    m_fd = socket(peer_addr->getFamily(), SOCK_STREAM, 0);
    if (m_fd < 0) {
        ERRORLOG("TcpClient::TcpClient() failed, socket create failed");
        return;
    }

    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdevent(m_fd);
    m_fd_event->setNonBlock();


    m_connection = std::make_shared<TcpConnection>(m_event_loop, m_fd, 128,nullptr, m_peer_addr, TcpConnectionByClient);
    m_connection->setConnectionType(TcpConnectionByClient);
}

TcpClient::~TcpClient() {
    if (m_fd > 0) {
        close(m_fd);
    }
}

//异步的进行connect
// connect成功会执行回调函数
void TcpClient::connect(std::function<void()> done) {
    int rt = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
    if (rt == 0) {
        // 连接成功执行回调函数
        if (done) {
            done();
        }
        DEBUGLOG("connect success, peer_addr[%s]", m_peer_addr->toString().c_str());
    } else if (rt == -1) {
        if (errno == EINPROGRESS) {
            // 错误码是EINPROGRESS时表示连接正在进行中，需要根据getsocket函数来判断是否连接成功
            m_fd_event->listen(FdEvent::OUT_EVENT, [this, done]() {
                int error = 0;
                socklen_t error_len = sizeof(error);
                getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &error, &error_len);

                bool is_connect_success = false;
                if (error == 0) {
                    DEBUGLOG("connect success, peer_addr[%s]", m_peer_addr->toString().c_str());
                    is_connect_success = true;
                    m_connection->setState(Connected);
                } else {
                    ERRORLOG("connect error , errno = %d, error:%s", errno, strerror(errno));
                }

                // TODO 需要去掉可写事件， 不然会一直触发?
                // 这里去掉的是连接成功后的写回调
                m_fd_event->cancel(FdEvent::OUT_EVENT);
                m_event_loop->addEpollEvent(m_fd_event);

                // 这里done调到下面来执行，不然上面的取消写回调会导致done无法把message写完后执行回调函数
                if (is_connect_success && done) {
                    done();
                }
            });
            // 将事件添加，并且开启loop
            m_event_loop->addEpollEvent(m_fd_event);
            if (!m_event_loop->isLooping()) {
                m_event_loop->loop();
            }
        } else {
            ERRORLOG("connect error , errno = %d, error:%s", errno, strerror(errno));
        }
    }
}

// 异步的发送message
// 发送message成功则会执行回调函数
void TcpClient::writeMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done) {
    
    // 1. 把message以及其对应的回调写入
    // 2. connection监听可写事件
    m_connection->pushSendMessage(message, done);
    m_connection->listenWrite();
}

// 异步的读取message
// 读取=message成功则会执行回调函数
void TcpClient::readMessage(const std::string& message, std::function<void(AbstractProtocol::s_ptr)> done) {
    // 监听可读事件
    // 从buffer里decode得到的message对象， 判断是否与msg_id相等，若相等执行相应的回调
    m_connection->pushReadMessage(message, done);
    m_connection->listenRead();
}

}