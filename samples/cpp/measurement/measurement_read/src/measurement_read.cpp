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

#include <ecal/msg/proto/message.h>
#include <ecal/measurement/imeasurement.h>

#include <iostream>

#include "person.pb.h"

void print_person(const pb::People::Person& person)
{
  std::cout << "person id    : " << person.id()            << std::endl;
  std::cout << "person name  : " << person.name()          << std::endl;
  std::cout << "person stype : " << person.stype()         << std::endl;
  std::cout << "person email : " << person.email()         << std::endl;
  std::cout << "dog.name     : " << person.dog().name()    << std::endl;
  std::cout << "house.rooms  : " << person.house().rooms() << std::endl;
  std::cout << std::endl;
}

int main(int /*argc*/, char** /*argv*/)
{
  // create a new measurement
  eCAL::measurement::IMeasurement meas(MEASUREMENT_PATH);

  // create a channel (topic name "person")
  eCAL::measurement::IChannel<pb::People::Person> person_channel = meas.Get<pb::People::Person>("person");

  // iterate over the messages
  for (const auto& person_entry : person_channel)
  {
    std::cout << "Person object at timestamp " << person_entry.send_timestamp << std::endl;
    print_person(person_entry.message);
  }

  return 0;
}
