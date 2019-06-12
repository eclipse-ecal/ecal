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

void do_run(int runs)
{
  eCAL::CSubscriber sub_pkg("pkg_send");
  eCAL::CPublisher  pub_pkg("pkg_reply");

  std::cout << "-------------------------------" << std::endl;
  std::cout << " LATENCY / THROUGHPUT TEST"      << std::endl;
  std::cout << "-------------------------------" << std::endl;

  std::vector<long long> diff_array;
  diff_array.resize(runs);

  std::string rec_buf;
  rec_buf.reserve(4 * 1024 * 1024);

  for (int reply = 0; reply < runs; reply++)
  {
    if (sub_pkg.Receive(rec_buf, nullptr, -1))
    {
      diff_array[reply] = eCAL::Time::GetMicroSeconds() - *((long long*)rec_buf.c_str());
      pub_pkg.Send(rec_buf);
    }
  }

  long long diff_time = 0;
  for (int reply = 0; reply < runs; reply++)
  {
    diff_time += diff_array[reply];
  }
  long long avg_time = diff_time/runs;
  std::cout << "Rec buffer size               :   " << rec_buf.size()/1024 << " kB" << std::endl;
  std::cout << "Message average receive time  :   " << avg_time << " us" << std::endl;
  std::cout << "Throughput                    :   " << static_cast<int>(((rec_buf.size()*runs)/1024.0)/(diff_time/1000.0/1000.0))        << " kB/s"  << std::endl;
  std::cout << "                              :   " << static_cast<int>(((rec_buf.size()*runs)/1024.0/1024.0)/(diff_time/1000.0/1000.0)) << " MB/s"  << std::endl;
  std::cout << "                              :   " << static_cast<int>(runs/(diff_time/1000.0/1000.0))                                  << " Msg/s" << std::endl << std::endl;
}

int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "latency_reply");

  // run tests
  while (eCAL::Ok())
  {
    do_run(1000);
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
