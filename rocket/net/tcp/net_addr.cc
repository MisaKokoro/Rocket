#include <string.h>
#include "rocket/common/log.h"
#include "rocket/net/tcp/net_addr.h"
namespace rocket {

IPNetAddr::IPNetAddr(const std::string &ip, uint16_t port) : m_ip(ip), m_port(port) {
    memset(&m_addr, 0, sizeof(m_addr));

    m_addr.sin_family = AF_INET;
    // 注意inet_addr函数存在不可重入性，以及传入非法的ip地址后无法判断是否出错的问题
    m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
    m_addr.sin_port = htons(m_port);
}

// 传入的是127.0.0.1::8080这种形式的地址
IPNetAddr::IPNetAddr(const std::string &addr) {
    size_t i = addr.find_first_of(":");
    if (i == std::string::npos) {
        ERRORLOG("addr fomat error shoud be x.x.x.x:xx, but %s", addr.c_str());
        return;
    }

    m_ip = addr.substr(0,i);
    m_port = static_cast<uint16_t>(std::stoi(addr.substr(i + 1)));

    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
    m_addr.sin_port = ntohs(m_port);
}

IPNetAddr::IPNetAddr(sockaddr_in addr) : m_addr(addr) {
    m_ip = std::string(inet_ntoa(addr.sin_addr));
    m_port = htons(addr.sin_port);
}

sockaddr* IPNetAddr::getSockAddr() {
    return reinterpret_cast<sockaddr*>(&m_addr);
}

int IPNetAddr::getFamily() {
    return AF_INET;
}

std::string IPNetAddr::toString() {
    std::string res = m_ip + ":" + std::to_string(m_port);
    return res;
}

socklen_t IPNetAddr::getSockLen() {
    return sizeof(m_addr);
}

bool IPNetAddr::checkValid() {
    if (m_ip.empty()) {
        return false;
    }

    if (m_port < 0 || m_port > 65535) {
        return false;
    }

    if (m_addr.sin_addr.s_addr == INADDR_NONE) {
        return false;
    }

    return true;
}
}