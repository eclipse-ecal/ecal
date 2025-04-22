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
#include <ecal/msg/protobuf/subscriber.h>

#include <iostream>

#include "person.pb.h"

/*
  Here we create the subscriber callback function that is called everytime,
  when a new message arrived from a publisher.
*/
void OnPerson(const eCAL::STopicId& topic_id_, const pb::People::Person& person_, const long long time_, const long long clock_)
{
  std::cout << "------------------------------------------"                    << "\n";
  std::cout << " Received Protobuf message in C++ "                            << "\n";
  std::cout << "------------------------------------------"                    << "\n";
  std::cout << " topic name   : " << topic_id_.topic_name                      << "\n";
  std::cout << " topic time   : " << time_                                     << "\n";
  std::cout << " topic clock  : " << clock_                                    << "\n";
  std::cout << ""                                                              << "\n";
  std::cout << " Content of message type \"" << person_.GetTypeName()  << "\"" << "\n";
  std::cout << "------------------------------------------"                    << "\n";
  std::cout << " id          : " << person_.id()                               << "\n";
  std::cout << " name        : " << person_.name()                             << "\n";
  std::cout << " stype       : " << person_.stype()                            << "\n";
  std::cout << " email       : " << person_.email()                            << "\n";
  std::cout << " dog.name    : " << person_.dog().name()                       << "\n";
  std::cout << " house.rooms : " << person_.house().rooms()                    << "\n";
  std::cout << "------------------------------------------"                    << "\n";
  std::cout                                                                    << "\n";
}

int main()
{
  std::cout << "----------------------" << std::endl;
  std::cout << " C++: PERSON RECEIVER"  << std::endl;
  std::cout << "----------------------" << std::endl;

  /*
    Initialize eCAL. You always have to initialize eCAL before using its API.
    The name of our eCAL Process will be "person receive". 
    This name will be visible in the eCAL Monitor, once the process is running.
  */
  eCAL::Initialize("person receive");

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
    Creating the eCAL Subscriber. An eCAL Process can create multiple subscribers (and publishers).
    The topic we are going to receive is called "person".
    The data type is "Pb.People.Person", generated from the protobuf definition.
  */
  eCAL::protobuf::CSubscriber<pb::People::Person> subscriber("person");

  /*
    Create and register a receive callback. The callback will be called whenever a new message is received.
  */
  subscriber.SetReceiveCallback(&OnPerson);

  /*
    Creating an infinite loop.
    eCAL Supports a stop signal; when an eCAL Process is stopped, eCAL::Ok() will return false.
  */
  while(eCAL::Ok())
  {
    /*
      Sleep for 500ms to avoid busy waiting.
    */
    eCAL::Process::SleepMS(500);
  }

  /*
    Finalize eCAL. This will stop all eCAL processes and free all resources.
    You should always finalize eCAL before exiting your application.
  */
  eCAL::Finalize();
  
  return(0);
}
