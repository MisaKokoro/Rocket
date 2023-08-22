#ifndef ROCKET_NET_CODER_TINYPB_CODER_H
#define ROCKET_NET_CODER_TINYPB_CODER_H

#include "rocket/net/coder/abstract_coder.h"
#include "rocket/net/coder/abstract_protocol.h"
#include "rocket/net/coder/tinypb_protocol.h"
namespace rocket {

class TinyPBCoder : public AbstractCoder {
public:
    // 将message对象转化为字节流写入到buffer
    void encode(std::vector<AbstractProtocol::s_ptr> &messages, TcpBuffer::s_ptr out_buffer);

    // 将buffer的字节流转化为message
    void decode(std::vector<AbstractProtocol::s_ptr> &out_messages, TcpBuffer::s_ptr out_buffer);

    const char* encodeTinyPB(std::shared_ptr<TinyPBProtocol> message, int& len);

    ~TinyPBCoder() {

    }
};
}
#endif