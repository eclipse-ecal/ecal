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

#include <iostream>
#include <sstream>
#include <map>
#include <mutex>
#include <chrono>
#include <thread>

#define SUBSCRIBER_NUMBER  200

// globals
struct SSubCount
{
  SSubCount()
  {
    read = 0;
    sub  = nullptr;
  }
  int                read;
  eCAL::CSubscriber* sub;
};
typedef std::map<std::string, SSubCount> SubMapT;
SubMapT          g_sub_map;
std::mutex       g_sub_map_sync;

int              g_overalll_read = 0;
std::chrono::steady_clock::time_point start_time(std::chrono::nanoseconds(0));

// global subscriber callback
void OnReceive(const eCAL::STopicId& topic_id_)
{
  // statistics
  {
    std::lock_guard<std::mutex> lock(g_sub_map_sync);
    g_sub_map[topic_id_.topic_name].read++;
  }
  g_overalll_read++;
}

int main()
{
  // initialize eCAL API
  eCAL::Initialize("multiple_rec_cb");

  // create dummy subscriber
  std::cout << "create subscribers .." << std::endl;
  for(int i = 0; i < SUBSCRIBER_NUMBER; i++)
  {
    // publisher topic name
    std::stringstream topic_name;
    topic_name << "PUB_" << i;

    // create subscriber
    eCAL::CSubscriber* sub = new eCAL::CSubscriber(topic_name.str());
    struct SSubCount sub_count;
    sub_count.sub = sub;

    // add subscriber
    {
      std::lock_guard<std::mutex> lock(g_sub_map_sync);
      g_sub_map[topic_name.str()] = sub_count;
    }

    // setup receive callback
    auto callback = std::bind(OnReceive, std::placeholders::_1);
    sub->SetReceiveCallback(callback);
  }

  // safe the start time
  start_time = std::chrono::steady_clock::now();

  // idle main thread
  while(eCAL::Ok())
  {
    // sleep 100 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // check time and print results every second
    std::chrono::duration<double> diff_time = std::chrono::steady_clock::now() - start_time;
    if (diff_time >= std::chrono::seconds(1))
    {
      printf("\n\n");
      start_time = std::chrono::steady_clock::now();
      int i = 0;
      // print map
      {
        for(SubMapT::iterator iter = g_sub_map.begin(); iter != g_sub_map.end(); ++iter)
        {
          printf("sub %4i: %10i Msg/s\n", i+1, int(iter->second.read/diff_time.count()));
          iter->second.read = 0;
          i++;
        }
      }
      printf("\n");
      printf("Sum:      %10i  Msg/s\n", int(g_overalll_read/diff_time.count()));
      printf("Sum:      %10i kMsg/s\n", int(g_overalll_read/1000.0/diff_time.count()));
      printf("Sum:      %10i MMsg/s\n", int(g_overalll_read/1000.0/1000.0/diff_time.count()));
      g_overalll_read = 0;
    }
  }

  // destroy and delete dummy topics
  std::cout << "start destruction sequence .." << std::endl;
  {
    std::lock_guard<std::mutex> lock(g_sub_map_sync);
    for(SubMapT::iterator iter = g_sub_map.begin(); iter != g_sub_map.end(); ++iter)
    {
      delete iter->second.sub;
    }
    g_sub_map.clear();
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
