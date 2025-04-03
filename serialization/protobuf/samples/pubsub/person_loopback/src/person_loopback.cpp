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
#include <ecal/msg/protobuf/publisher.h>
#include <ecal/msg/protobuf/subscriber.h>

#include <iostream>

#include "person.pb.h"

int main()
{
  // initialize eCAL API
  eCAL::Initialize("person publisher");

  // set process state
  eCAL::Process::SetState(eCAL::Process::eSeverity::healthy, eCAL::Process::eSeverityLevel::level1, "I feel good !");

  // create a publisher (topic name "person")
  eCAL::protobuf::CPublisher<pb::People::Person> pub("person");

  // generate a class instance of Person
  pb::People::Person person;

  eCAL::protobuf::CSubscriber<pb::People::Person> sub("person");
  auto receive_lambda = [](const eCAL::STopicId& /*topic_id_*/, const pb::People::Person& person_, const long long /*time_*/, const long long /*clock_*/){
    std::cout << "------------------------------------------" << std::endl;
    std::cout << " RECEIVED                                 " << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    std::cout << "person id    : " << person_.id()            << std::endl;
    std::cout << "person name  : " << person_.name()          << std::endl;
    std::cout << "person stype : " << person_.stype()         << std::endl;
    std::cout << "person email : " << person_.email()         << std::endl;
    std::cout << "dog.name     : " << person_.dog().name()    << std::endl;
    std::cout << "house.rooms  : " << person_.house().rooms() << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    std::cout                                                 << std::endl;

  };
  sub.SetReceiveCallback(receive_lambda);

  // enter main loop
  auto cnt = 0;
  while(eCAL::Ok())
  {
    // set person object content
    person.set_id(++cnt);
    person.set_name("Max");
    person.set_stype(pb::People::Person_SType_MALE);
    person.set_email("max@mail.net");
    person.mutable_dog()->set_name("Brandy");
    person.mutable_house()->set_rooms(4);

    // send the person object
    pub.Send(person);

    // print content
    std::cout << "------------------------------------------" << std::endl;
    std::cout << " SENT                                     " << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    std::cout << "person id    : " << person.id()            << std::endl;
    std::cout << "person name  : " << person.name()          << std::endl;
    std::cout << "person stype : " << person.stype()         << std::endl;
    std::cout << "person email : " << person.email()         << std::endl;
    std::cout << "dog.name     : " << person.dog().name()    << std::endl;
    std::cout << "house.rooms  : " << person.house().rooms() << std::endl;
    std::cout                                                << std::endl;

    // sleep 500 ms
    eCAL::Process::SleepMS(500);
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
