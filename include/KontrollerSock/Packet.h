#ifndef KONTROLLER_SOCK_PACKET_H
#define KONTROLLER_SOCK_PACKET_H

#include <cstdint>

namespace KontrollerSock {

static const char* kPort = "40807";

struct EventPacket {
   enum Type : uint16_t {
      kButton = 0x0001,
      kDial = 0x0002,
      kSlider = 0x0003
   };

   uint16_t type;
   uint16_t id;
   uint32_t value;
};

} // namespace KontrollerSock

#endif
