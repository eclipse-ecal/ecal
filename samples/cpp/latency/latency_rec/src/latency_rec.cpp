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

#include <chrono>
#include <iostream>
#include <mutex>
#include <numeric>
#include <vector>

// data structure for later evaluation
struct SCallbackPar
{
  SCallbackPar() { latency_array.reserve(100000); };
  std::mutex             mtx;
  std::vector<long long> latency_array;
  size_t                 rec_size = 0;
  size_t                 msg_num  = 0;
};

// time getter
long long get_microseconds()
{
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  return(std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count());
}

// message receive callback
void on_receive(const struct eCAL::SReceiveCallbackData* data_, SCallbackPar* par_)
{
  // get receive time stamp
  auto rec_time = get_microseconds();

  // update latency, size and msg number
  std::lock_guard<std::mutex> lock(par_->mtx);
  par_->latency_array.push_back(rec_time - data_->time);
  par_->rec_size = data_->size;
  par_->msg_num++;
}

// single test run
void do_run()
{
  // subscriber
  eCAL::CSubscriber sub("ping");

  // apply subscriber callback function
  SCallbackPar cb_par;
  auto callback = std::bind(on_receive, std::placeholders::_2, &cb_par);
  sub.AddReceiveCallback(callback);

  size_t msg_last(0);
  while (eCAL::Ok())
  {
    // check once a second if we still receive new messages
    // if not, we stop and evaluate this run
    {
      std::lock_guard<std::mutex> lock(cb_par.mtx);
      if ((cb_par.msg_num > 0) && (msg_last == cb_par.msg_num)) break;
      else msg_last = cb_par.msg_num;
    }
    eCAL::Process::SleepMS(1000);
  }

  // detach callback
  sub.RemReceiveCallback();

  // evaluate all
  long long sum_msg = cb_par.latency_array.size();
  std::cout << "--------------------------------------------" << std::endl;
  std::cout << "Messages received             : " << sum_msg  << std::endl;
  if (sum_msg > 0)
  {
    long long sum_time = std::accumulate(cb_par.latency_array.begin(), cb_par.latency_array.end(), 0LL);
    long long avg_time = sum_time / sum_msg;
    std::cout << "Message size received         : " << cb_par.rec_size/1024 << " kB" << std::endl;
    std::cout << "Message average latency       : " << avg_time             << " us" << std::endl;
    std::cout << "Throughput                    : " << static_cast<int>(((cb_par.rec_size * sum_msg) / 1024.0) / (sum_time / 1000.0 / 1000.0))          << " kB/s"  << std::endl;
    std::cout << "                              : " << static_cast<int>(((cb_par.rec_size * sum_msg) / 1024.0 / 1024.0) / (sum_time / 1000.0 / 1000.0)) << " MB/s"  << std::endl;
    std::cout << "                              : " << static_cast<int>(sum_msg / (sum_time / 1000.0 / 1000.0))                                         << " Msg/s" << std::endl;
  }
  std::cout << "--------------------------------------------" << std::endl;
}

int main(int /*argc*/, char** /*argv*/)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "latency_rec");

  while (eCAL::Ok())
  {
    // run test
    do_run();
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
