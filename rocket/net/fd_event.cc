#include <string.h>
#include "rocket/net/fd_event.h"

namespace rocket {
    FdEvent::FdEvent(int fd) : m_fd(fd) {
        memset(&m_listen_events, 0, sizeof(m_listen_events));
    }
    FdEvent::~FdEvent(){

    }

    std::function<void()> FdEvent::handler(TriggerEvent event) {
        if (event == IN_EVENT) {
           return m_read_callback;
        } else {
           return m_write_callback;
        }
    }
     
    // 监听读事件或者写事件并设置相应的回调函数
    void FdEvent::listen(TriggerEvent event, std::function<void()> callback) {
        if (event == IN_EVENT) {
            m_listen_events.events |= EPOLLIN;
            m_read_callback = callback;
        } else {
            m_listen_events.events |= EPOLLOUT;
            m_write_callback = callback;
        }
        // TODO 为啥这里的data可以用来存数据，它本来是干什么用的
        m_listen_events.data.ptr = this; // 将FdEvnet指针传出去

    }
} // namespace rocket

