#ifndef ROCKET_NET_TCP_ACCEPTOR_H
#define ROCKET_NET_TCP_ACCEPTOR_H
#include <memory>
#include "rocket/net/tcp/net_addr.h"

namespace rocket {

class TcpAcceptor{
public:
    using s_ptr = std::shared_ptr<TcpAcceptor>;

    TcpAcceptor(NetAddr::s_ptr loacl_addr);

    ~TcpAcceptor();

    std::pair<int, NetAddr::s_ptr> accept();

    int getListenFd();
    
private:
    NetAddr::s_ptr m_local_addr; // 服务器监听的地址， addr-> ip:port

    int m_family {-1};

    int m_listenfd {-1};
 
};
}
#endif