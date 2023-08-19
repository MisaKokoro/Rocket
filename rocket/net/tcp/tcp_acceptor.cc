#include <string.h>
#include "rocket/net/tcp/tcp_acceptor.h"
#include "rocket/common/log.h"
#include "rocket/net/tcp/net_addr.h"

namespace rocket {
TcpAcceptor::TcpAcceptor(NetAddr::s_ptr loacl_addr) : m_local_addr(loacl_addr) {
    if (!loacl_addr->checkValid()) {
        ERRORLOG("invalid addr %s", loacl_addr->toString().c_str());
        exit(1);
    }
    m_family = loacl_addr->getFamily();
    m_listenfd = socket(m_family, SOCK_STREAM, 0);
    if (m_listenfd < 0) {
        ERRORLOG("socket create failed, errno is %d, error:%s", errno, strerror(errno));
        exit(1);
    }

    // 设置socket为REUSEADDR模式，这样在tcp连接关闭后可以重用地址，不需要每次更换端口号了
    int val = 1;
    int rt = setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (rt != 0) {
        ERRORLOG("set socket REUSEADDR failed, errno is %d, error:%s", errno, strerror(errno));
    }

    // TODO 设置listenfd为非阻塞模式

    rt = bind(m_listenfd, loacl_addr->getSockAddr(), loacl_addr->getSockLen());
    if (rt != 0) {
        ERRORLOG("bind addr failed, errno is %d, error:%s", errno, strerror(errno));
        exit(1);
    }

    rt = listen(m_listenfd, 1000);
    if (rt != 0) {
        ERRORLOG("listen fd failed, fd = %d, errno is %d, error:%s",m_listenfd, errno, strerror(errno));
        exit(1);
    }
}

TcpAcceptor::~TcpAcceptor() {

}

int TcpAcceptor::accept() {
    if (m_family == AF_INET) {
        sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        memset(&client_addr, 0, sizeof(client_addr));

        int client_fd = ::accept(m_listenfd, reinterpret_cast<sockaddr*>(&client_addr), &len);
        if (client_fd < 0) {
            ERRORLOG("accept client failed , errno is %d, error:%s", errno, strerror(errno));
        }

        IPNetAddr peer_addr(client_addr);
        INFOLOG("success accept client, ip:%s", peer_addr.toString().c_str());

        return client_fd;

    } else {
        // 其他的一些地址
        return 0;
    }
}
}