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

#include <tclap/CmdLine.h>
#include <iostream>
#include <string>

void do_run(const int runs, int snd_size /*kB*/, int delay /*ms*/, bool zero_copy)
{
  // log parameter
  std::cout << "--------------------------------------------"    << std::endl;
  std::cout << "Runs                    : " << runs              << std::endl;
  std::cout << "Message size            : " << snd_size << " kB" << std::endl;
  std::cout << "Message delay           : " << delay    << " ms" << std::endl;
  if (zero_copy)
  {
    std::cout << "Zero copy               : ON"  << std::endl;
  }
  else
  {
    std::cout << "Zero copy               : OFF" << std::endl;
  }

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "latency_snd");

  // create publisher and subscriber
  eCAL::CPublisher pub("ping");

  // enable zero copy mode
  pub.EnableZeroCopy(zero_copy);

  // prepare send buffer
  std::vector<char> snd_array(snd_size * 1024);

  // let them match
  eCAL::Process::SleepMS(2000);

  int run(0);
  for (run = 0; run < runs; ++run)
  {
    // get time and send message
    pub.Send(snd_array.data(), snd_array.size(), eCAL::Time::GetMicroSeconds());
    // delay
    eCAL::Process::SleepMS(delay);
  }

  // log test
  std::cout << "Messages sent           : " << run               << std::endl;
  std::cout << "--------------------------------------------"    << std::endl;

  // finalize eCAL API
  eCAL::Finalize();
}

int main(int /*argc*/, char** /*argv*/)
{
  int runs (200); // number of cylces
  int delay(10);  // delay between two publications in ms
  int size (0);   // message payload size

  // test for message size from 1 kB to 16 MB (zero copy off)
  for (size = 1; size <= 16384; size *= 2) do_run(runs, size, delay, false);

  // test for message size from 1 kB to 16 MB (zero copy on)
  for (size = 1; size <= 16384; size *= 2) do_run(runs, size, delay, true);

  return(0);
}
