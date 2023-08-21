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


    m_connection = std::make_shared<TcpConnection>(m_event_loop, m_fd, 128, m_peer_addr);
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
                if (error == 0) {
                    DEBUGLOG("connect success, peer_addr[%s]", m_peer_addr->toString().c_str());
                    if (done) {
                        done();
                    }
                } else {
                    ERRORLOG("connect error , errno = %d, error:%s", errno, strerror(errno));
                }

                // TODO 需要去掉可写事件， 不然会一直触发?
                m_fd_event->cancel(FdEvent::OUT_EVENT);
                m_event_loop->addEpollEvent(m_fd_event);
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

}

// 异步的读取message
// 读取=message成功则会执行回调函数
void TcpClient::readMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done) {

}

}