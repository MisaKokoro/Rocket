#include "rocket/common/log.h"
#include <thread>
#include <vector>
using namespace std;

void func() {
     DEBUGLOG("test log %s", "11");
}
int main() {
    DEBUGLOG("test log %s", "main");
    vector<thread> th;
    for (int i = 0; i < 10; i++) {
        th.push_back(thread(func));
    }

    for (int i = 0; i < 10; i++) {
        th[i].join();
    }
    return 0;
}