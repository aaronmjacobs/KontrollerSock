#ifndef KONTROLLER_SOCK_HANDLES_H
#define KONTROLLER_SOCK_HANDLES_H

#include "Sock.h"

namespace KontrollerSock {

// General resource handle
// Default constructable / movable RAII wrapper
template<typename T, T InvalidValue, typename Deleter, Deleter DeleterInst>
struct ResourceHandle {
   ResourceHandle() : data(InvalidValue) {
   }

   ResourceHandle(T value) : data(value) {
   }

   ResourceHandle(ResourceHandle&& other) : data(other.data) {
      other.data = InvalidValue;
   }

   ~ResourceHandle() {
      if (data != InvalidValue) {
         DeleterInst(data);
      }
   }

   ResourceHandle& operator=(ResourceHandle&& other) {
      if (data != InvalidValue) {
         DeleterInst(data);
      }

      data = other.data;
      other.data = InvalidValue;
      return *this;
   }

   explicit operator bool() const {
      return data != InvalidValue;
   }

   T data;
};

// Set up a few specific resource handles

inline void socketSystemDeleter(int initializeResult) {
   Sock::System::terminate();
}
using SocketSystemHandle = ResourceHandle<int, 0, decltype(socketSystemDeleter), socketSystemDeleter>;

inline void socketDeleter(Sock::Socket socket) {
   Sock::shutdown(socket, SHUT_RDWR);
   Sock::close(socket);
}
using SocketHandle = ResourceHandle<Sock::Socket, Sock::kInvalidSocket, decltype(socketDeleter), socketDeleter>;

using AddrInfoHandle = ResourceHandle<addrinfo*, nullptr, decltype(Sock::freeaddrinfo), Sock::freeaddrinfo>;

} // namespace KontrollerSock

#endif
