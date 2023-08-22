#include <syscall.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <string.h>
#include "rocket/common/util.h"
namespace rocket {

// 将获取到的线程号和进程号缓存下来，避免频繁调用系统调用
static int g_pid = 0;
static thread_local int g_thread_id = 0; 

pid_t getPid() {
    if (g_pid != 0) {
        return g_pid;
    }
    return getpid();
}

pid_t getThreadId() {
    if (g_thread_id != 0) {
        return g_thread_id;
    }
    return syscall(SYS_gettid); // 用pthread_self()代替？
}

int64_t getNowMs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

uint32_t getInt32FromNetByte(const char* buf) {
    int32_t res = 0;
    memcpy(&res, buf, sizeof(res));
    res = htonl(res);
    return res;
}


} // namespace rocket
