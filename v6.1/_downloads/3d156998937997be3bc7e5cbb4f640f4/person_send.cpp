/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

// Including the eCAL convenience header
#include <ecal/ecal.h>
// In addition we include the msg protobuf publisher
#include <ecal/msg/protobuf/publisher.h>

#include <iostream>

// Here we include the compiled protobuf header for the message "Person"
#include "person.pb.h"

int main()
{
  std::cout << "--------------------" << std::endl;
  std::cout << " C++: PERSON SENDER"  << std::endl;
  std::cout << "--------------------" << std::endl;

  /*
    Initialize eCAL. You always have to initialize eCAL before using its API.
    The name of our eCAL Process will be "person send". 
    This name will be visible in the eCAL Monitor, once the process is running.
  */
  eCAL::Initialize("person send");

  /*
    Print some eCAL version information.
  */
  std::cout << "eCAL " << eCAL::GetVersionString() << " (" << eCAL::GetVersionDateString() << ")" << "\n";

  /*
    Set the state for the program.
    You can vary between different states like healthy, warning, critical ...
    This can be used to communicate the application state to applications like eCAL Monitor/Sys.
  */
  eCAL::Process::SetState(eCAL::Process::eSeverity::healthy, eCAL::Process::eSeverityLevel::level1, "I feel good!");

  /*
    Now we create a new publisher that will publish the topic "person".
    The data type is "pb.People.Person", generated from the protobuf definition.    
  */
  eCAL::protobuf::CPublisher<pb::People::Person> publisher("person");

  /*
    Construct a message. The message is a protobuf struct that will be sent to the subscribers.
  */
  pb::People::Person person;
  person.set_id(0);
  person.set_name("Max");
  person.set_stype(pb::People::Person_SType_MALE);
  person.set_email("max@mail.net");
  person.mutable_dog()->set_name("Brandy");
  person.mutable_house()->set_rooms(4);

  /*
    Creating an infinite publish-loop.
    eCAL Supports a stop signal; when an eCAL Process is stopped, eCAL_Ok() will return false.
  */
  auto loop_count = 0;
  while(eCAL::Ok())
  {
    /*
      Change in each loop the content of the message to see a difference per message.
    */
    person.set_id(loop_count++);

    /*
      Send the message. The message is sent to all subscribers that are currently connected to the topic "person".
    */
    if (publisher.Send(person)) 
    {
      std::cout << "----------------------------------"        << "\n";
      std::cout << "Sent protobuf message in C++: "            << "\n";
      std::cout << "----------------------------------"        << "\n";
      std::cout << "person id    : " << person.id()            << "\n";
      std::cout << "person name  : " << person.name()          << "\n";
      std::cout << "person stype : " << person.stype()         << "\n";
      std::cout << "person email : " << person.email()         << "\n";
      std::cout << "dog.name     : " << person.dog().name()    << "\n";
      std::cout << "house.rooms  : " << person.house().rooms() << "\n";
      std::cout << "----------------------------------"        << "\n";
      std::cout                                                << "\n";
    }
    else
    {
      std::cout << "Failed to send Protobuf message in C++!"   << "\n";
    }

    /*
      Sleep for 500ms to send in a frequency of 2 hz.
    */
    eCAL::Process::SleepMS(500);
  }

  /*
    Finalize eCAL. This will stop all eCAL processes and free all resources.
    You should always finalize eCAL when you are done using it.
  */
  eCAL::Finalize();

  return(0);
}
