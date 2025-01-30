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
#include <ecal/pubsub/subscriber.h>

#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

// print performance results
void PrintStatistic(const std::string& topic_name_, const std::chrono::duration<double>& diff_time_, const size_t size_, long long& bytes_, long long& msgs_, const struct eCAL::SReceiveCallbackData& data_)
{
    std::stringstream out;
    out << "Topic Name:            " << topic_name_                                                      << std::endl;
    if (data_.buffer_size > 15)
    {
      out << "Message [0 - 15]:      ";
      for (auto i = 0; i < 16; ++i) out << (static_cast<const char*>(data_.buffer))[i] << " ";
      out << std::endl;
    }
    out << "Message size (kByte):  " << (unsigned int)(size_  / 1024.0)                                        << std::endl;
    out << "kByte/s:               " << (unsigned int)(bytes_ / 1024.0 /                   diff_time_.count()) << std::endl;
    out << "MByte/s:               " << (unsigned int)(bytes_ / 1024.0 / 1024.0 /          diff_time_.count()) << std::endl;
    out << "GByte/s:               " << (unsigned int)(bytes_ / 1024.0 / 1024.0 / 1024.0 / diff_time_.count()) << std::endl;
    out << "Messages/s:            " << (unsigned int)(msgs_  /                            diff_time_.count()) << std::endl;
    out << "Latency (us):          " << (diff_time_.count() * 1e6) / (double)msgs_                             << std::endl;
    std::cout << out.str() << std::endl;
    msgs_  = 0;
    bytes_ = 0;
}

// main entry
int main()
{
  // initialize eCAL API
  eCAL::Initialize("performance_rec");

  // create subscriber for topic "Performance"
  eCAL::CSubscriber sub("Performance");

  // helper variables for time and throughput
  std::chrono::steady_clock::time_point start_time(std::chrono::nanoseconds(0));
  long long msgs (0);
  long long bytes(0);

  // add callback
    auto on_receive = [&](const eCAL::STopicId& topic_id_, const eCAL::SReceiveCallbackData & data_) {
    auto size = data_.buffer_size;

    msgs++;
    bytes += size;

    // check time and print results every second
    const std::chrono::duration<double> diff_time = std::chrono::steady_clock::now() - start_time;
    if (diff_time >= std::chrono::seconds(1))
    {
      PrintStatistic(topic_id_.topic_name, diff_time, size, bytes, msgs, data_);
      start_time = std::chrono::steady_clock::now();
    }
  };
  sub.SetReceiveCallback(std::bind(on_receive, std::placeholders::_1, std::placeholders::_3));

  // idle main thread
  while(eCAL::Ok())
  {
    // sleep 100 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
