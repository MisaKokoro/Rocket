#include "rocket/net/io_thread_group.h"
#include "rocket/common/log.h"

namespace rocket {
IOThreadGroup::IOThreadGroup(int size) {
    m_io_thread_groups.resize(size);
    for (int i = 0; i < size; i++) {
        m_io_thread_groups[i] = new IOThread();
    }

    DEBUGLOG("IOThreadGroup create success, size : %d", size);
    
}

IOThreadGroup::~IOThreadGroup() {

}

void IOThreadGroup::start() {
    for (auto &io_thread : m_io_thread_groups) {
        io_thread->start();
    }
}

// TODO 为什么要轮询设计？ 轮询的获取IOThread，只是获取
IOThread* IOThreadGroup::getIOThread() {
    if (m_index == m_io_thread_groups.size() || m_index == -1) {
        m_index = 0;
    }

    return m_io_thread_groups[m_index++];
}

void IOThreadGroup::join() {
    for (auto &io_thread : m_io_thread_groups) {
        io_thread->join();
    }
}

}