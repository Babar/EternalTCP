#ifndef __ETERNAL_TCP_UNIX_SOCKET_HANDLER__
#define __ETERNAL_TCP_UNIX_SOCKET_HANDLER__

#include "SocketHandler.hpp"

class UnixSocketHandler : public SocketHandler {
public:
  UnixSocketHandler();
  virtual bool hasData(int fd);
  virtual ssize_t read(int fd, void* buf, size_t count);
  virtual ssize_t write(int fd, const void* buf, size_t count);
  virtual int connect(const std::string &hostname, int port);
  virtual int listen(int port) = 0;
  virtual void close(int fd);

protected:
  int serverSocket;
};

#endif // __ETERNAL_TCP_UNIX_SOCKET_HANDLER__