#ifndef ROCKET_NET_RPC_RPC_CLOSURE_H
#define ROCKET_NET_RPC_RPC_CLOSURE_H

#include <memory>
#include <google/protobuf/stubs/callback.h>

namespace rocket {
class RpcClosure : public google::protobuf::Closure {
public:
    RpcClosure(std::function<void()> cb) : m_cb(cb) {}

    void Run() override {
        if (m_cb) {
            m_cb();
        }
    }

    ~RpcClosure() {}

private:
    std::function<void()> m_cb {nullptr};
};
}
#endif