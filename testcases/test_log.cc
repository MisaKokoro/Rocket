#include <thread>
#include <vector>
#include "rocket/common/log.h"
#include "rocket/common/config.h"
using namespace std;

void func() {
    DEBUGLOG("test log %s", "thread");
    INFOLOG("test log %s", "thread");
}
int main() {
    
    vector<thread> th;
    for (int i = 0; i < 10; i++) {
        th.push_back(thread(func));
    }

    for (int i = 0; i < 10; i++) {
        th[i].join();
    }
    DEBUGLOG("test log %s", "main");
    INFOLOG("test log %s", "main");
    return 0;
}