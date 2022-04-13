#include <ecal/ecal.h>
#include <ecal/msg/string/publisher.h>

#include <iostream>
#include <thread>

int main(int argc, char **argv)
{
  std::cout << "Publisher Is Starting...\n";  
  eCAL::Initialize(argc, argv, "Hello World Publisher");
  eCAL::string::CPublisher<std::string> publisher("eCAL:hello_world_topic");  
  publisher.SetLayerMode(eCAL::TLayer::tlayer_all, eCAL::TLayer::smode_off);
  // enable this to publish messages over udp
  //publisher.SetLayerMode(eCAL::TLayer::tlayer_udp_mc, eCAL::TLayer::smode_on);
  // this is enabled to publish messages over shared memory
  publisher.SetLayerMode(eCAL::TLayer::tlayer_shm, eCAL::TLayer::smode_on);  
  int counter = 0;
  std::string msg = "";
  while(eCAL::Ok())
  {
    msg = "Hello World " + std::to_string(++counter);
    publisher.Send(msg);  
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }  
  eCAL::Finalize();
}