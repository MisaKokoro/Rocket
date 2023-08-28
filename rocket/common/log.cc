#include <sys/time.h>
#include <sstream>
#include <stdio.h>
#include <assert.h>
#include "rocket/common/log.h"
#include "rocket/common/util.h"
#include "rocket/common/config.h"
#include "rocket/common/run_time.h"
#include "rocket/net/eventloop.h"

namespace rocket {



static Logger* g_logger = nullptr; // 全局的日志器
void Logger::InitGlobalLogger(LogType type /*=ASYNC*/) {
    if (!g_logger) {
        LogLevel global_log_level = StringToLogLevel(Config::GetGlobalConfig()->m_log_level);
        g_logger = new Logger(global_log_level,type);
        g_logger->init();
        printf("Init LogLevel [%s]\n",LogLevelToString(global_log_level).c_str());
    }
}
Logger* Logger::GetGlobalLogger() {
    return g_logger;
}

Logger::Logger(LogLevel level, LogType type) : m_set_level(level), m_log_type(type) {
    if (type == ASYNC) {
        auto config = Config::GetGlobalConfig();
        m_async_logger = std::make_shared<AsyncLogger>(
                config->m_log_file_name + "_rpc", 
                config->m_log_file_path,
                config->m_log_max_file_size);
        m_app_async_logger = std::make_shared<AsyncLogger>(
                config->m_log_file_name + "_app", 
                config->m_log_file_path,
                config->m_log_max_file_size);
    }
}

// 如果在构造函数进行初始化，addTimeEvent会触发打印日志操作，但此时logger还没有初始化完成
// 并不能打印，因此需要二段初始化
void Logger::init() {
    if (m_log_type == ASYNC) {
        auto config = Config::GetGlobalConfig();
        m_timer_event = std::make_shared<TimerEvent>(config->m_log_sync_inteval, true, std::bind(&Logger::syncLoop, this));
        EventLoop::GetCurrentEventLoop()->addTimerEvent(m_timer_event);
    }
}

// 同步m_buffer 到 async_logger的 buffer队尾
void Logger::syncLoop() {
    // printf("sync to synclogger\n");
    std::vector<std::string> tmp;
    std::vector<std::string> appTmp;

    ScopeMutex<Mutex> lock(m_mutex);
    m_buffer.swap(tmp);
    lock.unlock();

    ScopeMutex<Mutex> applock(m_app_mutex);
    m_app_buffer.swap(appTmp);
    applock.unlock();


    if (!tmp.empty()) {
        m_async_logger->pushLogBuffer(tmp);
    }
    if (!appTmp.empty()) {
        m_app_async_logger->pushLogBuffer(appTmp);
    }

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

    // 输出msg_id相关信息
    std::string msg_id = RunTime::GetRunTime()->m_msg_id;
    std::string method_name = RunTime::GetRunTime()->m_method_name;
    if (!msg_id.empty()) {
        ss << "[" << msg_id << "]\t";
    }
    if (!method_name.empty()) {
        ss << "[" << method_name << "]\t";
    }

    return ss.str();
}

void Logger::pushLog(const std::string &msg) {
    if (m_log_type == ASYNC) {
        ScopeMutex<Mutex> lock(m_mutex);
        m_buffer.push_back(msg);
    } else {
        printf("%s", msg.c_str());
    }
}

void Logger::pushAppLog(const std::string &msg) {
    if (m_log_type == ASYNC) {
        ScopeMutex<Mutex> lock(m_app_mutex);
        m_app_buffer.push_back(msg);
    } else {
        printf("%s\n", msg.c_str());
    }
}

void Logger::log() {
    // 在这段代码中，tmp 变量是用来保存日志缓冲区的副本的临时队列。它的目的是确保在打印日志消息时，不会持有互斥锁。
    // ScopeMutex<Mutex> lock(m_mutex);
    // std::vector<std::string> tmp;
    // m_buffer.swap(tmp);
    // lock.unlock();

    // while (!tmp.empty()) {
    //     std::string msg = tmp.front();
    //     tmp.pop();
    //     printf("%s",msg.c_str());
    // }
}

LogLevel Logger::getLogLevel() {
    return m_set_level;
}

AsyncLogger::AsyncLogger(const std::string& file_name, const std::string& file_path, int max_size) : 
    m_file_name(file_name), m_file_path(file_path), m_max_file_size(max_size) {
    int rt = sem_init(&m_sempahore,0,0);
    assert(rt == 0);
    pthread_create(&m_thread, nullptr, &AsyncLogger::Loop, this);
    pthread_cond_init(&m_condition, nullptr);
    sem_wait(&m_sempahore);
    printf("AsyncLogger init\n");

}

// 异步日志器会一直处在一个loop中打印日志
void* AsyncLogger::Loop(void* arg) {
    AsyncLogger* logger = reinterpret_cast<AsyncLogger*>(arg);
    // TODO 如果防止构造函数里初始化会有一点小问题
    // pthread_cond_init(&logger->m_condition, nullptr);

    sem_post(&logger->m_sempahore);

    while (true) {
        if (logger->m_stop_flag) {
            break;
        }

        // 从日志队列里面取出一批日志
        ScopeMutex<Mutex> lock(logger->m_mutex);
        while (logger->m_buffer.empty()) {
            pthread_cond_wait(&logger->m_condition, logger->m_mutex.getMutex());
        }
        printf("wake up sem\n");
        std::vector<std::string> tmp;
        tmp.swap(logger->m_buffer.front());
        logger->m_buffer.pop();
        lock.unlock();
        
        // 获取当前的日期与之前的日期比较是否应该日志滚动以及日志文件的命名
        std::string date = getNowDate();
        if (date != logger->m_date) {
            logger->m_no = 0;
            logger->m_reopen_flag = true;
            logger->m_date = date;
        }
        if (logger->m_file_handler == nullptr) {
            logger->m_reopen_flag = true;
        } 

        // 生成日志文件全名
        // printf("date = %s\n, date.size() = %d", date.c_str(), date.size());
        std::stringstream ss;
        ss << logger->m_file_path << logger->m_file_name << "."
           << date << "_log."; 
        // printf("ss.str() = %s\n",ss.str().c_str());
        std::string log_file_name = ss.str() + std::to_string(logger->m_no++);
        // printf("log file name = %s\n", log_file_name.c_str());
        // 将文件打开
        if (logger->m_reopen_flag) {
            if (logger->m_file_handler) {
                fclose(logger->m_file_handler);
            }
            logger->m_file_handler = fopen(log_file_name.c_str(), "a");
            logger->m_reopen_flag = false;
        }
        
        // 日志文件已经够大了，重新创建一个日志文件
        if (ftell(logger->m_file_handler) > logger->m_max_file_size) {
            fclose(logger->m_file_handler);

            log_file_name = ss.str() + std::to_string(logger->m_no++);
            logger->m_file_handler = fopen(log_file_name.c_str(), "a");
            logger->m_reopen_flag = false;
        }

        for (auto &data : tmp) {
            // printf("%s", data.c_str());
            fwrite(data.c_str(), 1, data.size(), logger->m_file_handler);
        }
        fflush(logger->m_file_handler);

    }

    return nullptr;
}

void AsyncLogger::stop() {
    m_stop_flag = true;
}

void AsyncLogger::flush() {
    if (m_file_handler) {
        fflush(m_file_handler);
    }
}

void AsyncLogger::pushLogBuffer(const std::vector<std::string> &vec) {
    ScopeMutex<Mutex> lock(m_mutex);
    m_buffer.push(vec);
    lock.unlock();

    pthread_cond_broadcast(&m_condition);
}

} // namespace rocket