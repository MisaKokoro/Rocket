#ifndef ROCKET_NET_CODER_TINYPB_PROTOCOL_H
#define ROCKET_NET_CODER_TINYPB_PROTOCOL_H

#include <string>
#include "rocket/net/coder/abstract_protocol.h"
namespace rocket {
struct TinyPBProtocol : public AbstractProtocol {
public:
    static char PB_START;
    static char PB_END;
public:
    TinyPBProtocol() {}
    ~TinyPBProtocol() {}

public:
    int32_t m_pk_len {0}; // 包的长度
    int32_t m_req_id_len {0};
    // req_id继承父类

    int32_t m_method_name_len {0};
    std::string m_method_name;
    int32_t m_err_code {0};
    int32_t m_err_info_len {0}; // 错误信息长度
    
    std::string m_err_info;
    std::string m_pb_data; // protobuffer字节流数据

    int32_t m_check_sum {0}; // 校验和

    bool parse_success {false};
};





} // namespace rocket
#endif