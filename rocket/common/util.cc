#include "rocket/common/util.h"
#include <syscall.h>
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
    return syscall(SYS_gettid);
}

} // namespace rocket
