#ifndef ROCKET_NET_FD_EVENT_H
#define ROCKET_NET_FD_EVENT_H

#include <functional>
#include <sys/epoll.h>


namespace rocket {

class FdEvent {
public:
    enum TriggerEvent {
        IN_EVENT = EPOLLIN,
        OUT_EVENT = EPOLLOUT,
    };

    FdEvent(int fd);
    FdEvent();
    ~FdEvent();

    std::function<void()> handler(TriggerEvent event);
    void listen(TriggerEvent event, std::function<void()> callback); // 这里的listen类似register函数，注册一个度或者写事件

    int getFd() const {
        return m_fd;
    }

    epoll_event getEpollEvent() {
        return m_listen_events;
    }

protected:
    int m_fd{-1};

    epoll_event m_listen_events;
    std::function<void()> m_read_callback;
    std::function<void()> m_write_callback;
};

} // namespace rocket

#endif