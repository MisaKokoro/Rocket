#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "rocket/net/tcp/tcp_client.h"
#include "rocket/common/log.h"
#include "rocket/net/fd_event_group.h"
#include "rocket/common/error_code.h"

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
// connect 完成之后才应该执行回调函数，
void TcpClient::connect(std::function<void()> done) {
    int rt = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
    if (rt == 0) {
        // 连接成功执行回调函数
        DEBUGLOG("connect success, peer_addr[%s]", m_peer_addr->toString().c_str());
        initLocalAddr();
        m_connection->setState(Connected);
        if (done) {
            done();
        }

    } else if (rt == -1) {
        if (errno == EINPROGRESS) {
            m_fd_event->listen(FdEvent::OUT_EVENT, [this, done]() {
                int rt = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
                if ((rt < 0 && errno == EISCONN) || rt == 0) {
                    DEBUGLOG("connect [%s] success", m_peer_addr->toString().c_str());
                    initLocalAddr();
                    m_connection->setState(Connected);
                } else {
                    if (errno == ECONNREFUSED) {
                        m_connect_errcode = ERROR_PEER_CLOSED;
                        m_connect_error_info = "connect error, sys error =" + std::string(strerror(errno));
                    } else {
                        m_connect_errcode = ERROR_FAILED_CONNECT;
                        m_connect_error_info = "connect error, sys error =" + std::string(strerror(errno));
                    }
                    ERRORLOG("connect error errno = %d, error:%s", errno, strerror(errno));
                    // TODO 如果关闭套接字，再创建一个，这里的m_fd_event已经不和m_fd绑定了，应该如何处理？
                    // close(m_fd);
                    // m_fd = socket(m_peer_addr->getFamily(), SOCK_STREAM, 0);
                }

                m_event_loop->deleteEpollEvent(m_fd_event);

                if (done) {
                    done();
                }
            }
            );
            // 将事件添加，并且开启loop
            m_event_loop->addEpollEvent(m_fd_event);
            if (!m_event_loop->isLooping()) {
                m_event_loop->loop();
            }
        } else {
            ERRORLOG("connect error , errno = %d, error:%s", errno, strerror(errno));
            if (done) {
                done();
            }
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

void TcpClient::stop() {
    if (m_event_loop->isLooping()) {
        m_event_loop->stop();
    }
}

int TcpClient::getConnectErrorCode() {
    return m_connect_errcode;
}

std::string TcpClient::getConnectErrorInfo() {
    return m_connect_error_info;
}

NetAddr::s_ptr TcpClient::getLocalAddr() {
    return m_local_addr;
}

NetAddr::s_ptr TcpClient::getPeerAddr() {
    return m_peer_addr;
}
// 获取连接成功后的本地地址
void TcpClient::initLocalAddr() {
    struct sockaddr_in localAddr;
    socklen_t addrLen = sizeof(localAddr);
    int ret = getsockname(m_fd, reinterpret_cast<sockaddr*>(&localAddr), &addrLen);
    if (ret != 0) {
        ERRORLOG("get local addr failed, errno = %d, error : %s", errno, strerror(errno));
        return;
    }
    m_local_addr = std::make_shared<IPNetAddr>(localAddr);
}

}