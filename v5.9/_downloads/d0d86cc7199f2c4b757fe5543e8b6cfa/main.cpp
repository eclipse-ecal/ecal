#include <ecal/ecal.h>
#include <ecal/msg/string/subscriber.h>

#include <iostream>
#include <thread>

// Callback for receiving messages
void HelloWorldCallback(const std::string& message)
{
  std::cout << "Received Message: " << message << std::endl;
}

int main(int argc, char** argv)
{
  // Initialize eCAL
  eCAL::Initialize(argc, argv, "Hello World Subscriber");

  // Create a subscriber that listenes on the "hello_world_topic"
  eCAL::string::CSubscriber<std::string> subscriber("hello_world_topic");

  // Set the Callback
  subscriber.AddReceiveCallback(std::bind(&HelloWorldCallback, std::placeholders::_2));

  // Just don't exit
  while (eCAL::Ok())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  // finalize eCAL API
  eCAL::Finalize();
}
