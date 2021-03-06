#include <errno.h>
#include <fcntl.h>
#include <stdio.h>  // snprintf
#include <sys/socket.h>
#include <sys/uio.h>  // readv
#include <unistd.h>

#include "src/net/SocketOp.h"


using namespace serverlib;


namespace
{

typedef struct sockaddr SA;


#if VALGRIND || defined (NO_ACCEPT4)
void setNonBlockAndCloseOnExec(int sockfd)
{
  // non-block
  int flags = ::fcntl(sockfd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  int ret = ::fcntl(sockfd, F_SETFL, flags);
  // FIXME check

  // close-on-exec
  flags = ::fcntl(sockfd, F_GETFD, 0);
  flags |= FD_CLOEXEC;
  ret = ::fcntl(sockfd, F_SETFD, flags);
  // FIXME check

  (void)ret;
}
#endif

}  // namespace

const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in6* addr){
  return static_cast<const struct sockaddr*>(static_cast<const void*>(addr));
}

struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in6* addr){
  return static_cast<struct sockaddr*>(static_cast<void*>(addr));
}

const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in* addr){
  return static_cast<const struct sockaddr*>(static_cast<const void*>(addr));
}

struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in* addr){
  return static_cast<struct sockaddr*>(static_cast<void*>(addr));
}

const struct sockaddr_in* sockets::sockaddr_in_cast(const struct sockaddr* addr){
  return static_cast<const struct sockaddr_in*>(static_cast<const void*>(addr));
}

const struct sockaddr_in6* sockets::sockaddr_in6_cast(const struct sockaddr* addr){
  return static_cast<const struct sockaddr_in6*>(static_cast<const void*>(addr));
}

int sockets::createNonblockingOrDie(sa_family_t family){
	//设置为非阻塞socket
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
		perror("Error in sockets::createNonblockingOrDie");
    }
    return sockfd;
}

void sockets::bindOrDie(int sockfd, const struct sockaddr* addr){
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if (ret < 0){
		perror("Error in sockets::bindOrDie");
		::close(sockfd);
		exit(1);
    }
}

void sockets::listenOrDie(int sockfd){
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0){
		perror("Error in sockets::listenOrDie");
		::close(sockfd);
		exit(1);
    }
}

int sockets::accept(int sockfd, struct sockaddr_in6* addr){
  socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
#if VALGRIND || defined (NO_ACCEPT4)
  int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
  setNonBlockAndCloseOnExec(connfd);
#else
  int connfd = ::accept4(sockfd, sockaddr_cast(addr),
                         &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
  if (connfd < 0) {
    int savedErrno = errno;
    perror("Error in Socket::accept");
    switch (savedErrno) {
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO: // ???
      case EPERM:
      case EMFILE: // per-process lmit of open file desctiptor ???
        // expected errors
        errno = savedErrno;
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        // unexpected errors
        perror( "unexpected error of ::accept " );
        break;
      default:
        perror( "unknown error of ::accept " );
        break;
    }
  }
  return connfd;
}

int sockets::connect(int sockfd, const struct sockaddr* addr){
  return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

ssize_t sockets::read(int sockfd, void *buf, size_t count){
  return ::read(sockfd, buf, count);
}

ssize_t sockets::readv(int sockfd, const struct iovec *iov, int iovcnt){
    return ::readv(sockfd, iov, iovcnt);
}
ssize_t sockets::readFd(int fd, std::vector<char> &buf, int *savedErrno){
	const size_t init_size = 1024;
	
	// saved an ioctl()/FIONREAD call to tell how much to read
	char extrabuf[65536];
	buf.clear();
	buf.resize(init_size);	
	
	struct iovec vec[2];
	
	vec[0].iov_base =  &*(buf.begin()); //保存vector的首地址
	vec[0].iov_len = init_size;
	vec[1].iov_base = extrabuf;
	vec[1].iov_len = sizeof extrabuf;
	// when there is enough space in this buffer, don't read into extrabuf.
	// when extrabuf is used, we read 128k-1 bytes at most.
	const int iovcnt = (init_size < sizeof extrabuf) ? 2 : 1;
	
	const ssize_t n = sockets::readv(fd, vec, iovcnt);
	if (n < 0) {
		*savedErrno = errno;
	}
	else if (static_cast<size_t>(n) <= init_size) {
		//将input_buf中的数据保存到buf指针中
		//std::string str(input_buf.begin(), input_buf.end());
		if(static_cast<size_t>(n) == init_size)
			buf.push_back('\0');
		else{
			buf[n] ='\0';
			buf.resize(n+1);
		}
		
	}
	else  //n > init_size, 有数据保存到extrabuf中
	{
		std::string str(buf.begin(), buf.end());
		std::string str2(extrabuf, extrabuf+(n-init_size));
		str += str2;
		buf.resize(str.size());
		buf.assign(str.begin(),str.end());
		buf.push_back('\0');
		
	}
	
	//printf("ReadFd() Read from client fd=%d:\n%s\n", fd, buf);
	
	return n;
}
ssize_t sockets::write(int sockfd, const void *buf, size_t count){
    return ::write(sockfd, buf, count);
}

void sockets::close(int sockfd){
  if (::close(sockfd) < 0) {
    perror("Error in sockets::close");
  }
}

void sockets::shutdownWrite(int sockfd){
  if (::shutdown(sockfd, SHUT_WR) < 0) {
    perror("Error in sockets::shutdownWrite");
  }
}

void sockets::toIpPort(char* buf, size_t size,
                       const struct sockaddr* addr)
{
  toIp(buf,size, addr);
  size_t end = ::strlen(buf);
  const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
  uint16_t port = ntohs(addr4->sin_port); //sockets::networkToHost16(addr4->sin_port);
  assert(size > end);
  snprintf(buf+end, size-end, ":%u", port);
}

void sockets::toIp(char* buf, size_t size,
                   const struct sockaddr* addr)
{
    if (addr->sa_family == AF_INET) {
		assert(size >= INET_ADDRSTRLEN);
		const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
		::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
    }
    else if (addr->sa_family == AF_INET6) {
		assert(size >= INET6_ADDRSTRLEN);
		const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
		::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
    }
}

void sockets::fromIpPort(const char* ip, uint16_t port,
                         struct sockaddr_in* addr)
{
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);//hostToNetwork16(port);
	if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)	{
		perror("Error in sockets::fromIpPort");
	}
}

void sockets::fromIpPort(const char* ip, uint16_t port,
                         struct sockaddr_in6* addr)
{
	addr->sin6_family = AF_INET6;
	addr->sin6_port = htons(port);//hostToNetwork16(port);
	if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0){
		perror("Error in sockets::fromIpPort");
	}
}

int sockets::getSocketError(int sockfd){
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);

    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)  {
        return errno;
    }
    else {
        return optval;
    }
}

struct sockaddr_in6 sockets::getLocalAddr(int sockfd)
{
  struct sockaddr_in6 localaddr;
  bzero(&localaddr, sizeof localaddr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
  if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0)
  {
    perror("Error in sockets::getLocalAddr");
  }
  return localaddr;
}

struct sockaddr_in6 sockets::getPeerAddr(int sockfd)
{
  struct sockaddr_in6 peeraddr;
  bzero(&peeraddr, sizeof peeraddr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
  if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
  {
    perror("Error in sockets::getPeerAddr");
  }
  return peeraddr;
}

bool sockets::isSelfConnect(int sockfd)
{
  struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
  struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
  if (localaddr.sin6_family == AF_INET)
  {
    const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr);
    const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
    return laddr4->sin_port == raddr4->sin_port
        && laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
  }
  else if (localaddr.sin6_family == AF_INET6)
  {
    return localaddr.sin6_port == peeraddr.sin6_port
        && memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof localaddr.sin6_addr) == 0;
  }
  else
  {
    return false;
  }
}