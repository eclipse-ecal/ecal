#include <ecal/ecal.h>
#include <ecal/msg/protobuf/subscriber.h>

#include <iostream>
#include <thread>

#include "hello_world.pb.h"

void HelloWorldCallback(const proto_messages::HelloWorld& hello_world_msg)
{
  std::cout << hello_world_msg.name() << " sent a message with ID "
            << hello_world_msg.id() << ":" << std::endl
            << hello_world_msg.msg() << std::endl << std::endl;
}

int main(int argc, char** argv)
{
  // Initialize eCAL and create a protobuf subscriber
  eCAL::Initialize(argc, argv, "Hello World Protobuf Subscriber");
  eCAL::protobuf::CSubscriber<proto_messages::HelloWorld> subscriber("hello_world_protobuf");

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
