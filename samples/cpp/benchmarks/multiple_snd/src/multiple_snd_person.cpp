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
#include <ecal/msg/protobuf/publisher.h>

#include <array>
#include <iostream>
#include <sstream>
#include <map>
#include <mutex>
#include <chrono>
#include <thread>
#include <string>

#include <person.pb.h>

#define PUBLISHER_NUMBER 5
const std::array<std::string, PUBLISHER_NUMBER> PersonNames{ "Max", "Marie", "Jana", "Christoph", "Sarah"};

using PersonSendVector_t = std::vector<eCAL::protobuf::CPublisher<pb::People::Person>> ;

void SendAll(PersonSendVector_t& vector)
{
  pb::People::Person person;
  int person_index = 0;
  for (auto& pub : vector)
  {
    person.set_name(PersonNames[person_index]);
    person.set_id(person_index);
    pub.Send(person);
    person_index++;
  }
}

void MultipleSendPerson(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "multiple_snd_vector");

  // create dummy subscriber
  std::vector<eCAL::protobuf::CPublisher<pb::People::Person>> vector_of_publishers;
  std::cout << "Creating publishers ..." << std::endl;
  for(int i = 0; i < PUBLISHER_NUMBER; i++)
  {
    // publisher topic name
    std::stringstream tname;
    tname << "PERSON_" << i;

    // create subscriber
    vector_of_publishers.emplace_back(tname.str());
  }

  int i(0);
  // idle main thread
  while(eCAL::Ok() && i < 100)
  {
    // sleep 100 ms
    SendAll(vector_of_publishers);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    i++;
  }

  std::cout << "Moving publishers ..." << std::endl;
  PersonSendVector_t moved_vector_of_publishers(std::move(vector_of_publishers));
 
  while (eCAL::Ok())
  {
    SendAll(moved_vector_of_publishers);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // finalize eCAL API
  eCAL::Finalize();
}
