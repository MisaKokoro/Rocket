#ifndef ROCKET_COMMON_LOG_H
#define ROCKET_COMMON_LOG_H

#include <string>
#include <queue>
#include "rocket/common/mutex.h"

namespace rocket {

#define DEBUGLOG(str, ...)                                                                                                      \
do {                                                                                                                             \
    if (rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Debug) {                                                     \                                                                                                                                                          
        std::string msg = (rocket::LogEvent(rocket::LogLevel::Debug)).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) ;    \
        msg += "\n";                                                                                                                \
        rocket::Logger::GetGlobalLogger()->pushLog(msg);                                                                            \
        rocket::Logger::GetGlobalLogger()->log();                                                               \
    }                                                                                                                               \    
} while(0);                                                                                                                     \


#define INFOLOG(str, ...)                                                                                                      \
do {                                                                                                                             \
    if (rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Info) {                                                     \                                                                                                                                                          
        std::string msg = (rocket::LogEvent(rocket::LogLevel::Info)).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) ;    \
        msg += "\n";                                                                                                                \
        rocket::Logger::GetGlobalLogger()->pushLog(msg);                                                                            \
        rocket::Logger::GetGlobalLogger()->log();                                                               \
    }                                                                                                                               \    
} while(0);   


#define ERRORLOG(str, ...)                                                                                                      \
do {                                                                                                                             \
    if (rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Error) {                                                     \                                                                                                                                                          
        std::string msg = (rocket::LogEvent(rocket::LogLevel::Error)).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) ;    \
        msg += "\n";                                                                                                                \
        rocket::Logger::GetGlobalLogger()->pushLog(msg);                                                                            \
        rocket::Logger::GetGlobalLogger()->log();                                                               \
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

class Logger {
public:
    Logger(LogLevel level) : m_set_level(level) {}
    void pushLog(const std::string &msg);
    void log();
    LogLevel getLogLevel();

public:
    static Logger* GetGlobalLogger();
    static void InitGlobalLogger();

private:
    LogLevel m_set_level; // 当前的日志级别，只允许打印比当前设置的日志级别低的日志
    std::queue<std::string> m_buffer;
    Mutex m_mutex;
    

};

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


} // namespace rocket

#endif