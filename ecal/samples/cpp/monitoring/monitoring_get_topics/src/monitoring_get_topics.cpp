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

#include <chrono>
#include <iostream>
#include <string>
#include <set>
#include <thread>

int main()
{
  int                                   run(0), runs(10);
  std::chrono::steady_clock::time_point start_time;

  // initialize eCAL core API
  eCAL::Initialize("monitoring get topics");

  // monitor for ever
  while(eCAL::Ok())
  {
    // GetTopics
    {
      std::set<eCAL::STopicId> topic_id_pub_set;
      std::set<eCAL::STopicId> topic_id_sub_set;

      start_time = std::chrono::steady_clock::now();
      for (run = 0; run < runs; ++run)
      {
        eCAL::Registration::GetPublisherIDs(topic_id_pub_set);
        eCAL::Registration::GetSubscriberIDs(topic_id_pub_set);
      }

      auto num_entities = topic_id_pub_set.size() + topic_id_sub_set.size();
      auto diff_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time);
      std::cout << "GetTopics      : " << static_cast<double>(diff_time.count()) / runs << " ms" << " (" << num_entities << " entities)" << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
