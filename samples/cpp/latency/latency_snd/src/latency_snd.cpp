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

void do_run(const int runs, int snd_size /*kB*/)
{
  eCAL::CPublisher  pub_pkg("pkg_send");
  eCAL::CSubscriber sub_pkg("pkg_reply");

  // let them match
  eCAL::Process::SleepMS(1000);

  std::vector<long long> diff_array;
  diff_array.resize(runs);
  std::vector<char> snd_array;
  snd_array.resize(snd_size * 1024);

  std::string rec_buf;
  rec_buf.reserve(snd_size * 1024);

  std::cout << "Press Enter to start .." << std::endl;
  getchar();

  std::cout << "-------------------------------" << std::endl;
  std::cout << " LATENCY / THROUGHPUT TEST"      << std::endl;
  std::cout << " RUNS  : " << runs               << std::endl;
  std::cout << " SIZE  : " << snd_size <<  " kB" << std::endl;
  std::cout << "-------------------------------" << std::endl;

  for (int reply = 0; reply < runs; reply++)
  {
    long long start_time_loop = eCAL::Time::GetMicroSeconds();
    memcpy(snd_array.data(), &start_time_loop, sizeof(long long));
    pub_pkg.Send(snd_array.data(), snd_array.size());
    if (sub_pkg.Receive(rec_buf, nullptr, 100))
    {
      diff_array[reply] = eCAL::Time::GetMicroSeconds() - start_time_loop;
    }
  }

  long long diff_time = 0;
  for (int reply = 0; reply < runs; reply++)
  {
    diff_time += diff_array[reply];
  }
  long long avg_time = diff_time / runs;
  std::cout << "Message average roundtrip time " << avg_time << " us" << std::endl << std::endl;
}

int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "latency_snd");

  // run tests
  for (int size = 1; size <= 4096; size *= 2)
  {
    do_run(1000, size);
  }

  // finalize eCAL API
  eCAL::Finalize();

  printf("Press Enter to close ..\n");
  getchar();

  return(0);
}
