#ifndef KONTROLLER_SOCK_CLIENT_H
#define KONTROLLER_SOCK_CLIENT_H

#include "KontrollerSock/Handles.h"
#include "KontrollerSock/Packet.h"

#include <Kontroller/Kontroller.h>

#include <atomic>
#include <thread>

namespace KontrollerSock {

class Client {
public:
   Client();

   void run(const char* endpoint);

   void shutDown() {
      shuttingDown = true;
   }

   Kontroller::State getState() {
      std::lock_guard<std::mutex> lock(mutex);
      return state;
   }

private:
   SocketHandle connect(const char* endpoint);
   void updateState(const EventPacket& packet);

   std::atomic_bool shuttingDown;
   std::mutex mutex;
   Kontroller::State state;
};

} // namespace KontrollerSock

#endif
