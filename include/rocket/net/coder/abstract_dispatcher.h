#pragma once

#include <memory>
#include <google/protobuf/service.h>
#include "rocket/net/abstract_data.h"
#include "rocket/net/tcp/tcp_connection.h"

namespace rocket {

class TcpConnection;

class AbstractDispatcher {
 public:
  typedef std::shared_ptr<AbstractDispatcher> ptr;

  AbstractDispatcher() {}

  virtual ~AbstractDispatcher() {}

  virtual void dispatch(AbstractData* data, TcpConnection* conn) = 0;

};

}


#endif
