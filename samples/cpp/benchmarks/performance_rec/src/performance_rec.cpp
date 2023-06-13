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
#include <chrono>

int main(int argc, char **argv)
{
  long long msgs (0);
  long long bytes(0);

  // initialize eCAL API
  eCAL::Initialize(argc, argv, "performance_rec");

  // create subscriber for topic "Performance"
  eCAL::STopicInformation topic_info;
  topic_info.encoding = "base";
  topic_info.type = "std::string";
  eCAL::CSubscriber sub("Performance", topic_info);

  // safe the start time
  auto start_time(std::chrono::steady_clock::now());

  // receive updates
  std::string rcv_buf;
  while(eCAL::Ok())
  {
    // receive content with infinite timeout
    const bool success = sub.ReceiveBuffer(rcv_buf, nullptr, -1);
    // collect data
    if(success)
    {
      msgs++;
      bytes += rcv_buf.size();
    }

    // check time and print results every second
    const std::chrono::duration<double> diff_time = std::chrono::steady_clock::now() - start_time;
    if (diff_time >= std::chrono::seconds(1))
    {
      start_time = std::chrono::steady_clock::now();
      std::stringstream out;
      if (rcv_buf.size() > 15)
      {
        out << "Message [0 - 15]:      ";
        for (auto i = 0; i < 16; ++i) out << rcv_buf[i] << " ";
        out << std::endl;
      }
      out << "Message size (kByte):  " << (unsigned int)(rcv_buf.size()  / 1024.0)                             << std::endl;
      out << "kByte/s:               " << (unsigned int)(bytes / 1024.0 /                   diff_time.count()) << std::endl;
      out << "MByte/s:               " << (unsigned int)(bytes / 1024.0 / 1024.0 /          diff_time.count()) << std::endl;
      out << "GByte/s:               " << (unsigned int)(bytes / 1024.0 / 1024.0 / 1024.0 / diff_time.count()) << std::endl;
      out << "Messages/s:            " << (unsigned int)(msgs  /                            diff_time.count()) << std::endl;
      out << "Latency (us):          " << (diff_time.count() * 1e6) / (double)msgs                             << std::endl;
      std::cout << out.str() << std::endl;
      msgs  = 0;
      bytes = 0;
    }
  }

  // destroy subscriber
  sub.Destroy();

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
