#include <ecal/ecal.h>
#include <ecal/msg/string/subscriber.h>
#include <thread>
#include <iostream>

void OnReceiveCrazy(const char* topic_name_, const std::string& msg_, long long time_, long long clock_, long long id_)
{
  static long long last_clock = 0;
  if (clock_ < last_clock)
  {
    std::cout << "Received old data clock_ " << clock_ << " < " << last_clock << std::endl;
   }
  last_clock = clock_;
}

int main()
{
  eCAL::Initialize(0, nullptr, "Receive Crazy");

  eCAL::string::CSubscriber<std::string> sub_crazy("foo");
  sub_crazy.AddReceiveCallback(OnReceiveCrazy);

  while (eCAL::Ok())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  eCAL::Finalize();
}