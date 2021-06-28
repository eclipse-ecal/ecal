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
#include <thread>

int main(int argc, char **argv)
{
  long long msgs  = 0;
  long long bytes = 0;
  long long slen  = 0;

  // initialize eCAL API
  eCAL::Initialize(argc, argv, "performance_rec");

  // create subscriber for topic "Performance"
  eCAL::CSubscriber sub("Performance", "base:std::string");

  // dump instance state if creation failed
  if(!sub.IsCreated())
  {
    std::cout << "Could not create subscriber !" << std::endl;
    return(0);
  }

  // safe the start time
  auto start_time(std::chrono::steady_clock::now());

  // receive updates
  std::string rcv_buf;
  while(eCAL::Ok())
  {
    // receive content with infinite timeout
    size_t rcv_len = sub.Receive(rcv_buf, nullptr, -1);
    // collect data
    if(rcv_len > 0)
    {
      msgs++;
      slen = rcv_len;
      bytes += rcv_len;
    }

    // check time and print results every second
    std::chrono::duration<double> diff_time = std::chrono::steady_clock::now() - start_time;
    if (diff_time >= std::chrono::seconds(1))
    {
      start_time = std::chrono::steady_clock::now();
      std::stringstream out;
      out << "Message size (kByte):  " << int(slen / 1024)                             << std::endl;
      out << "kByte/s:               " << int(bytes / 1024 / diff_time.count())        << std::endl;
      out << "MByte/s:               " << int(bytes / 1024 / 1024 / diff_time.count()) << std::endl;
      out << "Messages/s:            " << int(msgs / diff_time.count())                << std::endl;
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
