#ifndef ROCKET_NET_IO_THREAD_GROUP_H
#define ROCKET_NET_IO_THREAD_GROUP_H
#include <vector>
#include "rocket/net/io_thread.h"
//IO 线程池实现(但好像并不是池化机制)


namespace rocket {
class IOThreadGroup {
public:
    IOThreadGroup(int size);

    ~IOThreadGroup();

    void start();

    void join();

    IOThread* getIOThread();

private:
    int m_size {0};
    std::vector<IOThread*> m_io_thread_groups;
    int m_index {-1};
};
}

#endif