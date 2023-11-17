#include <sys/timerfd.h>
#include <string.h>
#include <functional>
#include "rocket/net/timer.h"
#include "rocket/common/log.h"
#include "rocket/common/util.h"

namespace rocket {
Timer::Timer() : FdEvent() {
    // TODO timerfd相关内容，TFD_CLOEXEC这些都是什么含义？
    // 如果文件描述符的 CLOEXEC 标志被设置为 1，那么在执行 exec 函数时，该文件描述符将会被关闭，不会被继承到新的进程中。
    m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    if (m_fd == -1) {
        ERRORLOG("create timer failed, timerfd create failed, erron = %d, error: %s", errno, strerror(errno));
        exit(1);
    }
    // 判断是否创建成功？

    DEBUGLOG("timer fd = %d", m_fd);

    // TODO 使用bind包装函数，或者使用function包装函数与函数指针的区别是什么？
    listen(FdEvent::IN_EVENT,std::bind(&Timer::onTimer, this));
}

Timer::~Timer() {

}

void Timer::onTimer() {
    // 先把收到的读请求解决掉，以便定时器可以再次触发
    char buf[8];
    while (true) {
       if ((read(m_fd, buf, 8) == -1) && errno == EAGAIN) {
            break;
        }
    }

    int64_t now = getNowMs();
    std::vector<TimerEvent::s_ptr> tmps;
    std::queue<std::function<void()>> tasks;

    ScopeMutex<Mutex> lock(m_mutex);
    auto it = m_pending_events.begin();
    // 这里一开始在for里面定义了 auto it = m_pending_events.begin()导致上面那个it一直没有变化，然后就重复打印
    for (it = m_pending_events.begin(); it != m_pending_events.end(); ++it) {
        // 如果当前任务需要被执行，并且没有被取消执行，则将其加入到执行队列
        if (now >= it->first) {
            if (!it->second->isCanceled()) {
                tmps.push_back(it->second);
                tasks.push(it->second->getCallBack());
            }
        } else {
            break;
        }
    }
    // 将已经执行过的任务删除

    m_pending_events.erase(m_pending_events.begin(), it);
    lock.unlock();

    // 需要重复执行的任务还需要再次添加进去，注意这些任务需要重新调整arriveTime
    for (auto timer_event : tmps) {
        if (timer_event->isRepeated()) {
            timer_event->resetArriveTime();
            addTimerEvent(timer_event);
        }
    }
    // DEBUGLOG("pending task size = %d", m_pending_events.size());
    resetArriveTime(); 
    // 执行所有定时任务
    // DEBUGLOG("tasks size = %d", tasks.size());
    while (!tasks.empty()) {
        auto cb = tasks.front();
        tasks.pop();
        if (cb) {
            cb();
        }
    }
}

void Timer::resetArriveTime() {
    ScopeMutex<Mutex> lock(m_mutex);
    if (m_pending_events.empty()) {
        return;
    }
    // 得到第一个任务，也就是最早执行的任务
    auto first_task = *m_pending_events.begin();
    lock.unlock();

    int64_t now = getNowMs();
    int64_t interval = 0;
    // 比较最早执行的任务与当前时间的大小，如果还没到执行最早的任务，那么更新间隔
    if (now < first_task.second->getArriveTime()) {
        interval = first_task.second->getArriveTime() - now;
    } else { // 最早的任务早就应该执行，设置100ms后执行
        interval = 100;
    }

    // 计算定时器应该多长时间后触发
    timespec value;
    memset(&value, 0, sizeof(value));
    value.tv_sec = interval / 1000;
    value.tv_nsec = (interval % 1000) * 1000000;

    itimerspec timer_interval;
    memset(&timer_interval, 0 , sizeof (timer_interval));
    timer_interval.it_value = value;
    int rt = timerfd_settime(m_fd, 0, &timer_interval, nullptr);
    if (rt == -1) {
        ERRORLOG("timerfd settime failed, errno = %d, error:%s",  errno, strerror(errno));
    }
}

void Timer::addTimerEvent(TimerEvent::s_ptr event) {
    bool is_reset_timerfd = false;
    ScopeMutex<Mutex> lock(m_mutex);
    // 如果还没有时间加入，那么新加入的时间需要重置timerfd的触发时间
    if (m_pending_events.empty()) {
        is_reset_timerfd = true;
    } else {
        auto it = m_pending_events.begin();
        // 如果新加入的event的触发时间比当前队列里面所有的event触发时间都短，那么需要将timerfd的触发时间更新
        if (event->getArriveTime() < it->second->getArriveTime()) {
            is_reset_timerfd = true;
        }
    }
    m_pending_events.emplace(event->getArriveTime(), event);
    lock.unlock();

    if (is_reset_timerfd) {
        resetArriveTime();
    }
}

// 从定时器中删除一个事件
void Timer::deleteTimerEvent(TimerEvent::s_ptr event) {
    // 首先将当前定时事件设置为取消执行
    event->setCanceled(true);
    ScopeMutex<Mutex> lock(m_mutex);
    auto begin = m_pending_events.lower_bound(event->getArriveTime());
    auto end = m_pending_events.upper_bound(event->getArriveTime());

    auto it = begin;
    for (it = begin; it != end; it++) {
        if (it->second == event) {
            break;
        }
    }

    if (it != end) {
        m_pending_events.erase(it);
    }
    lock.unlock();
    DEBUGLOG("success delete event, event arrive time %lld", event->getArriveTime());
}


}