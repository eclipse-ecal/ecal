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
#include <vector>
#include <atomic>
#include <chrono>
#include <thread>
#include <memory>

#define PAYLOAD_SIZE      1024
#define PRINT_LOG            0
#define PUBLISHER_NUMBER   200

int main()
{
  // initialize eCAL API
  eCAL::Initialize("multiple_snd");

  // create publisher
  struct SPubCount
  {
    SPubCount()
    {
      written = 0;
    }
    long long                         written;
    std::shared_ptr<eCAL::CPublisher> pub;
  };
  typedef std::vector<SPubCount> PubVecT;
  PubVecT pub_vec;
  pub_vec.resize(PUBLISHER_NUMBER);

  // default send string
  std::string send_s = "Hello World ";
  while(send_s.size() < PAYLOAD_SIZE)
  {
    send_s += send_s;
  }
  send_s.resize(PAYLOAD_SIZE);

  for(int i = 0; i < PUBLISHER_NUMBER; i++)
  {
    std::stringstream topic_name;
    topic_name << "PUB_" << i;

    // publisher topic name
    std::shared_ptr<eCAL::CPublisher> pub = std::make_shared<eCAL::CPublisher>(topic_name.str());
    struct SPubCount pub_count;
    pub_count.pub = std::move(pub);
    pub_vec[i] = pub_count;
  }

  // loop counter
  std::atomic<int> global_written(0);

  // safe the start time
  auto start_time = std::chrono::steady_clock::now();

  // send updates
  int cnt(0);
  while(eCAL::Ok())
  {
    cnt++;

    // send dummy topics
    for(int i = 0; i < PUBLISHER_NUMBER; i++)
    {
#if PRINT_LOG
      size_t sent1 = pub_vec[i].pub->Send(send_s, cnt);
      if(sent1 <= 0) std::cout << std::endl << "Sending topic "   << pub_vec[i].pub->GetTopicName() << " failed !" << std::endl;
      else           std::cout << std::endl << "Published topic " << pub_vec[i].pub->GetTopicName() << " with \"" << send_s << "\"" << std::endl;
#else
      // send only
      pub_vec[i].pub->Send(send_s, cnt);
      pub_vec[i].written++;
      global_written++;
#endif
    }

#if PRINT_LOG
    // sleep 10 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
#else
    // check time and print results every second
    std::chrono::duration<double> diff_time = std::chrono::steady_clock::now() - start_time;
    if (diff_time >= std::chrono::seconds(1))
    {
      printf("\n\n");
      start_time = std::chrono::steady_clock::now();
      for(int i = 0; i < PUBLISHER_NUMBER; i++)
      {
        printf("pub %4i: %10i Msg/s\n", i+1, int(pub_vec[i].written/diff_time.count()));
        pub_vec[i].written = 0;
      }
      printf("\n");
      printf("Sum:      %10i  Msg/s\n", int(global_written/diff_time.count()));
      printf("Sum:      %10i kMsg/s\n", int(global_written/1000.0/diff_time.count()));
      printf("Sum:      %10i MMsg/s\n", int(global_written/1000.0/1000.0/diff_time.count()));
      global_written = 0;

      static int oversample = 0;
      oversample++;
      if(oversample == 1)
      {
        oversample = 0;
      }
    }
#endif
  }

  // destroy publisher
  pub_vec.clear();

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
