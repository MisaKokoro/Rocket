#include <sys/eventfd.h>
#include <string.h>
#include "rocket/net/eventloop.h"
#include "rocket/common/log.h"
#include "rocket/common/util.h"

// 只能用在addEpollEvent和deleteEpollEvent函数中
#define ADD_TO_EPOLL() \
auto it = m_listen_fds.find(event->getFd()); \
int op = EPOLL_CTL_ADD;\
if (it != m_listen_fds.end()) {\
    op = EPOLL_CTL_MOD;\
}\
epoll_event tmp = event->getEpollEvent();\
int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp);\
if (rt == -1) {\
    ERRORLOG("failed epoll_ctl when add fd %d, errno = %d, error:%s", event->getFd(),errno, strerror(errno));\
}\
m_listen_fds.insert(event->getFd()); \
DEBUGLOG("add event success, fd[%d]", event->getFd()); \

#define DELETE_TO_EPOLL() \
auto it = m_listen_fds.find(event->getFd()); \
int op = EPOLL_CTL_DEL;\
if (it != m_listen_fds.end()) {\
    return;\
}\
epoll_event tmp = event->getEpollEvent();\
int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp);\
if (rt == -1) {\
    ERRORLOG("failed epoll_ctl when delete fd %d, errno = %d, error:%s", event->getFd(),errno, strerror(errno));\
}\
m_listen_fds.erase(event->getFd()); \
DEBUGLOG("delete event success, fd[%d]", event->getFd()); \


namespace rocket {

static thread_local EventLoop* t_current_eventloop = nullptr;
static int g_epoll_max_timeout = 10000;
static int g_epoll_max_events = 10;

EventLoop::EventLoop() {
    if (t_current_eventloop != nullptr) {
        ERRORLOG("failed to create event loop, this thread has created event loop");
        exit(0);
    }

    m_thread_id = getThreadId();
    // 创建epollfd
    m_epoll_fd = epoll_create(10);
    if (m_epoll_fd == -1) {
        ERRORLOG("failed to create event loop, epoll create failed, error info[%d], error:%s", errno, strerror(errno));
        exit(0);
    }
    
    initWakeupFdEvent();
    initTimer();
    // 成功创建eventloop，并给当前线程的t_current_eventloop赋值
    INFOLOG("succ create event loop int thread %d", m_thread_id);
    t_current_eventloop = this;
}

EventLoop::~EventLoop() {
    // 关闭epoll
    close(m_epoll_fd);
    if (m_wamkeup_fd_event) {
        delete m_wamkeup_fd_event;
        m_wamkeup_fd_event = nullptr;
    }

    if (m_timer) {
        delete m_timer;
        m_timer = nullptr;
    }
}

void EventLoop::loop() {
    while (!m_stop_flag) {
        // 和日志那里的思想一样，将任务全部取出来再挨个执行,提高并发量
        ScopeMutex<Mutex> lock(m_mutex);
        std::queue<std::function<void()>> tmp_tasks;
        m_pending_tasks.swap(tmp_tasks);
        lock.unlock();

        while (!tmp_tasks.empty()) {
            auto cb = tmp_tasks.front();
            tmp_tasks.pop();
            if (cb) {
                cb();
            }

        }

        int timeout = g_epoll_max_timeout;
        epoll_event result_events[g_epoll_max_events];
        // DEBUGLOG("now begin epoll wait");
        int rt = epoll_wait(m_epoll_fd, &result_events[0], g_epoll_max_events, timeout);
        // DEBUGLOG("epoll wait end, rt = %d", rt);
        if (rt < 0) {
            ERRORLOG("epoll_wait error, errno = %d\n",errno);
        } else {
            for (int i = 0; i < rt; i++) {
                epoll_event trigger_event = result_events[i];
                FdEvent *fd_event = static_cast<FdEvent*>(trigger_event.data.ptr);
                if (fd_event == nullptr) {
                    continue;
                }
                DEBUGLOG("fd_event fd = %d, events = %d", fd_event->getFd(), fd_event->getEpollEvent().events);
                if (trigger_event.events & EPOLLIN) {
                    DEBUGLOG("fd[%d] trigger EPOLLIN event", fd_event->getFd());
                    addTask(fd_event->handler(FdEvent::IN_EVENT));
                }
                if (trigger_event.events & EPOLLOUT) {
                    DEBUGLOG("fd[%d] trigger EPOLLOUT event", fd_event->getFd());
                    addTask(fd_event->handler(FdEvent::OUT_EVENT));
                }
            }
        }

    }
}

void EventLoop::stop() {
    m_stop_flag = true;
}

// TODO这里为什么要是否是自己进行添加？
void EventLoop::addEpollEvent(FdEvent *event) {
    // 判断执行add操作的线程是否和eventloop的线程一致，如果一致则直接由本线程完成添加操作
    if (isInLoopThread()) {
        // 这个event是否已经添加，如果已经添加则进行修改操作
       ADD_TO_EPOLL()
    } else {
        // 如果不是本线程调用的这个函数，那么需要用一个callback来完成add操作
        auto cb = [this,event]() {
            ADD_TO_EPOLL();
        };
        addTask(cb);
    }
}

void EventLoop::deleteEpollEvent(FdEvent *event) {
    if (isInLoopThread()) {
       DELETE_TO_EPOLL()
    } else {
        auto cb = [this,event]() {
            DELETE_TO_EPOLL();
        };
        addTask(cb);
    }
}

void EventLoop::addTask(std::function<void()> cb, bool is_wake_up /*=false*/) {
    ScopeMutex<Mutex> lock(m_mutex);
    m_pending_tasks.push(cb);
    lock.unlock();
    
    if (is_wake_up) {
        wakeup();
    }
}

void EventLoop::addTimerEvent(TimerEvent::s_ptr event) {
    m_timer->addTimerEvent(event);
}


bool EventLoop::isInLoopThread() {
    return m_thread_id == getThreadId();
}

void EventLoop::wakeup() {
    m_wamkeup_fd_event->wakeup();
}

void EventLoop::initWakeupFdEvent() {
    // 创建wakeup fd，并且将其添加到epoll的监听集合
    // wakeup fd 创建为非阻塞
    m_wakeup_fd = eventfd(0, EFD_NONBLOCK);
    if (m_wakeup_fd == -1) {
        ERRORLOG("failed to create event loop, wakeup eventfd create failed, error info[%d], error:%s", errno, strerror(errno));
        exit(0);
    }
    m_wamkeup_fd_event = new WakeUpFdEvent(m_wakeup_fd);
    auto read_callback = [this]() {
        char buf[8];
        while (read(m_wakeup_fd, buf, 8) != -1 && errno != EAGAIN) {

        }
        DEBUGLOG("read full bytes from wakeup fd[%d]", m_wakeup_fd);
    };
    m_wamkeup_fd_event->listen(FdEvent::IN_EVENT,read_callback);
    addEpollEvent(m_wamkeup_fd_event);
}

void EventLoop::initTimer() {
    m_timer = new Timer();
    addEpollEvent(m_timer);
    INFOLOG("timer init succsee");
}

} // namespace rocket
