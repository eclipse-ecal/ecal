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

#include <ecal/msg/protobuf/omeasurement.h>

#include <iostream>

#include "person.pb.h"

constexpr auto ONE_SECOND = 1000000;

int main(int /*argc*/, char** /*argv*/)
{
  // create a new measurement
  eCAL::measurement::OMeasurement meas(".");

  // create a channel (topic name "person")
  eCAL::protobuf::OChannel<pb::People::Person> person_channel = eCAL::measurement::CreateChannel<eCAL::protobuf::OChannel<pb::People::Person>>(meas, "person");

  pb::People::Person person;
  person.set_name("Max");

  long long timestamp = 0;

  for (int i = 0; i< 100; i++)
  {
    person.set_id(i);

    person_channel << eCAL::measurement::make_frame(person, timestamp);
    timestamp += ONE_SECOND;
  }

  return 0;
}
