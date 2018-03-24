#ifndef KONTROLLER_SOCK_SOCK_H
#define KONTROLLER_SOCK_SOCK_H

#if defined(_WIN32)
#  define SOCK_WINDOWS 1
#  define SOCK_POSIX 0
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#  define SOCK_WINDOWS 0
#  define SOCK_POSIX 1
#else
#  error "Platform not supported"
#endif

#include <cstdint>

#if SOCK_WINDOWS
#  include <WinSock2.h>
#  include <WS2tcpip.h>
#  pragma comment(lib, "Ws2_32.lib") // Need to link with Ws2_32.lib
#  define SHUT_RD SD_RECEIVE
#  define SHUT_WR SD_SEND
#  define SHUT_RDWR SD_BOTH
using ssize_t = SSIZE_T;
#elif SOCK_POSIX
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <netinet/tcp.h>
#  include <sys/errno.h>
#  include <sys/ioctl.h>
#  include <sys/socket.h>
#  include <sys/types.h>
#  include <unistd.h>
#endif

namespace Sock {

constexpr int kSocketError = -1;

#if SOCK_WINDOWS
using Socket = SOCKET;
constexpr Socket kInvalidSocket = INVALID_SOCKET;
constexpr int kWouldBlock = WSAEWOULDBLOCK;
#elif SOCK_POSIX
using Socket = int;
constexpr Socket kInvalidSocket = -1;
constexpr int kWouldBlock = EWOULDBLOCK;
#endif

namespace System {

inline int getLastError() {
#if SOCK_WINDOWS
   return WSAGetLastError();
#elif SOCK_POSIX
   return errno;
#endif
}

inline int initialize() {
#if SOCK_WINDOWS
   WSADATA wsaData;
   return WSAStartup(MAKEWORD(2, 2), &wsaData);
#elif SOCK_POSIX
   return 0;
#endif
}

inline bool terminate() {
#if SOCK_WINDOWS
   return WSACleanup() == 0;
#elif SOCK_POSIX
   return true;
#endif
}

} // namespace System

namespace Endian {

inline uint32_t hostToNetworkLong(uint32_t hostLong) {
   return htonl(hostLong);
}

inline uint16_t hostToNetworkShort(uint16_t hostShort) {
   return htons(hostShort);
}

inline uint32_t networkToHostLong(uint32_t netLong) {
   return ntohl(netLong);
}

inline uint16_t networkToHostShort(uint16_t netShort) {
   return ntohs(netShort);
}

} // namespace Endian

inline Socket accept(Socket socket, sockaddr* addr, socklen_t* addrlen) {
   return ::accept(socket, addr, addrlen);
}

inline int bind(Socket socket, const sockaddr* addr, socklen_t addrlen) {
   return ::bind(socket, addr, addrlen);
}

inline int close(Socket socket) {
#if SOCK_WINDOWS
   return ::closesocket(socket);
#elif SOCK_POSIX
   return ::close(socket);
#endif
}

inline int connect(Socket socket, const sockaddr* addr, socklen_t addrlen) {
   return ::connect(socket, addr, addrlen);
}

inline void freeaddrinfo(addrinfo* res) {
   return ::freeaddrinfo(res);
}

inline int getaddrinfo(const char* node, const char* service, const addrinfo* hints, addrinfo** result) {
   return ::getaddrinfo(node, service, hints, result);
}

inline int getpeername(Socket socket, sockaddr* address, socklen_t* addressLen) {
   return ::getpeername(socket, address, addressLen);
}

inline int getsockname(Socket socket, sockaddr* address, socklen_t* addressLen) {
   return ::getsockname(socket, address, addressLen);
}

inline int getsockopt(Socket socket, int level, int optionName, void* optionValue, socklen_t* optionLen) {
#if SOCK_WINDOWS
   return ::getsockopt(socket, level, optionName, static_cast<char*>(optionValue), optionLen);
#elif SOCK_POSIX
   return ::getsockopt(socket, level, optionName, optionValue, optionLen);
#endif
}

inline int ioctl(Socket socket, unsigned long command, unsigned long* arg) {
#if SOCK_WINDOWS
   return ::ioctlsocket(socket, static_cast<long>(command), arg);
#elif SOCK_POSIX
   return ::ioctl(socket, command, arg);
#endif
}

inline int listen(Socket socket, int backlog) {
   return ::listen(socket, backlog);
}

inline ssize_t recv(Socket socket, void* buf, size_t len, int flags) {
#if SOCK_WINDOWS
   return ::recv(socket, static_cast<char*>(buf), static_cast<int>(len), flags);
#elif SOCK_POSIX
   return ::recv(socket, buf, len, flags);
#endif
}

inline ssize_t recvfrom(Socket socket, void* buf, size_t len, int flags, sockaddr* srcAddr, socklen_t* addrlen) {
#if SOCK_WINDOWS
   return ::recvfrom(socket, static_cast<char*>(buf), static_cast<int>(len), flags, srcAddr, addrlen);
#elif SOCK_POSIX
   return ::recvfrom(socket, buf, len, flags, srcAddr, addrlen);
#endif
}

inline int select(Socket nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, timeval* timeout) {
#if SOCK_WINDOWS
   return ::select(0, readfds, writefds, exceptfds, timeout);
#elif SOCK_POSIX
   return ::select(nfds, readfds, writefds, exceptfds, timeout);
#endif
}

inline ssize_t send(Socket socket, const void* buf, size_t len, int flags) {
#if SOCK_WINDOWS
   return ::send(socket, static_cast<const char*>(buf), static_cast<int>(len), flags);
#elif SOCK_POSIX
   return ::send(socket, buf, len, flags);
#endif
}

inline ssize_t sendto(Socket socket, const void* buf, size_t len, int flags, const sockaddr* destAddr, socklen_t addrlen) {
#if SOCK_WINDOWS
   return ::sendto(socket, static_cast<const char*>(buf), static_cast<int>(len), flags, destAddr, addrlen);
#elif SOCK_POSIX
   return ::sendto(socket, buf, len, flags, destAddr, addrlen);
#endif
}

inline int setsockopt(Socket socket, int level, int optionName, const void* optionValue, socklen_t optionLen) {
#if SOCK_WINDOWS
   return ::setsockopt(socket, level, optionName, static_cast<const char*>(optionValue), optionLen);
#elif SOCK_POSIX
   return ::setsockopt(socket, level, optionName, optionValue, optionLen);
#endif
}

inline int shutdown(Socket socket, int how) {
   return ::shutdown(socket, how);
}

inline Socket socket(int domain, int type, int protocol) {
   return ::socket(domain, type, protocol);
}

} // namespace Sock

#endif
