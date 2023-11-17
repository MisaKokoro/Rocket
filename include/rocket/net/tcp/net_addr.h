#ifndef ROCKET_NET_TCP_NET_ADDR_H
#define ROCKET_NET_TCP_NET_ADDR_H
#include <arpa/inet.h>
#include <string>
#include <memory>
namespace rocket {

// 基本网络地址，一个抽象类
class NetAddr {
public:
    using s_ptr = std::shared_ptr<NetAddr>;
    virtual sockaddr* getSockAddr() = 0;

    virtual socklen_t getSockLen() = 0;

    virtual int getFamily() = 0;

    virtual std::string toString() = 0;

    virtual bool checkValid() = 0;

};

// 封装ipv4 ip地址方便使用
class IPNetAddr : public NetAddr {
public:
    IPNetAddr(const std::string &ip, uint16_t port);

    IPNetAddr(const std::string &addr);

    IPNetAddr(sockaddr_in addr);

    sockaddr* getSockAddr();

    int getFamily();

    std::string toString();

    socklen_t getSockLen();

    bool checkValid();
private:
    std::string m_ip; // ip地址 点十分形式
    uint16_t m_port {0}; // 端口号

    sockaddr_in m_addr; // ip地址在Linux内部表达形式
};

}
#endif