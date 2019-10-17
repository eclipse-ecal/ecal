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
#include <numeric>
#include <string>
#include <vector>

void do_run()
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "latency_rec");

  // create publisher and subscriber
  eCAL::CSubscriber sub_pkg("pkg_send");
  eCAL::CPublisher  pub_pkg("pkg_reply");

  // prepare timestamp array and
  std::vector<long long> diff_array;
  // reserve for 5000 elements
  diff_array.reserve(5000);

  // prepare receive buffer
  std::string rec_buf;
  int rec_timeout(-1);
  int rec_pkgs(0);

  for(;;)
  {
    long long snd_time(0);
    if (sub_pkg.Receive(rec_buf, &snd_time, rec_timeout))
    {
      // store time stamp
      diff_array.push_back(eCAL::Time::GetMicroSeconds() - snd_time);
      rec_pkgs++;
      // reply
      pub_pkg.Send(rec_buf);
      // we reduce timeout to 1000 ms to detect lost packages
      rec_timeout = 1000;
    }
    else
    {
      // timeout, let's stop and summarize
      break;
    }
  }

  // calculate receive time over all received messages
  long long sum_time = std::accumulate(diff_array.begin(), diff_array.end(), 0LL);
  long long avg_time = sum_time/rec_pkgs;
  std::cout << "-------------------------------"    << std::endl;
  std::cout << " LATENCY / THROUGHPUT TEST     "    << std::endl;
  std::cout << "-------------------------------"    << std::endl;
  std::cout << "Received buffer size          :   " << rec_buf.size()/1024 << " kB" << std::endl;
  std::cout << "Received packages             :   " << rec_pkgs << std::endl;
  std::cout << "Message average receive time  :   " << avg_time << " us" << std::endl;
  std::cout << "Throughput                    :   " << static_cast<int>(((rec_buf.size()*rec_pkgs)/1024.0)/(sum_time /1000.0/1000.0))        << " kB/s"  << std::endl;
  std::cout << "                              :   " << static_cast<int>(((rec_buf.size()*rec_pkgs)/1024.0/1024.0)/(sum_time /1000.0/1000.0)) << " MB/s"  << std::endl;
  std::cout << "                              :   " << static_cast<int>(rec_pkgs /(sum_time /1000.0/1000.0))                                 << " Msg/s" << std::endl << std::endl;

  // finalize eCAL API
  eCAL::Finalize();
}

int main(int /*argc*/, char** /*argv*/)
{
  // run tests
  while (eCAL::Ok())
  {
    do_run();
  }

  return(0);
}
