#ifndef ROCKET_NET_TIMER_EVENT_H
#define ROCKET_NET_TIMER_EVENT_H
#include <functional>
#include <memory>
namespace rocket {

class TimerEvent {
public:
    typedef std::shared_ptr<TimerEvent> s_ptr;

    TimerEvent(int interval, bool is_repeated, std::function<void()> cb);

    int64_t getArriveTime() const {
        return m_arrive_time;
    }

    void setCanceled(bool value) {
        m_is_canceled = value;
    }

    bool isCanceled() {
        return m_is_canceled;
    }

    bool isRepeated() {
        return m_is_repeated;
    }

    std::function<void()> getCallBack() {
        return m_task;
    }

    void resetArriveTime();
    int64_t m_arrive_time; // ms 时间的最晚结束时间
    int64_t m_interval; // ms 时间间隔

    bool m_is_repeated {false}; // 任务是否会反复执行
    bool m_is_canceled {false}; // 取消定时任务

    std::function<void()> m_task;
};
 
}
#endif