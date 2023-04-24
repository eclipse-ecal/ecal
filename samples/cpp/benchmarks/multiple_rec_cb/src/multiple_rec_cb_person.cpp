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
#include <ecal/msg/protobuf/subscriber.h>

#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

#include <person.pb.h>

#define SUBSCRIBER_NUMBER 5

typedef std::map<std::string, size_t> SubMapT;
SubMapT     g_sub_map_person;
std::mutex  g_sub_map_sync_person;

// global subscriber callback
void OnReceivePerson(const char* topic_name_)
{
  // statistics
  {
    std::lock_guard<std::mutex> lock(g_sub_map_sync_person);
    g_sub_map_person[topic_name_]++;
    std::cout << "Received something from " << topic_name_ << std::endl;
  }
}

void MultipleRecPerson(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "multiple_rec_cb");

  // create dummy subscriber
  std::vector<eCAL::protobuf::CSubscriber<pb::People::Person>> vector_of_subscribers;
  std::cout << "create subscribers .." << std::endl;
  for(int i = 0; i < SUBSCRIBER_NUMBER; i++)
  {
    // publisher topic name
    std::stringstream tname;
    tname << "PERSON_" << i;

    // create subscriber
    vector_of_subscribers.emplace_back(tname.str());

    // setup receive callback
    auto callback = std::bind(OnReceivePerson, std::placeholders::_1);
    vector_of_subscribers.back().AddReceiveCallback(callback);
  }

  // idle main thread
  while(eCAL::Ok())
  {
    // sleep 100 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // finalize eCAL API
  eCAL::Finalize();
}
