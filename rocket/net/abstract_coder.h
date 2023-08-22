#ifndef ROCKET_NET_ABSTRACT_CODER_H
#define ROCKET_NET_ABSTRACT_CODER_H

#include <vector>
#include <memory>
#include "rocket/net/tcp/tcp_buffer.h"
#include "rocket/net/abstract_protocol.h"

namespace rocket {

class AbstractCoder {
public:
    using s_ptr = std::shared_ptr<AbstractCoder>;
public:
    // 将message对象转化为字节流写入到buffer
    virtual void encode(std::vector<AbstractProtocol::s_ptr> &messages, TcpBuffer::s_ptr out_buffer) = 0;

    // 将buffer的字节流转化为message
    virtual void decode(std::vector<AbstractProtocol::s_ptr> &out_messages, TcpBuffer::s_ptr out_buffer) = 0;
     
    virtual ~AbstractCoder() {}
};
}
#endif