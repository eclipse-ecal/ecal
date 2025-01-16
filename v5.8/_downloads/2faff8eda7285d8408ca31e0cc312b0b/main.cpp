#include <ecal/ecal.h>
#include <ecal/msg/protobuf/publisher.h>

#include <iostream>
#include <thread>

#include "hello_world.pb.h"

int main(int argc, char** argv)
{
  // Initialize eCAL and create a protobuf publisher
  eCAL::Initialize(argc, argv, "Hello World Protobuf Publisher");
  eCAL::protobuf::CPublisher<proto_messages::HelloWorld> publisher("hello_world_protobuf");

  // Ask the user to input his name
  std::cout << "Please enter your name: ";
  std::string name;
  std::getline(std::cin, name);

  unsigned int id = 0;

  // Infinite loop (using eCAL::Ok() will enable us to gracefully shutdown the
  // Process from another application)
  while (eCAL::Ok())
  {
    // Let the user input a message
    std::cout << "Type the message you want to send: ";
    std::string message;
    std::getline(std::cin, message);

    // Create a protobuf message object
    proto_messages::HelloWorld hello_world_message;
    hello_world_message.set_name(name);
    hello_world_message.set_msg (message);
    hello_world_message.set_id  (id++);

    // Send the message
    publisher.Send(hello_world_message);
    std::cout << "Sent message!" << std::endl << std::endl;
  }

  // finalize eCAL API
  eCAL::Finalize();
}
