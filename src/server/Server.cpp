#include "KontrollerSock/Packet.h"
#include "KontrollerSock/Server.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#include <chrono>

namespace KontrollerSock {

namespace {

bool sendData(SOCKET socket, const char* data, size_t size) {
   size_t bytesWritten = 0;

   while (bytesWritten < size) {
      int result = send(socket, data + bytesWritten, static_cast<int>(size - bytesWritten), 0);
      if (result == SOCKET_ERROR) {
         // Connection lost
         return false;
      }

      assert(result > 0);
      bytesWritten += result;
   }

   return true;
}

bool sendPacket(SOCKET socket, EventPacket packet) {
   EventPacket networkPacket;
   networkPacket.type = htons(packet.type);
   networkPacket.id = htons(packet.id);
   networkPacket.value = htonl(packet.value);

   return sendData(socket, reinterpret_cast<const char*>(&networkPacket), sizeof(networkPacket));
}

bool sendEvents(SOCKET socket, const std::vector<ButtonEvent>& buttonEvents, const std::vector<DialEvent>& dialEvents, const std::vector<SliderEvent>& sliderEvents) {
   bool success = true;

   for (ButtonEvent buttonEvent : buttonEvents) {
      EventPacket packet;
      packet.type = EventPacket::kButton;
      packet.id = static_cast<uint16_t>(buttonEvent.button);
      packet.value = static_cast<uint32_t>(buttonEvent.pressed);

      success = success && sendPacket(socket, packet);
   }

   for (DialEvent dialEvent : dialEvents) {
      EventPacket packet;
      packet.type = EventPacket::kDial;
      packet.id = static_cast<uint16_t>(dialEvent.dial);
      static_assert(sizeof(packet.value) == sizeof(dialEvent.value), "Packet data size does not match event data size");
      memcpy(&packet.value, &dialEvent.value, sizeof(packet.value));

      success = success && sendPacket(socket, packet);
   }

   for (SliderEvent sliderEvent : sliderEvents) {
      EventPacket packet;
      packet.type = EventPacket::kSlider;
      packet.id = static_cast<uint16_t>(sliderEvent.slider);
      static_assert(sizeof(packet.value) == sizeof(sliderEvent.value), "Packet data size does not match event data size");
      memcpy(&packet.value, &sliderEvent.value, sizeof(packet.value));

      success = success && sendPacket(socket, packet);
   }

   return success;
}

bool sendInitialState(SOCKET socket, const Kontroller::State& state) {
   std::vector<ButtonEvent> buttonEvents;
   buttonEvents.reserve(35);
   buttonEvents.push_back({ Kontroller::Button::kTrackPrevious, state.trackPrevious });
   buttonEvents.push_back({ Kontroller::Button::kTrackNext, state.trackNext });
   buttonEvents.push_back({ Kontroller::Button::kCycle, state.cycle });
   buttonEvents.push_back({ Kontroller::Button::kMarkerSet, state.markerSet });
   buttonEvents.push_back({ Kontroller::Button::kMarkerPrevious, state.markerPrevious });
   buttonEvents.push_back({ Kontroller::Button::kMarkerNext, state.markerNext });
   buttonEvents.push_back({ Kontroller::Button::kRewind, state.rewind });
   buttonEvents.push_back({ Kontroller::Button::kFastForward, state.fastForward });
   buttonEvents.push_back({ Kontroller::Button::kStop, state.stop });
   buttonEvents.push_back({ Kontroller::Button::kPlay, state.play });
   buttonEvents.push_back({ Kontroller::Button::kRecord, state.record });
   buttonEvents.push_back({ Kontroller::Button::kGroup1Solo, state.groups[0].solo });
   buttonEvents.push_back({ Kontroller::Button::kGroup1Mute, state.groups[0].mute });
   buttonEvents.push_back({ Kontroller::Button::kGroup1Record, state.groups[0].record });
   buttonEvents.push_back({ Kontroller::Button::kGroup2Solo, state.groups[1].solo });
   buttonEvents.push_back({ Kontroller::Button::kGroup2Mute, state.groups[1].mute });
   buttonEvents.push_back({ Kontroller::Button::kGroup2Record, state.groups[1].record });
   buttonEvents.push_back({ Kontroller::Button::kGroup3Solo, state.groups[2].solo });
   buttonEvents.push_back({ Kontroller::Button::kGroup3Mute, state.groups[2].mute });
   buttonEvents.push_back({ Kontroller::Button::kGroup3Record, state.groups[2].record });
   buttonEvents.push_back({ Kontroller::Button::kGroup4Solo, state.groups[3].solo });
   buttonEvents.push_back({ Kontroller::Button::kGroup4Mute, state.groups[3].mute });
   buttonEvents.push_back({ Kontroller::Button::kGroup4Record, state.groups[3].record });
   buttonEvents.push_back({ Kontroller::Button::kGroup5Solo, state.groups[4].solo });
   buttonEvents.push_back({ Kontroller::Button::kGroup5Mute, state.groups[4].mute });
   buttonEvents.push_back({ Kontroller::Button::kGroup5Record, state.groups[4].record });
   buttonEvents.push_back({ Kontroller::Button::kGroup6Solo, state.groups[5].solo });
   buttonEvents.push_back({ Kontroller::Button::kGroup6Mute, state.groups[5].mute });
   buttonEvents.push_back({ Kontroller::Button::kGroup6Record, state.groups[5].record });
   buttonEvents.push_back({ Kontroller::Button::kGroup7Solo, state.groups[6].solo });
   buttonEvents.push_back({ Kontroller::Button::kGroup7Mute, state.groups[6].mute });
   buttonEvents.push_back({ Kontroller::Button::kGroup7Record, state.groups[6].record });
   buttonEvents.push_back({ Kontroller::Button::kGroup8Solo, state.groups[7].solo });
   buttonEvents.push_back({ Kontroller::Button::kGroup8Mute, state.groups[7].mute });
   buttonEvents.push_back({ Kontroller::Button::kGroup8Record, state.groups[7].record });

   std::vector<DialEvent> dialEvents;
   dialEvents.reserve(8);
   dialEvents.push_back({ Kontroller::Dial::kGroup1, state.groups[0].dial });
   dialEvents.push_back({ Kontroller::Dial::kGroup2, state.groups[1].dial });
   dialEvents.push_back({ Kontroller::Dial::kGroup3, state.groups[2].dial });
   dialEvents.push_back({ Kontroller::Dial::kGroup4, state.groups[3].dial });
   dialEvents.push_back({ Kontroller::Dial::kGroup5, state.groups[4].dial });
   dialEvents.push_back({ Kontroller::Dial::kGroup6, state.groups[5].dial });
   dialEvents.push_back({ Kontroller::Dial::kGroup7, state.groups[6].dial });
   dialEvents.push_back({ Kontroller::Dial::kGroup8, state.groups[7].dial });

   std::vector<SliderEvent> sliderEvents;
   sliderEvents.reserve(8);
   sliderEvents.push_back({ Kontroller::Slider::kGroup1, state.groups[0].slider });
   sliderEvents.push_back({ Kontroller::Slider::kGroup2, state.groups[1].slider });
   sliderEvents.push_back({ Kontroller::Slider::kGroup3, state.groups[2].slider });
   sliderEvents.push_back({ Kontroller::Slider::kGroup4, state.groups[3].slider });
   sliderEvents.push_back({ Kontroller::Slider::kGroup5, state.groups[4].slider });
   sliderEvents.push_back({ Kontroller::Slider::kGroup6, state.groups[5].slider });
   sliderEvents.push_back({ Kontroller::Slider::kGroup7, state.groups[6].slider });
   sliderEvents.push_back({ Kontroller::Slider::kGroup8, state.groups[7].slider });

   return sendEvents(socket, buttonEvents, dialEvents, sliderEvents);
}

struct WinsockHandle {
   WinsockHandle() {
      startupResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
   }

   ~WinsockHandle() {
      if (startupResult == 0) {
         WSACleanup();
      }
   }

   WSADATA wsaData;
   int startupResult;
};

struct SocketHandle {
   SocketHandle(SOCKET sock) : socket(sock) {
   }

   ~SocketHandle() {
      if (socket != INVALID_SOCKET) {
         closesocket(socket);
      }
   }

   SOCKET socket;
};

struct AddrInfoHandle {
   AddrInfoHandle(struct addrinfo* addr) : info(addr) {
   }

   ~AddrInfoHandle() {
      if (info) {
         freeaddrinfo(info);
      }
   }

   struct addrinfo* info;
};

} // namespace

Server::Server()
   : shuttingDown(false), threadCounter(0), globalKontrollerState{} {
}

Server::~Server() {
   shutDown();
}

bool Server::run() {
   Kontroller kontroller;

   kontroller.setButtonCallback([this, &kontroller](Kontroller::Button button, bool pressed) {
      std::lock_guard<std::mutex> lock(threadDataMutex);

      globalKontrollerState = kontroller.getState();

      ButtonEvent event;
      event.button = button;
      event.pressed = pressed;

      for (auto& pair : threadData) {
         {
            std::lock_guard<std::mutex> eventLock(pair.second->eventMutex);
            pair.second->buttonEvents.push_back(event);
         }

         pair.second->cv.notify_all();
      }
   });

   kontroller.setDialCallback([this, &kontroller](Kontroller::Dial dial, float value) {
      std::lock_guard<std::mutex> lock(threadDataMutex);

      globalKontrollerState = kontroller.getState();

      DialEvent event;
      event.dial = dial;
      event.value = value;

      for (auto& pair : threadData) {
         {
            std::lock_guard<std::mutex> eventLock(pair.second->eventMutex);
            pair.second->dialEvents.push_back(event);
         }

         pair.second->cv.notify_all();
      }
   });

   kontroller.setSliderCallback([this, &kontroller](Kontroller::Slider slider, float value) {
      std::lock_guard<std::mutex> lock(threadDataMutex);

      globalKontrollerState = kontroller.getState();

      SliderEvent event;
      event.slider = slider;
      event.value = value;

      for (auto& pair : threadData) {
         {
            std::lock_guard<std::mutex> eventLock(pair.second->eventMutex);
            pair.second->sliderEvents.push_back(event);
         }

         pair.second->cv.notify_all();
      }
   });

   // Initialize Winsock, set up listen socket
   WinsockHandle winsockHandle;
   if (winsockHandle.startupResult != 0) {
      printf("WSAStartup failed with error: %d\n", winsockHandle.startupResult);
      return false;
   }

   {
      SocketHandle listenSocket(INVALID_SOCKET);

      {
         AddrInfoHandle addrInfo(nullptr);

         struct addrinfo hints = {};
         hints.ai_family = AF_INET;
         hints.ai_socktype = SOCK_STREAM;
         hints.ai_protocol = IPPROTO_TCP;
         hints.ai_flags = AI_PASSIVE;
         int addrInfoResult = getaddrinfo(NULL, kPort, &hints, &addrInfo.info);
         if (addrInfoResult != 0) {
            printf("getaddrinfo failed with error: %d\n", addrInfoResult);
            return false;
         }

         listenSocket.socket = socket(addrInfo.info->ai_family, addrInfo.info->ai_socktype, addrInfo.info->ai_protocol);
         if (listenSocket.socket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            return false;
         }

         int bindResult = bind(listenSocket.socket, addrInfo.info->ai_addr, (int)addrInfo.info->ai_addrlen);
         if (bindResult == SOCKET_ERROR) {
            printf("bind failed with error: %d\n", WSAGetLastError());
            return false;
         }
      }

      int listenResult = listen(listenSocket.socket, SOMAXCONN);
      if (listenResult == SOCKET_ERROR) {
         printf("listen failed with error: %d\n", WSAGetLastError());
         return false;
      }

      unsigned long nonBlocking = 1;
      int ioctrlResult = ioctlsocket(listenSocket.socket, FIONBIO, &nonBlocking);
      if (ioctrlResult == SOCKET_ERROR) {
         printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
         return false;
      }

      while (!shuttingDown) {
         // Wait until a client attempts to connect, or we are shutting down
         SOCKET clientSocket = INVALID_SOCKET;
         while (!shuttingDown && clientSocket == INVALID_SOCKET) {
            clientSocket = accept(listenSocket.socket, NULL, NULL);

            if (clientSocket == INVALID_SOCKET) {
               int error = WSAGetLastError();
               if (error != WSAEWOULDBLOCK) {
                  printf("accept failed with error: %d\n", WSAGetLastError());
                  return false;
               }

               std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
         }

         // Spin off a new thread to manage the connection to the client
         if (!shuttingDown) {
            uint64_t newThreadId = 0;
            {
               std::lock_guard<std::mutex> lock(threadDataMutex);
               newThreadId = threadCounter++;

               assert(threadData.count(newThreadId) == 0);
               threadData[newThreadId] = nullptr;
            }

            std::thread thread([this](uint64_t id, SOCKET socket) { manageConnection(id, socket); }, newThreadId, clientSocket);
            thread.detach();
         }
      }
   }

   // Wait for the threads to terminate
   assert(shuttingDown);
   {
      std::unique_lock<std::mutex> lock(threadDataMutex);

      for (auto& pair : threadData) {
         pair.second->cv.notify_all();
      }

      while (!threadData.empty()) {
         threadDataCv.wait_for(lock, std::chrono::milliseconds(1), [this]() { return threadData.empty(); });
      }
   }

   return true;
}

void Server::shutDown() {
   shuttingDown = true;
}

void Server::manageConnection(uint64_t id, uint64_t uintSocket) {
   SOCKET socket = static_cast<SOCKET>(uintSocket);

   std::shared_ptr<ThreadData> data = std::make_shared<ThreadData>();
   Kontroller::State localKontrollerState;

   // Register ourselves
   {
      std::lock_guard<std::mutex> lock(threadDataMutex);

      assert(threadData.count(id) == 1 && threadData[id] == nullptr); // Space should be reserved for us, but no data allocated yet
      threadData[id] = data;
      localKontrollerState = globalKontrollerState;
   }

   BOOL tcpNoDelay = TRUE;
   int optResult = setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&tcpNoDelay), sizeof(tcpNoDelay));
   if (optResult == SOCKET_ERROR) {
      printf("Unable to disable the Nagle algorithm, connection may be jittery!\n");
   }

   if (sendInitialState(socket, localKontrollerState)) {
      std::vector<ButtonEvent> buttonEvents;
      std::vector<DialEvent> dialEvents;
      std::vector<SliderEvent> sliderEvents;

      while (!shuttingDown) {
         // Wait for events, copy them to local storage
         {
            std::unique_lock<std::mutex> lock(data->eventMutex);
            data->cv.wait(lock, [this, &data]() {
               return shuttingDown || !data->buttonEvents.empty() || !data->dialEvents.empty() || !data->sliderEvents.empty();
            });

            if (shuttingDown) {
               break;
            }

            buttonEvents = data->buttonEvents;
            data->buttonEvents.clear();

            dialEvents = data->dialEvents;
            data->dialEvents.clear();

            sliderEvents = data->sliderEvents;
            data->sliderEvents.clear();
         }

         // Send events to client
         bool success = sendEvents(socket, buttonEvents, dialEvents, sliderEvents);
         buttonEvents.clear();
         dialEvents.clear();
         sliderEvents.clear();

         if (!success) {
            break;
         }
      }
   }

   // Close the connection
   shutdown(socket, SD_BOTH);
   closesocket(socket);

   // Unregister ourselves
   {
      std::lock_guard<std::mutex> lock(threadDataMutex);

      assert(threadData.count(id) == 1); // Somehow we're not in the map?
      threadData.erase(id);
   }
}

} // namespace KontrollerSock
