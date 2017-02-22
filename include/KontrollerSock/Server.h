#ifndef KONTROLLER_SOCK_SERVER_H
#define KONTROLLER_SOCK_SERVER_H

#include <Kontroller/Kontroller.h>

#include <atomic>
#include <cstdint>
#include <map>
#include <thread>
#include <vector>

namespace KontrollerSock {

struct ButtonEvent {
   Kontroller::Button button;
   bool pressed;
};

struct DialEvent {
   Kontroller::Dial dial;
   float value;
};

struct SliderEvent {
   Kontroller::Slider slider;
   float value;
};

class Server {
public:
   Server();

   ~Server();

   bool run();

   void shutDown();

private:
   struct ThreadData {
      std::mutex eventMutex;
      std::condition_variable cv;

      std::vector<ButtonEvent> buttonEvents;
      std::vector<DialEvent> dialEvents;
      std::vector<SliderEvent> sliderEvents;
   };

   void initCallbacks(Kontroller& kontroller);

   void manageConnection(uint64_t id, uint64_t socket);

   std::atomic_bool shuttingDown;
   uint64_t threadCounter;

   std::mutex threadDataMutex;
   std::condition_variable threadDataCv;
   std::map<uint64_t, std::shared_ptr<ThreadData>> threadData;
   Kontroller::State globalKontrollerState;
};

} // namespace KontrollerSock

#endif
