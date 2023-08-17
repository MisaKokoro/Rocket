#ifndef ROCKET_NET_EVENTLOOP_H
#define ROCKET_NET_EVENTLOOP_H
#include <sys/types.h>
#include <sys/epoll.h>
#include <set>
#include <queue>
#include <functional>
#include "rocket/common/mutex.h"
#include "rocket/net/fd_event.h"
#include "rocket/net/wakeup_fd_event.h"
#include "rocket/net/timer.h"


namespace rocket {

class EventLoop {
public:
    EventLoop();

    ~EventLoop();

    void loop();

    void stop();

    void wakeup();

    void addEpollEvent(FdEvent *event);

    void deleteEpollEvent(FdEvent *event);

    bool isInLoopThread();

    void addTask(std::function<void()> cb, bool is_wake_up = false); 

    void addTimerEvent(TimerEvent::s_ptr event);

private:
    void dealWekeup();

    void initWakeupFdEvent();

    void initTimer();

private:
    pid_t m_thread_id {0};

    int m_epoll_fd {0};

    int m_wakeup_fd {0}; // 一个eventfd， 用于唤醒epoll_wait

    bool m_stop_flag {false};

    std::set<int> m_listen_fds;

    std::queue<std::function<void()>> m_pending_tasks;

    WakeUpFdEvent *m_wamkeup_fd_event {nullptr};

    Timer *m_timer {nullptr};

    Mutex m_mutex;


};
} // namespace rocket

#endif