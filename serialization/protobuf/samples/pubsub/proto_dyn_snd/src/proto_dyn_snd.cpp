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

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/dynamic_publisher.h>

#include <iostream>

#include "person.pb.h"

int main()
{
  // initialize eCAL API
  eCAL::Initialize("person publisher dynamic");

  // set process state
  eCAL::Process::SetState(eCAL::Process::eSeverity::healthy, eCAL::Process::eSeverityLevel::level1, "I feel good !");

  // create a dynamic publisher (topic name "person")
  eCAL::protobuf::CDynamicPublisher pub1("person", pb::People::Person);

  // create a dynamic publisher (topic name "person")
  eCAL::protobuf::CDynamicPublisher pub2("person", "pb.People.Person");
  pb::People::Person person;

  // set person1 object content
  person.set_name("Max");
  person.set_stype(pb::People::Person_SType_MALE);
  person.set_email("max@mail.net");
  person.mutable_dog()->set_name("Brandy");
  person.mutable_house()->set_rooms(4);

  // enter main loop
  auto                                cnt = 0;
  std::shared_ptr<pb::People::Person> person;

  while(eCAL::Ok())
  {
    person.set_id(cnt);
    if (++cnt % 2)
    {
      // modify and send the person1 object
      pub1.Send(person);
    }
    else
    {
      // modify and send the person2 object
      pub2.Send(person);
    }

    // print current person message
    std::cout << "person id    : " << person.id()            << std::endl;
    std::cout << "person name  : " << person.name()          << std::endl;
    std::cout << "person stype : " << person.stype()         << std::endl;
    std::cout << "person email : " << person.email()         << std::endl;
    std::cout << "dog.name     : " << person.dog().name()    << std::endl;
    std::cout << "house.rooms  : " << person.house().rooms() << std::endl;
    std::cout << std::endl;

    // sleep 500 ms
    eCAL::Process::SleepMS(500);
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
