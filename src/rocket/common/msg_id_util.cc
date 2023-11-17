#include <atomic>
#include "rocket/common/msg_id_util.h"
namespace rocket {

static std::atomic<int> g_msg_id(1000000000);


// static thread_local std::string t_msg_id_no;
// static thread_local std::string t_max_msg_id_no;

std::string MsgIDUtil::GenMsgID() {
    g_msg_id++;
    return std::to_string(g_msg_id);
}

}