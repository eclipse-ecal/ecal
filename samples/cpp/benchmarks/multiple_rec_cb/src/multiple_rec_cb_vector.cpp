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

#include <iostream>
#include <sstream>
#include <map>
#include <mutex>
#include <chrono>
#include <thread>

#define SUBSCRIBER_NUMBER 10

typedef std::map<std::string, size_t> SubMapT;
SubMapT     g_sub_map_vector;
std::mutex  g_sub_map_sync_vector;

// global subscriber callback
void OnReceiveVector(const char* topic_name_)
{
  // statistics
  {
    std::lock_guard<std::mutex> lock(g_sub_map_sync_vector);
    g_sub_map_vector[topic_name_]++;
    std::cout << "Received something from " << topic_name_ << std::endl;
  }
}

void MultipleRecVector(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "multiple_rec_cb");

  // create dummy subscriber
  std::vector<eCAL::CSubscriber> vector_of_subscribers;
  std::cout << "create subscribers .." << std::endl;
  for(int i = 0; i < SUBSCRIBER_NUMBER; i++)
  {
    // publisher topic name
    std::stringstream tname;
    tname << "PUB_" << i;

    // create subscriber
    vector_of_subscribers.emplace_back(tname.str());

    // setup receive callback
    auto callback = std::bind(OnReceiveVector, std::placeholders::_1);
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
