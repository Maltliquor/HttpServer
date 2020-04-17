#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>  // snprintf

#include "src/net/InetAddress.h"
#include "src/net/Socket.h"
#include "src/net/SocketOp.h"



using namespace serverlib;

Socket::~Socket()
{
  sockets::close(_sock_fd);
}

bool Socket::getTcpInfo(struct tcp_info* tcpi) const
{
  socklen_t len = sizeof(*tcpi);
  bzero(tcpi, len);
  return ::getsockopt(_sock_fd, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
}


void Socket::bindAddress(const InetAddress& addr)
{
  sockets::bindOrDie(_sock_fd, addr.getSockAddr());
}

void Socket::listen()
{
  sockets::listenOrDie(_sock_fd);
}

int Socket::accept(InetAddress* peeraddr)
{
  struct sockaddr_in6 addr;
  bzero(&addr, sizeof addr);
  int connfd = sockets::accept(_sock_fd, &addr);
  if (connfd >= 0) {
    peeraddr->setSockAddrInet6(addr);
  }
  return connfd;
}

void Socket::shutdownWrite()
{
  sockets::shutdownWrite(_sock_fd);
}

void Socket::setTcpNoDelay(bool on)
{
  int optval = on ? 1 : 0;
  ::setsockopt(_sock_fd, IPPROTO_TCP, TCP_NODELAY,
               &optval, static_cast<socklen_t>(sizeof optval));
  // FIXME CHECK
}

void Socket::setReuseAddr(bool on)
{
  int optval = on ? 1 : 0;
  ::setsockopt(_sock_fd, SOL_SOCKET, SO_REUSEADDR,
               &optval, static_cast<socklen_t>(sizeof optval));
  // FIXME CHECK
}

void Socket::setReusePort(bool on)
{
#ifdef SO_REUSEPORT
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(_sock_fd, SOL_SOCKET, SO_REUSEPORT,
                         &optval, static_cast<socklen_t>(sizeof optval));
  if (ret < 0 && on)
  {
    perror("SO_REUSEPORT failed.");
  }
#else
  if (on)
  {
    perror("SO_REUSEPORT is not supported.");
  }
#endif
}

void Socket::setKeepAlive(bool on)
{
  int optval = on ? 1 : 0;
  ::setsockopt(_sock_fd, SOL_SOCKET, SO_KEEPALIVE,
               &optval, static_cast<socklen_t>(sizeof optval));
  // FIXME CHECK
}
