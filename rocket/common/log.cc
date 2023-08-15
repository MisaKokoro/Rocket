#include <sys/time.h>
#include <sstream>
#include <stdio.h>
#include "rocket/common/log.h"
#include "rocket/common/util.h"

namespace rocket {



static Logger* g_logger = nullptr; // 全局的日志器
Logger* Logger::GetGlobalLogger() {
    if (g_logger) {
        return g_logger;
    }

    g_logger = new Logger();
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
       << "[" << m_pid << ":" << m_thread_id << "]\t"
       << "[" << std::string(__FILE__) << ":" <<  __LINE__ << "]\t";

    return ss.str();
}

void Logger::pushLog(const std::string &msg) {
    m_buffer.push(msg);
}

void Logger::log() {
    while (!m_buffer.empty()) {
        std::string msg = m_buffer.front();
        m_buffer.pop();
        printf(msg.c_str());
    }
}

} // namespace rocket