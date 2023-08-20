# Rocket

### 2023.8.19:17:32
新增了tcp的一些模块
对unix下的网络地址进行了封装
实现了tcp的一个buffer
封装了tcp的accept操作
 
 ### 2023.8.20:00:15
 实现了tcpserver的功能
 tcpserver就是整个rpc服务的主体，是主reactor

 ### 2023.8.21:00:02
 1.修复了一个日志打印的bug，该bug会导致当命名字符串为msg时，打印错误日志，因此将打印日志宏里面的msg改名为了_msg
 确保不会重名
2.修复了一个delete epoll的bug，该bug会导致无法删除epoll，因为判断的条件写错了
3.增加了TcpConnection类，该类实现了对端连接之后的一系列操作的封装

但是还是有点小问题，不知道为什么，test_client程序总是要等一会儿才能收到消息