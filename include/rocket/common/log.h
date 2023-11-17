#ifndef ROCKET_COMMON_LOG_H
#define ROCKET_COMMON_LOG_H

#include <string>
#include <queue>
#include <memory>
#include <semaphore.h>
#include "rocket/common/mutex.h"
#include "rocket/net/timer_event.h"

namespace rocket {

#define DEBUGLOG(str, ...)                                                                                                          \
do {                                                                                                                                \
if (rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Debug) {                                                            \
    std::string _msg = (rocket::LogEvent(rocket::LogLevel::Debug)).toString()                                                       \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) ;         \
        _msg += "\n";                                                                                                               \
        rocket::Logger::GetGlobalLogger()->pushLog(_msg);                                                                           \
    }                                                                                                                               \
} while(0);                                                                                                                         \


#define INFOLOG(str, ...)                                                                                                           \
do {                                                                                                                                \
    if (rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Info) {                                                         \
        std::string _msg = (rocket::LogEvent(rocket::LogLevel::Info)).toString()                                                    \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) ;         \
        _msg += "\n";                                                                                                               \
        rocket::Logger::GetGlobalLogger()->pushLog(_msg);                                                                           \
    }                                                                                                                               \
} while(0);   


#define ERRORLOG(str, ...)                                                                                                          \
do {                                                                                                                                \
    if (rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Error) {                                                        \
        std::string _msg = (rocket::LogEvent(rocket::LogLevel::Error)).toString()                                                   \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) ;         \
        _msg += "\n";                                                                                                               \
        rocket::Logger::GetGlobalLogger()->pushLog(_msg);                                                                           \
    }                                                                                                                               \
} while(0);   

#define APPDEBUGLOG(str, ...)                                                                                                          \
do {                                                                                                                                \
if (rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Debug) {                                                            \
    std::string _msg = (rocket::LogEvent(rocket::LogLevel::Debug)).toString()                                                       \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) ;         \
        _msg += "\n";                                                                                                               \
        rocket::Logger::GetGlobalLogger()->pushAppLog(_msg);                                                                           \
    }                                                                                                                               \
} while(0);                                                                                                                         \


#define APPINFOLOG(str, ...)                                                                                                           \
do {                                                                                                                                \
    if (rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Info) {                                                         \
        std::string _msg = (rocket::LogEvent(rocket::LogLevel::Info)).toString()                                                    \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) ;         \
        _msg += "\n";                                                                                                               \
        rocket::Logger::GetGlobalLogger()->pushAppLog(_msg);                                                                           \
    }                                                                                                                               \
} while(0);   


#define APPERRORLOG(str, ...)                                                                                                          \
do {                                                                                                                                \
    if (rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Error) {                                                        \
        std::string _msg = (rocket::LogEvent(rocket::LogLevel::Error)).toString()                                                   \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) ;         \
        _msg += "\n";                                                                                                               \
        rocket::Logger::GetGlobalLogger()->pushAppLog(_msg);                                                                           \
    }                                                                                                                               \
} while(0);  
// 将字符串格式化
// 没有学过的知识点，需要深化一下
template<typename... Args>
std::string formatString(const char* str, Args&&... args) {
    int size = snprintf(nullptr, 0, str, args...);

    std::string result;
    if (size > 0) {
        result.resize(size);
        // 这里将数据写入到一个buffer中，这个buffer就是relust存放字符串的指针，大小就是字符串加上空字符的大小
        snprintf(&result[0], size + 1, str, args...);
    }

    return result;
}

enum LogLevel {
    Unknown = 0,
    Debug = 1,
    Info = 2,
    Error = 3,
};

std::string LogLevelToString(LogLevel level);
LogLevel StringToLogLevel(const std::string &log_level);

class LogEvent {
public:
    LogEvent(LogLevel level) : m_level(level) {}
    std::string getFileName() const {
        return m_file_name;
    }

    LogLevel getLogLevel() const {
        return m_level;
    }

    std::string toString();
private:
    std::string m_file_name;
    int32_t m_file_line; // 行号                     
    int32_t m_pid;
    int32_t m_thread_id;

    LogLevel m_level;
};

class AsyncLogger {
public:
    static void* Loop(void* arg);
public:
    using s_ptr = std::shared_ptr<AsyncLogger>;
    AsyncLogger(const std::string& file_name, const std::string& file_path, int max_size);

    void stop();

    void flush();

    void pushLogBuffer(const std::vector<std::string> &vec);
private:
    // TODO 为什么使用队列vector来存？
    std::queue<std::vector<std::string>> m_buffer; 

    std::string m_file_name; // 日志名字
    std::string m_file_path; // 日志路径

    int m_max_file_size {0}; // 日志文件单个文件最大大小

    sem_t m_sempahore;
    pthread_t m_thread;
    pthread_cond_t m_condition;
    Mutex m_mutex;

    std::string m_date; // 当前打印日志的日期
    FILE* m_file_handler {nullptr}; // 打开的日志文件句柄

    bool m_reopen_flag {false};
    bool m_stop_flag {false};
    int m_no {0}; // 日志文件编号
    
};

class Logger {
public:
    enum LogType {
        ASYNC,
        SYNC,
    };
public:
    Logger(LogLevel level, LogType type);
    void pushLog(const std::string &msg);
    void pushAppLog(const std::string &msg);
    void log();
    void syncLoop();
    void init();
    LogLevel getLogLevel();

public:
    static Logger* GetGlobalLogger();
    static void InitGlobalLogger(LogType type = ASYNC);

private:
    LogLevel m_set_level; // 当前的日志级别，只允许打印比当前设置的日志级别低的日志
    std::vector<std::string> m_buffer;
    std::vector<std::string> m_app_buffer;

    Mutex m_mutex;
    Mutex m_app_mutex;
    AsyncLogger::s_ptr m_async_logger;
    AsyncLogger::s_ptr m_app_async_logger;
    TimerEvent::s_ptr m_timer_event;

    LogType m_log_type {ASYNC};
};






} // namespace rocket

#endif