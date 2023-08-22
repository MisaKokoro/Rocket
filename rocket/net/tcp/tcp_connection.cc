#include <unistd.h>
#include <string.h>
#include "rocket/net/tcp/tcp_connection.h"
#include "rocket/net/fd_event_group.h"
#include "rocket/common/log.h"
#include "rocket/net/coder/string_coder.h"

namespace rocket {

TcpConnection::TcpConnection(EventLoop* event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr, TcpConnectionType type) 
    : m_event_loop(event_loop), m_fd(fd), m_peer_addr(peer_addr), m_state(NotConnected), m_connection_type(type) {
    m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
    m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

    // 将对端的套接字加入fdgroup ，并为其注册读回调函数
    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdevent(fd);
    m_fd_event->setNonBlock();
    m_coder = new TinyPBCoder();

    // 服务端需要主动监听读事件
    if (m_connection_type == TcpConnectionByServer) {
        listenRead();
    }
}

TcpConnection::~TcpConnection() {
    DEBUGLOG("addr TcpConnection %s destory", m_peer_addr->toString().c_str());
    if (m_coder) {
        delete m_coder;
        m_coder = nullptr;
    }
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
            DEBUGLOG("success read %d bytes from addr[%s]", rt, m_peer_addr->toString().c_str());
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

    if (m_connection_type == TcpConnectionByClient) {
        // 先将所有的消息全部取出来
        std::vector<AbstractProtocol::s_ptr> messages;
        for (auto &write_done : m_write_dones) {
            messages.push_back(write_done.first);
        }
        // 然后将消息转化为字节流
        m_coder->encode(messages, m_out_buffer);
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
        m_event_loop->addEpollEvent(m_fd_event);
    }

    //TODO 数据发送完之后执行回调函数，为什么只考虑客户端连接的情况？
    if (m_connection_type == TcpConnectionByClient) {
        for (auto &write_done : m_write_dones) {
            auto cb = write_done.second;
            if (cb) {
                cb(write_done.first);
            }
        }
        m_write_dones.clear();
    }
}

// 将rpc请求执行业务逻辑， 获取RPC响应，再把RPC响应发送出去
void TcpConnection::execute() {
    // TODO 这里为什么要分为客户端和服务端
    if (m_connection_type == TcpConnectionByServer) {
        // std::vector<char> tmp;
        // m_in_buffer->readFromBuffer(tmp, m_in_buffer->readAble());
        // std::string msg(tmp.begin(), tmp.end());
        // m_out_buffer->writeToBuffer(&tmp[0], tmp.size());
        // 将收到的消息解码
        std::vector<AbstractProtocol::s_ptr> receive_messages;
        std::vector<AbstractProtocol::s_ptr> repliy_messages;
        m_coder->decode(receive_messages, m_in_buffer);

        for (auto &receive_message : receive_messages) {
            INFOLOG("success get requst_id [%s] from client [%s]", receive_message->m_req_id.c_str(),
            m_peer_addr->toString().c_str());
            // 这里回复的消息先回复固定格式
            auto message = std::make_shared<TinyPBProtocol>();
            message->m_pb_data = "hello. this is rocket rpc test data";
            message->m_req_id = receive_message->m_req_id;
            repliy_messages.emplace_back(message);
        }

        // 将要发送的消息编码，并将消息发出
        m_coder->encode(repliy_messages, m_out_buffer);
        listenWrite();
    } else {
        // 将接受的message解码
        std::vector<AbstractProtocol::s_ptr> result;
        m_coder->decode(result, m_in_buffer);

        for (auto &message : result) {
            std::string req_id = message->getReqId();
            auto it = m_read_dones.find(req_id);
            if (it != m_read_dones.end()) {
                it->second(message);
            }
        }
    }
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

    m_fd_event->cancel(FdEvent::IN_EVENT);
    m_fd_event->cancel(FdEvent::OUT_EVENT);

    m_event_loop->deleteEpollEvent(m_fd_event); // 从eventloop中将这个fd删除，不再监听
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

void TcpConnection::setConnectionType(TcpConnectionType connection_type) {
    m_connection_type = connection_type;
}

// 监听可写事件
void TcpConnection::listenWrite() {
    m_fd_event->listen(FdEvent::OUT_EVENT, std::bind(&TcpConnection::onWrite, this));
    m_event_loop->addEpollEvent(m_fd_event);
}
// 监听可写事件
void TcpConnection::listenRead() {
    m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::onRead, this));
    m_event_loop->addEpollEvent(m_fd_event);
} 

void TcpConnection::pushSendMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done) {
    m_write_dones.emplace_back(message, done);
}

void TcpConnection::pushReadMessage(const std::string &req_id, std::function<void(AbstractProtocol::s_ptr)> done) {
    m_read_dones.emplace(req_id, done);
}

}