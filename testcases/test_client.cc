#include <unistd.h>
#include <string.h>
#include "rocket/net/tcp/net_addr.h"
#include "rocket/common/log.h"
#include "rocket/common/config.h"
#include "rocket/net/tcp/tcp_acceptor.h"
#include "rocket/net/tcp/tcp_server.h"
#include "rocket/net/fd_event_group.h"
#include "rocket/net/tcp/tcp_connection.h"

void test_connect() {
  // 调用 conenct 连接 server
  // wirte 一个字符串
  // 等待 read 返回结果

  int fd = socket(AF_INET, SOCK_STREAM, 0);

  if (fd < 0) {
    ERRORLOG("invalid fd %d", fd);
    exit(0);
  }

  sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(12345);
  inet_aton("127.0.0.1", &server_addr.sin_addr);

  int rt = connect(fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));

  DEBUGLOG("connect success");

  std::string msg = "hello rocket!cxcx";
  
  rt = write(fd, msg.c_str(), msg.length());

  DEBUGLOG("success write %d bytes, [%s]", rt, msg.c_str());

  char buf[100] = {0};
  rt = read(fd, buf, 100);
  DEBUGLOG("success read %d bytes, [%s]", rt, std::string(buf).c_str());

}
int main() {
    rocket::Config::InitGlobalConfig("/home/yanxiang/Desktop/MyProject/rocket/conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();

    rocket::IPNetAddr addr("127.0.0.1:12348");
    std::string msg = "hello shfghdsd!";
    DEBUGLOG("msg = %s", msg.c_str());
    DEBUGLOG("create addr %s", addr.toString().c_str());


    test_connect();



    return 0;
}
