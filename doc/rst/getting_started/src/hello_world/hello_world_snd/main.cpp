#include <ecal/ecal.h>
#include <ecal/msg/string/publisher.h>

#include <iostream>
#include <thread>

int main(int argc, char** argv)
{
  // Initialize eCAL. The name of our Process will be "Hello World Publisher"
  eCAL::Initialize(argc, argv, "Hello World Publisher");

  // Create a String Publisher that publishes on the topic "hello_world_topic"
  eCAL::string::CPublisher<std::string> publisher("hello_world_topic");

  // Create a counter, so something changes in our message
  int counter = 0;

  // Infinite loop (using eCAL::Ok() will enable us to gracefully shutdown the
  // Process from another application)
  while (eCAL::Ok())
  {
    // Create a message with a counter an publish it to the topic
    std::string message = "Hello World " + std::to_string(++counter);
    std::cout << "Sending message: " << message << std::endl;
    publisher.Send(message);

    // Sleep 500 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  // finalize eCAL API
  eCAL::Finalize();
}
