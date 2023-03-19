#include <ecal/ecal.h>
#include <ecal/msg/string/publisher.h>
#include <thread>

int main()
{
  eCAL::Initialize(0, nullptr, "Quick Publisher");

  eCAL::string::CPublisher<std::string> pub("foo");
  pub.ShmSetBufferCount(2);

  while (eCAL::Ok())
  {
    pub.Send("Hello World");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  eCAL::Finalize();
}