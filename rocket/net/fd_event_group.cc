#include "rocket/net/fd_event_group.h"
namespace rocket {
static FdEventGroup* g_fd_event_group = nullptr;

FdEventGroup* FdEventGroup::GetFdEventGroup() {
    if (g_fd_event_group) {
        return g_fd_event_group;
    }
    g_fd_event_group = new FdEventGroup(128);
    return g_fd_event_group;
}

FdEventGroup::FdEventGroup(int size) {
    m_fd_group.resize(size);
    for (int i = 0; i < size; i++) {
        m_fd_group[i] = new FdEvent(i);
    }
}

FdEventGroup::~FdEventGroup() {
    for (auto &fd_event : m_fd_group) {
        if (fd_event) {
            delete fd_event;
            fd_event = nullptr;
        }
    }
}

FdEvent* FdEventGroup::getFdevent(int fd) {
    ScopeMutex<Mutex> lock(m_mutex);
    if (fd < m_fd_group.size()) {
        return m_fd_group[fd];
    }

    // 当前文件描述符不够用了，需要进行扩容，扩容1.5倍
    int new_size = fd * 3 / 2;
    for (int i = m_fd_group.size(); i < new_size; i++) {
        m_fd_group.push_back(new FdEvent(i));
    }
    return m_fd_group[fd];
}
}