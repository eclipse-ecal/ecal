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

// time getter
long long get_microseconds()
{
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  return(std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count());
}

// single test run
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
    pub.Send(snd_array.data(), snd_array.size(), get_microseconds());
    // delay
    eCAL::Process::SleepMS(delay);
  }

  // log test
  std::cout << "Messages sent           : " << run               << std::endl;
  std::cout << "--------------------------------------------"    << std::endl;

  // finalize eCAL API
  eCAL::Finalize();
}

int main(int argc, char **argv)
{
  try
  {
    // parse command line
    TCLAP::CmdLine cmd("latency_snd");
    TCLAP::ValueArg<int> runs     ("r", "runs",      "Number of messages to send.", false, 1000, "int");
    TCLAP::ValueArg<int> size     ("s", "size",      "Messages size in kB.",        false, 64,   "int");
    TCLAP::ValueArg<int> delay    ("d", "delay",     "Messages send delay in ms.",  false, 10,   "int");
    TCLAP::SwitchArg     zero_copy("z", "zero_copy", "Switch zero copy mode on.");
    cmd.add(runs);
    cmd.add(size);
    cmd.add(delay);
    cmd.add(zero_copy);
    cmd.parse(argc, argv);

    if(size < 0)
    {
      // automatic size mode
      for (int s = 1; s <= 16384; s *= 2) do_run(runs.getValue(), s, delay.getValue(), zero_copy.getValue());
    }
    else
    {
      // run single test
      do_run(runs.getValue(), size.getValue(), delay.getValue(), zero_copy.getValue());
    }
  }
  catch (TCLAP::ArgException &e)  // catch any exceptions
  {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return EXIT_FAILURE;
  }

  return(0);
}
