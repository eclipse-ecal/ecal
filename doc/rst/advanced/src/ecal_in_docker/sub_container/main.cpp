#include <ecal/ecal.h>
#include <ecal/msg/string/subscriber.h>

#include <iostream>
#include <thread>

void HelloWorldCallback(const std::string& msg)
{
  std::cout << "Received Message: " << msg << std::endl;
}

int main(int argc, char **argv)
{
  std::cout << "Subscriber Is Starting...\n";  
  eCAL::Initialize(argc, argv, "Hello World Subscriber");
  eCAL::string::CSubscriber<std::string> subscriber("eCAL:hello_world_topic");
  subscriber.AddReceiveCallback(std::bind(&HelloWorldCallback, std::placeholders::_2));  
  while(eCAL::Ok())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }  
  eCAL::Finalize();
}