#ifndef ROCKET_NET_FD_EVENT_GROUP_H
#define ROCKET_NET_FD_EVENT_GROUP_H

#include <vector>
#include "rocket/net/fd_event.h"
#include "rocket/common/mutex.h"
namespace rocket {
// 由于所有的文件描述符的递增的，因此用一个全局的fdeventGroup进行统一管理
class FdEventGroup {
public:
    FdEventGroup(int size);

    ~FdEventGroup();

    FdEvent* getFdevent(int fd);
public:
    static FdEventGroup* GetFdEventGroup();
private:
    int m_size {0};
    std::vector<FdEvent*> m_fd_group;
    Mutex m_mutex;
};
}
#endif