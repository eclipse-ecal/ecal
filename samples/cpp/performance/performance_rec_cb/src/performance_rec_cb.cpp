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
#include <ecal/msg/string/subscriber.h>

#include <iostream>
#include <sstream>
#include <mutex>
#include <chrono>
#include <thread>

// globals
std::chrono::steady_clock::time_point start_time(std::chrono::nanoseconds(0));
long long                             g_msgs (0);
long long                             g_bytes(0);

// print performance results
void PrintStatistic(const std::string& topic_name_, const std::chrono::duration<double>& diff_time_, const size_t size_, long long& bytes_, long long& msgs_)
{
    std::stringstream out;
    out << "Topic Name:            " << topic_name_                                    << std::endl;
    out << "Message size (kByte):  " << int(size_  / 1024                            ) << std::endl;
    out << "kByte/s:               " << int(bytes_ / 1024        / diff_time_.count()) << std::endl;
    out << "MByte/s:               " << int(bytes_ / 1024 / 1024 / diff_time_.count()) << std::endl;
    out << "Messages/s:            " << int(msgs_                / diff_time_.count()) << std::endl;
    std::cout << out.str() << std::endl;
    msgs_  = 0;
    bytes_ = 0;
}

// subscriber callback function
void OnReceive(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_)
{
  size_t size = data_->size;

  g_msgs++;
  g_bytes += size;

  // block it 10 ms, so we emulate some workload in the callback
  // std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // check time and print results every second
  std::chrono::duration<double> diff_time = std::chrono::steady_clock::now() - start_time;
  if (diff_time >= std::chrono::seconds(1))
  {
    PrintStatistic(topic_name_, diff_time, size, g_bytes, g_msgs);
    start_time = std::chrono::steady_clock::now();
  }
}

// main entry
int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "performance_rec_cb");

  // create subscriber for topic "Performance"
  eCAL::CSubscriber sub("Performance");

  // dump instance state if creation failed
  if(!sub.IsCreated())
  {
    std::cout << "Could not create subscriber !" << std::endl;
    return(0);
  }

  // add callback
  sub.AddReceiveCallback(std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2));

  // idle main thread
  while(eCAL::Ok())
  {
    // sleep 100 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // destroy subscriber
  sub.Destroy();

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
