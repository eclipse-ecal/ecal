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

#include <iostream>

#include "person.pb.h"

void OnEvent(const eCAL::STopicId& topic_id_, const eCAL::SPubEventCallbackData& data_)
{
  std::cout << "topic name       : " << topic_id_.topic_name << std::endl;
  switch (data_.event_type)
  {
  case eCAL::ePublisherEvent::connected:
    std::cout << "event            : " << "connected" << std::endl;
    break;
  case eCAL::ePublisherEvent::disconnected:
    std::cout << "event            : " << "disconnected" << std::endl;
    break;
  case eCAL::ePublisherEvent::dropped:
    std::cout << "event            : " << "dropped" << std::endl;
    break;
  default:
    std::cout << "event            : " << "unknown" << std::endl;
    break;
  }
  std::cout << std::endl;
}

int main()
{
  // initialize eCAL API
  eCAL::Initialize("person publisher events");

  // set process state
  eCAL::Process::SetState(eCAL::Process::eSeverity::healthy, eCAL::Process::eSeverityLevel::level1, "I feel good !");

  // create a publisher (topic name "person")
  eCAL::protobuf::CPublisher<pb::People::Person> pub("person", OnEvent);

  // generate a class instance of Person
  pb::People::Person person;

  // start application and wait for events
  std::cout << "Please start 'person_rec_events sample." << std::endl << std::endl;

  // enter main loop
  auto cnt(0);
  while (eCAL::Ok() && (++cnt <= 10))
  {
    // set person object content
    person.set_id(cnt);
    person.set_name("Max");
    person.set_stype(pb::People::Person_SType_MALE);
    person.set_email("max@mail.net");
    person.mutable_dog()->set_name("Brandy");
    person.mutable_house()->set_rooms(4);

    // send the person object
    pub.Send(person);

    // sleep 500 ms
    eCAL::Process::SleepMS(1000);
  }

  // finalize eCAL API
  eCAL::Finalize();

  // keep alive for 10 seconds
  eCAL::Process::SleepMS(10000);

  return(0);
}
