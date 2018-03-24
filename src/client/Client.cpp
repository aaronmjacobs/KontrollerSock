#include "KontrollerSock/Client.h"

#include <cstdint>

namespace KontrollerSock {

namespace {

float* getDialVal(Kontroller::State &state, Kontroller::Dial dial) {
   switch (dial) {
   case Kontroller::Dial::kGroup1: return &state.groups[0].dial;
   case Kontroller::Dial::kGroup2: return &state.groups[1].dial;
   case Kontroller::Dial::kGroup3: return &state.groups[2].dial;
   case Kontroller::Dial::kGroup4: return &state.groups[3].dial;
   case Kontroller::Dial::kGroup5: return &state.groups[4].dial;
   case Kontroller::Dial::kGroup6: return &state.groups[5].dial;
   case Kontroller::Dial::kGroup7: return &state.groups[6].dial;
   case Kontroller::Dial::kGroup8: return &state.groups[7].dial;
   default: return nullptr;
   }
}

float* getSliderVal(Kontroller::State &state, Kontroller::Slider slider) {
   switch (slider) {
   case Kontroller::Slider::kGroup1: return &state.groups[0].slider;
   case Kontroller::Slider::kGroup2: return &state.groups[1].slider;
   case Kontroller::Slider::kGroup3: return &state.groups[2].slider;
   case Kontroller::Slider::kGroup4: return &state.groups[3].slider;
   case Kontroller::Slider::kGroup5: return &state.groups[4].slider;
   case Kontroller::Slider::kGroup6: return &state.groups[5].slider;
   case Kontroller::Slider::kGroup7: return &state.groups[6].slider;
   case Kontroller::Slider::kGroup8: return &state.groups[7].slider;
   default: return nullptr;
   }
}

bool* getButtonVal(Kontroller::State &state, Kontroller::Button button) {
   switch (button) {
   case Kontroller::Button::kTrackPrevious: return &state.trackPrevious;
   case Kontroller::Button::kTrackNext: return &state.trackNext;
   case Kontroller::Button::kCycle: return &state.cycle;
   case Kontroller::Button::kMarkerSet: return &state.markerSet;
   case Kontroller::Button::kMarkerPrevious: return &state.markerPrevious;
   case Kontroller::Button::kMarkerNext: return &state.markerNext;
   case Kontroller::Button::kRewind: return &state.rewind;
   case Kontroller::Button::kFastForward: return &state.fastForward;
   case Kontroller::Button::kStop: return &state.stop;
   case Kontroller::Button::kPlay: return &state.play;
   case Kontroller::Button::kRecord: return &state.record;
   case Kontroller::Button::kGroup1Solo: return &state.groups[0].solo;
   case Kontroller::Button::kGroup1Mute: return &state.groups[0].mute;
   case Kontroller::Button::kGroup1Record: return &state.groups[0].record;
   case Kontroller::Button::kGroup2Solo: return &state.groups[1].solo;
   case Kontroller::Button::kGroup2Mute: return &state.groups[1].mute;
   case Kontroller::Button::kGroup2Record: return &state.groups[1].record;
   case Kontroller::Button::kGroup3Solo: return &state.groups[2].solo;
   case Kontroller::Button::kGroup3Mute: return &state.groups[2].mute;
   case Kontroller::Button::kGroup3Record: return &state.groups[2].record;
   case Kontroller::Button::kGroup4Solo: return &state.groups[3].solo;
   case Kontroller::Button::kGroup4Mute: return &state.groups[3].mute;
   case Kontroller::Button::kGroup4Record: return &state.groups[3].record;
   case Kontroller::Button::kGroup5Solo: return &state.groups[4].solo;
   case Kontroller::Button::kGroup5Mute: return &state.groups[4].mute;
   case Kontroller::Button::kGroup5Record: return &state.groups[4].record;
   case Kontroller::Button::kGroup6Solo: return &state.groups[5].solo;
   case Kontroller::Button::kGroup6Mute: return &state.groups[5].mute;
   case Kontroller::Button::kGroup6Record: return &state.groups[5].record;
   case Kontroller::Button::kGroup7Solo: return &state.groups[6].solo;
   case Kontroller::Button::kGroup7Mute: return &state.groups[6].mute;
   case Kontroller::Button::kGroup7Record: return &state.groups[6].record;
   case Kontroller::Button::kGroup8Solo: return &state.groups[7].solo;
   case Kontroller::Button::kGroup8Mute: return &state.groups[7].mute;
   case Kontroller::Button::kGroup8Record: return &state.groups[7].record;
   default: return nullptr;
   }
}

enum class ReceiveResult {
   kSuccess,
   kError,
   kTimeout
};

bool receiveData(Sock::Socket socket, uint8_t* data, size_t size) {
   size_t bytesRead = 0;

   while (bytesRead < size) {
      ssize_t result = Sock::recv(socket, data + bytesRead, size - bytesRead, 0);
      if (result <= 0) {
         // Connection lost
         printf("recv failed with error: %d\n", Sock::System::getLastError());
         return false;
      }

      bytesRead += result;
   }

   return true;
}

ReceiveResult receivePacket(Sock::Socket socket, EventPacket& packet) {
   // Wait (with timeout) until there is data available
   fd_set fds;
   FD_ZERO(&fds);
   FD_SET(socket, &fds);
   timeval timeout = { 0, 100'000 }; // 100ms
   int selectResult = Sock::select(socket + 1, &fds, nullptr, nullptr, &timeout);
   if (selectResult < 0) {
      return ReceiveResult::kError;
   } else if (selectResult == 0) {
      return ReceiveResult::kTimeout;
   }

   // Make sure there is at least one full packet's worth of data available
   EventPacket dummyPacket;
   ssize_t bytesReady = Sock::recv(socket, &dummyPacket, sizeof(dummyPacket), MSG_PEEK);
   if (bytesReady < 0) {
      int error = Sock::System::getLastError();
      if (error != Sock::kWouldBlock) {
         return ReceiveResult::kError;
      }
   }
   if (bytesReady != sizeof(EventPacket)) {
      return ReceiveResult::kTimeout;
   }

   // Read the data
   EventPacket networkPacket;
   if (!receiveData(socket, reinterpret_cast<uint8_t*>(&networkPacket), sizeof(networkPacket))) {
      return ReceiveResult::kError;
   }

   // Translate from network byte order to host byte order
   packet.type = Sock::Endian::networkToHostShort(networkPacket.type);
   packet.id = Sock::Endian::networkToHostShort(networkPacket.id);
   packet.value = Sock::Endian::networkToHostLong(networkPacket.value);
   return ReceiveResult::kSuccess;
}

} // namespace

Client::Client() : shuttingDown(false), state{} {
}

void Client::run(const char* endpoint) {
   // Initialize the socket system
   int initializeResult = Sock::System::initialize();
   SocketSystemHandle socketSystemHandle(initializeResult);
   if (initializeResult != 0) {
      printf("Socket system startup failed with error: %d\n", initializeResult);
      return;
   }

   while (!shuttingDown) {
      SocketHandle clientSocket = connect(endpoint);
      if (!clientSocket) {
         std::this_thread::sleep_for(std::chrono::milliseconds(500));
         continue;
      }

      while (!shuttingDown) {
         EventPacket packet;
         ReceiveResult result = receivePacket(clientSocket.data, packet);

         if (result == ReceiveResult::kSuccess) {
            updateState(packet);
         } else if (result == ReceiveResult::kError) {
            break;
         }
      }
   }
}

KontrollerSock::SocketHandle Client::connect(const char* endpoint) {
   SocketHandle clientSocket;

   // Create / connect the socket
   {
      AddrInfoHandle addrInfo;

      addrinfo hints = {};
      hints.ai_family = AF_INET;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_protocol = IPPROTO_TCP;
      hints.ai_flags = AI_PASSIVE;
      int addrInfoResult = Sock::getaddrinfo(endpoint, kPort, &hints, &addrInfo.data);
      if (addrInfoResult != 0) {
         printf("getaddrinfo failed with error: %d\n", addrInfoResult);
         return {};
      }

      clientSocket.data = Sock::socket(addrInfo.data->ai_family, addrInfo.data->ai_socktype, addrInfo.data->ai_protocol);
      if (clientSocket.data == Sock::kInvalidSocket) {
         printf("socket failed with error: %d\n", Sock::System::getLastError());
         return {};
      }

      int connectResult = Sock::connect(clientSocket.data, addrInfo.data->ai_addr, static_cast<socklen_t>(addrInfo.data->ai_addrlen));
      if (connectResult == Sock::kSocketError) {
         printf("connect failed with error: %d\n", Sock::System::getLastError());
         return {};
      }
   }

   unsigned long nonBlocking = 1;
   int ioctrlResult = Sock::ioctl(clientSocket.data, FIONBIO, &nonBlocking);
   if (ioctrlResult == Sock::kSocketError) {
      printf("ioctlsocket failed with error: %d\n", Sock::System::getLastError());
      return {};
   }

   return clientSocket;
}

void Client::updateState(const EventPacket& packet) {
   bool boolValue = packet.value != 0;
   float floatValue = 0.0f;
   static_assert(sizeof(packet.value) == sizeof(floatValue), "Packet data size does not match event data size");
   memcpy(&floatValue, &packet.value, sizeof(floatValue));

   switch (packet.type) {
   case EventPacket::kButton:
      if (bool* buttonValue = getButtonVal(state, static_cast<Kontroller::Button>(packet.id)))
      {
         std::lock_guard<std::mutex> lock(mutex);
         *buttonValue = boolValue;
      }
      break;
   case EventPacket::kDial:
      if (float* dialValue = getDialVal(state, static_cast<Kontroller::Dial>(packet.id)))
      {
         std::lock_guard<std::mutex> lock(mutex);
         *dialValue = floatValue;
      }
      break;
   case EventPacket::kSlider:
      if (float* sliderValue = getSliderVal(state, static_cast<Kontroller::Slider>(packet.id)))
      {
         std::lock_guard<std::mutex> lock(mutex);
         *sliderValue = floatValue;
      }
      break;
   }
}

} // namespace KontrollerSock
