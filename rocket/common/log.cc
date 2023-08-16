#include <sys/time.h>
#include <sstream>
#include <stdio.h>
#include "rocket/common/log.h"
#include "rocket/common/util.h"
#include "rocket/common/config.h"

namespace rocket {



static Logger* g_logger = nullptr; // 全局的日志器
void Logger::InitGlobalLogger() {
    if (!g_logger) {
        LogLevel global_log_level = StringToLogLevel(Config::GetGlobalConfig()->m_log_level);
        g_logger = new Logger(global_log_level);
        printf("Init LogLevel [%s]\n",LogLevelToString(global_log_level).c_str());
    }
}
Logger* Logger::GetGlobalLogger() {
    return g_logger;
}


std::string LogLevelToString(LogLevel level) {
    switch (level) {
    case Debug:
        return "DEBUG";
    case Info:
        return "INFO";
    case Error:
        return "ERROR";    
    default:
        return "UNKNOWN";
    }
}

LogLevel StringToLogLevel(const std::string &log_level) {
    if (log_level == "DEBUG") {
        return Debug;
    }
    if (log_level == "INFO") {
        return Info;
    }
    if (log_level == "ERROR") {
        return Error;
    }

    return Unknown;
}

// 将日志转换成这种字符串格式 [Level][%y-%m-%d %H:%M:%s.%ms]\t[pid:thread_id]\t[file_name:line][%msg]
std::string LogEvent::toString() {
    // 获取年月日时分秒毫秒
    struct timeval now_time;
    gettimeofday(&now_time, nullptr);

    struct tm now_time_t;
    localtime_r(&(now_time.tv_sec), &now_time_t); // 线程安全的获取时间的函数localtime_r

    char buf[128];
    strftime(&buf[0], 128, "%y-%m-%d %H:%M:%S", &now_time_t);
    std::string time_str(buf);
    long ms = now_time.tv_usec / 1000;
    time_str += ".";
    time_str += std::to_string(ms);

    m_pid = getPid();
    m_thread_id = getThreadId();

    // 完成进程号线程号以及文件名等的拼接
    std::stringstream ss;
    ss << "[" << LogLevelToString(m_level) << "]"
       << "[" << time_str << "]\t"
       << "[" << m_pid << ":" << m_thread_id << "]\t";
    //    << "[" << std::string(__FILE__) << ":" <<  __LINE__ << "]\t"; // 行号应该放到宏里面添加，否则的话一直输出的是这一行

    return ss.str();
}

void Logger::pushLog(const std::string &msg) {
    ScopeMutex<Mutex> lock(m_mutex);
    m_buffer.push(msg);
}

void Logger::log() {
    // 在这段代码中，tmp 变量是用来保存日志缓冲区的副本的临时队列。它的目的是确保在打印日志消息时，不会持有互斥锁。
    ScopeMutex<Mutex> lock(m_mutex);
    std::queue<std::string> tmp;
    m_buffer.swap(tmp);
    lock.unlock();

    while (!tmp.empty()) {
        std::string msg = tmp.front();
        tmp.pop();
        printf("%s",msg.c_str());
    }
}

LogLevel Logger::getLogLevel() {
    return m_set_level;
}

} // namespace rocket