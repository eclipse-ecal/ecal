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
#include <vector>

void do_run(const int runs, int snd_size /*kB*/)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "latency_snd");

  // create publisher and subscriber
  eCAL::CPublisher  pub_pkg("pkg_send");
  eCAL::CSubscriber sub_pkg("pkg_reply");

  // let them match
  eCAL::Process::SleepMS(2000);

  // prepare timestamp array
  std::vector<long long> diff_array(runs);
  std::vector<char> snd_array(snd_size * 1024);
  int snd_pkgs(0);

  // prepare receive buffer
  std::string rec_buf;
  int rec_timeout(1000);

  std::cout << "-------------------------------" << std::endl;
  std::cout << " LATENCY / THROUGHPUT TEST"      << std::endl;
  std::cout << " RUNS  : " << runs               << std::endl;
  std::cout << " SIZE  : " << snd_size <<  " kB" << std::endl;
  std::cout << "-------------------------------" << std::endl;

  for (int reply = 0; reply < runs; ++reply)
  {
    // get time and send message
    long long snd_time = eCAL::Time::GetMicroSeconds();
    pub_pkg.Send(snd_array.data(), snd_array.size(), snd_time);
    // store sent packages
    snd_pkgs++;

    // receive reply with timeout
    if (sub_pkg.Receive(rec_buf, nullptr, rec_timeout))
    {
      // store time stamp
      diff_array[reply] = eCAL::Time::GetMicroSeconds() - snd_time;
    }
    else
    {
      // we lost a package -> stop
      std::cout << "Packages lost after message " << snd_pkgs << " -> stop." << std::endl << std::endl;
      break;
    }
  }

  // calculate roundtrip time over all received messages
  long long sum_time(0);
  for (int reply = 0; reply < snd_pkgs; reply++)
  {
    sum_time += diff_array[reply];
  }
  long long avg_time = sum_time/ snd_pkgs;
  std::cout << "Message average roundtrip time " << avg_time << " us" << std::endl << std::endl;

  // finalize eCAL API
  eCAL::Finalize();
}

int main(int argc, char **argv)
{
  try
  {
    // parse command line
    TCLAP::CmdLine cmd("latency_snd");
    TCLAP::ValueArg<int> runs("r", "runs", "Number of messages to send.", false, 1000, "int");
    TCLAP::ValueArg<int> size("s", "size", "Messages size in kB.",        false, 64,   "int");
    cmd.add(runs);
    cmd.add(size);
    cmd.parse(argc, argv);

    // run test
    do_run(runs.getValue(), size.getValue());
  }
  catch (TCLAP::ArgException &e)  // catch any exceptions
  {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return EXIT_FAILURE;
  }

  return(0);
}
