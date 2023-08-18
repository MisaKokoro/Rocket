#include <pthread.h>
#include <assert.h>
#include "rocket/net/io_thread.h"
#include "rocket/common/log.h"
#include "rocket/common/util.h"

namespace rocket {

IOThread::IOThread() {
    // 初始化两个信号量
    int rt = sem_init(&m_init_semaphore, 0, 0);
    assert(rt == 0);
    rt = sem_init(&m_start_semaphore, 0, 0);
    assert(rt == 0);

    pthread_create(&m_thread, nullptr, &IOThread::Main, this);
    // 线程刚刚创建完成，EventLoop可能还没有完成初始化，需要等待返回
    sem_wait(&m_init_semaphore);

    DEBUGLOG("IOThread [%d] create success", m_thread_id);

}

IOThread::~IOThread() {
    m_event_loop->stop();

    sem_destroy(&m_init_semaphore);
    sem_destroy(&m_start_semaphore);

    pthread_join(m_thread, nullptr);

    if (m_event_loop) {
        delete m_event_loop;
        m_event_loop = nullptr;
    }
}

void* IOThread::Main(void *arg) {
    IOThread *thread = static_cast<IOThread*>(arg);

    thread->m_event_loop = new EventLoop();
    thread->m_thread_id = getThreadId();

    sem_post(&thread->m_init_semaphore);

    // 通过start函数控制IOThread的启动，可以在loop之前完成添加event的动作
    DEBUGLOG("IOThread %d created, wait start semaphore", thread->m_thread_id);
    sem_wait(&thread->m_start_semaphore);
    DEBUGLOG("IOThread %d will start loop", thread->m_thread_id);
    thread->m_event_loop->loop();
}

void IOThread::start() {
    DEBUGLOG("Now invoke thead id[%d] eventloop", m_thread_id);
    sem_post(&m_start_semaphore);
}

void IOThread::join() {
    pthread_join(m_thread, nullptr);
}

EventLoop* IOThread::getEventLoop() {
    return m_event_loop;
}




}