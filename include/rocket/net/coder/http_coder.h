#ifndef ROCKET_NET_HTTP_HTTP_CODER_H
#define ROCKET_NET_HTTP_HTTP_CODER_H
#include <map>
#include <string>
#include "rocket/net/coder/abstract_coder.h"
#include "rocket/net/coder/abstract_protocol.h"
#include "rocket/net/tcp/tcp_buffer.h"
#include "rocket/net/http/http_define.h"
#include "rocket/net/http/http_request.h"
#include "rocket/net/http/http_response.h"



namespace rocket {
class HttpCoder : public AbstractCoder {
 public:
  HttpCoder();

  ~HttpCoder();

  void encode(TcpBuffer* buf, AbstractProtocol* data);
  
  void decode(TcpBuffer* buf, AbstractProtocol* data);

  ProtocalType getProtocalType();

 private:
  bool parseHttpRequestLine(HttpRequest* requset, const std::string& tmp);
  bool parseHttpRequestHeader(HttpRequest* requset, const std::string& tmp);
  bool parseHttpRequestContent(HttpRequest* requset, const std::string& tmp);
};
}
#endif
