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

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <tclap/CmdLine.h>

#include "latency_log.h"

// warmup runs not to measure
const int warmups(100);

// data structure for later evaluation
struct SCallbackPar
{
  SCallbackPar() { latency_array.reserve(100000); };
  std::mutex             mtx;
  std::vector<long long> latency_array;
  size_t                 rec_size = 0;
  size_t                 msg_num  = 0;
};

// message receive callback
void on_receive(const struct eCAL::SReceiveCallbackData& data_, SCallbackPar* par_, int delay_)
{
  // get receive time stamp
  auto rec_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

  // update latency, size and msg number
  const std::lock_guard<std::mutex> lock(par_->mtx);
  par_->latency_array.push_back(rec_time - data_.send_timestamp);
  par_->rec_size = data_.buffer_size;
  par_->msg_num++;
  // delay callback
  if(delay_ > 0) std::this_thread::sleep_for(std::chrono::milliseconds(delay_));
}

// single test run
void do_run(int delay_, std::string& log_file_)
{
  // initialize eCAL API
  eCAL::Initialize("latency_rec");

  // subscriber
  eCAL::CSubscriber sub("ping");

  // apply subscriber callback function
  SCallbackPar cb_par;
  auto callback = std::bind(on_receive, std::placeholders::_3, &cb_par, delay_);
  sub.SetReceiveCallback(callback);

  size_t msg_last(0);
  while (eCAL::Ok())
  {
    // check once a second if we still receive new messages
    // if not, we stop and evaluate this run
    {
      const std::lock_guard<std::mutex> lock(cb_par.mtx);
      if ((cb_par.msg_num > 0) && (msg_last == cb_par.msg_num)) break;
      else msg_last = cb_par.msg_num;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  // detach callback
  sub.RemoveReceiveCallback();

  // evaluate all
  evaluate(cb_par.latency_array, cb_par.rec_size, warmups, log_file_);

  // log all latencies into file
  log2file(cb_par.latency_array, cb_par.rec_size, log_file_);

  // finalize eCAL API
  eCAL::Finalize();
}

int main(int argc, char** argv)
{
  try
  {
    // parse command line
    TCLAP::CmdLine cmd("latency_rec");
    TCLAP::ValueArg<int>         delay(   "d", "delay",    "Callback process delay in ms.",     false,  0, "int");
    TCLAP::ValueArg<std::string> log_file("l", "log_file", "Base file name to export results.", false, "", "string");
    cmd.add(delay);
    cmd.add(log_file);
    cmd.parse(argc, argv);

    // run tests
    while(eCAL::Ok())
    {
      do_run(delay.getValue(), log_file.getValue());
    }
  }
  catch (TCLAP::ArgException& e)  // catch any exceptions
  {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return EXIT_FAILURE;
  }

  return(0);
}
