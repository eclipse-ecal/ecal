/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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

int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "person publisher dynamic");

  // set process state
  eCAL::Process::SetState(proc_sev_healthy, proc_sev_level1, "I feel good !");

  // create a dynamic publisher (topic name "person1")
  std::shared_ptr<pb::People::Person> person1{ new pb::People::Person() };
  eCAL::protobuf::CDynamicPublisher pub1("person1", person1);

  // create a dynamic publisher (topic name "person2")
  eCAL::protobuf::CDynamicPublisher pub2("person2", "pb.People.Person");
  std::shared_ptr<pb::People::Person> person2 = pub2.GetAs<pb::People::Person>();

  // enter main loop
  auto cnt = 0;
  while(eCAL::Ok())
  {
    // set person1 object content
    person1->set_id(++cnt);
    person1->set_name("Max");
    person1->set_stype(pb::People::Person_SType_MALE);
    person1->set_email("max@mail.net");
    person1->mutable_dog()->set_name("Brandy");
    person1->mutable_house()->set_rooms(4);

    // set person2 object content
    person2->set_id(++cnt);
    person2->set_name("Romy");
    person2->set_stype(pb::People::Person_SType_FEMALE);
    person2->set_email("romy@mail.net");
    person2->mutable_dog()->set_name("Gorky");
    person2->mutable_house()->set_rooms(4);

    // send the person objects
    pub1.Send();
    pub2.Send();

    // print content
    std::cout << "person id    : " << person1->id()            << std::endl;
    std::cout << "person name  : " << person1->name()          << std::endl;
    std::cout << "person stype : " << person1->stype()         << std::endl;
    std::cout << "person email : " << person1->email()         << std::endl;
    std::cout << "dog.name     : " << person1->dog().name()    << std::endl;
    std::cout << "house.rooms  : " << person1->house().rooms() << std::endl;
    std::cout                                                  << std::endl;

    // print content
    std::cout << "person id    : " << person2->id()            << std::endl;
    std::cout << "person name  : " << person2->name()          << std::endl;
    std::cout << "person stype : " << person2->stype()         << std::endl;
    std::cout << "person email : " << person2->email()         << std::endl;
    std::cout << "dog.name     : " << person2->dog().name()    << std::endl;
    std::cout << "house.rooms  : " << person2->house().rooms() << std::endl;
    std::cout                                                  << std::endl;

    // sleep 500 ms
    eCAL::Process::SleepMS(500);
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
