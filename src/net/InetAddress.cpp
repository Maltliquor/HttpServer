#include <netdb.h>

#include "src/net/InetAddress.h"
#include "src/net/SocketOp.h"

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr
//     {
//      __SOCKADDR_COMMON (sa_);    /* Common data: address family and length.  */
//      char sa_data[14];           /* Address data.  */
//     };

//     struct sockaddr_in {
//         short            sin_family;   // AF_INET,因為這是IPv4;
//         unsigned short   sin_port;     /* Port number.  */
//         struct in_addr   sin_addr;     /* Internet address.  */
//         char             sin_zero[8];  // Not used, must be zero */
//     };
//     struct in_addr {
//         unsigned long s_addr;          // load with inet_pton()
//     };

//     struct sockaddr_in6
//     {
//      __SOCKADDR_COMMON (sin6_);
//      in_port_t sin6_port;        /* Transport layer port # */
//      uint32_t sin6_flowinfo;     /* IPv6 flow information */
//      struct in6_addr sin6_addr;  /* IPv6 address */
//      uint32_t sin6_scope_id;     /* IPv6 scope-id */
//     };

using namespace serverlib;

//static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6),
//     "InetAddress is same size as sock_addrin6");
//static_assert(offsetof(struct sockaddr_in, sin_family) == 0, "sin_family offset 0");
//static_assert(offsetof(struct sockaddr_in6, sin6_family) == 0, "sin6_family offset 0");
//static_assert(offsetof(struct sockaddr_in, sin_port) == 2, "sin_port offset 2");
//static_assert(offsetof(struct sockaddr_in6, sin6_port) == 2, "sin6_port offset 2");

InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6){
	if (ipv6) {
		bzero(&_addr6, sizeof _addr6);
		_addr6.sin6_family = AF_INET6;
		struct in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
		_addr6.sin6_addr = ip;
		_addr6.sin6_port = ::htons(port);
	}
    else {
		bzero(&_addr, sizeof _addr);
		_addr.sin_family = AF_INET;
		in_addr_t ip = loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY;
		_addr.sin_addr.s_addr = ::htonl(ip);
		_addr.sin_port = ::htons(port);
    }
}

InetAddress::InetAddress(std::string ip, uint16_t port, bool ipv6){
	if (ipv6) {
		bzero(&_addr6, sizeof _addr6);
		sockets::fromIpPort(ip.c_str(), port, &_addr6);
	}
	else {
		bzero(&_addr, sizeof _addr);
		sockets::fromIpPort(ip.c_str(), port, &_addr);
	}
}

std::string InetAddress::toIpPort() const
{
	char buf[64] = "";
	sockets::toIpPort(buf, sizeof buf, getSockAddr());
	return buf;
}

std::string InetAddress::toIp() const
{
	char buf[64] = "";
	sockets::toIp(buf, sizeof buf, getSockAddr());
	return buf;
}

uint32_t InetAddress::ipNetEndian() const
{
	assert(family() == AF_INET);
	return _addr.sin_addr.s_addr;
}

uint16_t InetAddress::toPort() const
{
	return ntohl(portNetEndian());
}

static __thread char t_resolveBuffer[64 * 1024];

bool InetAddress::resolve(std::string hostname, InetAddress* out){
	assert(out != NULL);
	struct hostent hent;
	struct hostent* he = NULL;
	int herrno = 0;

	memset(&hent, 0, sizeof(hent));

	int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof t_resolveBuffer, &he, &herrno);
	if (ret == 0 && he != NULL) {
		assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
		out->_addr.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
		return true;
    }
	else {
		if (ret) {
		  perror("InetAddress::resolve");
		}
		return false;
	}
}

void InetAddress::setScopeId(uint32_t scope_id){
    if (family() == AF_INET6) {
		_addr6.sin6_scope_id = scope_id;
    }
}