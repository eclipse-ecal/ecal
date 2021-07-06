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
#include <mutex>
#include <numeric>
#include <string>
#include <vector>

struct SCallbackPar
{
  SCallbackPar() { pub_pkg.Create("pkg_reply");  diff_array.reserve(100000); };
  std::mutex             mtx;
  eCAL::CPublisher       pub_pkg;
  std::vector<long long> diff_array;
  size_t                 msg_num  = 0;
  size_t                 rec_size = 0;
};

void on_receive(const struct eCAL::SReceiveCallbackData* data_, SCallbackPar* par_)
{
  // store time stamp
  par_->diff_array.push_back(eCAL::Time::GetMicroSeconds() - data_->time);
  // update size and msg number
  par_->rec_size = data_->size;
  {
    std::lock_guard<std::mutex> lock(par_->mtx);
    par_->msg_num++;
  }
  // reply
  par_->pub_pkg.Send(data_->buf, data_->size);
}

void do_run()
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "latency_rec_cb");

  // subscriber
  eCAL::CSubscriber sub_pkg("pkg_send");

  // apply subscriber callback function
  SCallbackPar cb_par;
  auto callback = std::bind(on_receive, std::placeholders::_2, &cb_par);
  sub_pkg.AddReceiveCallback(callback);

  size_t msg_last(0);
  while (eCAL::Ok())
  {
    {
      std::lock_guard<std::mutex> lock(cb_par.mtx);
      if ((cb_par.msg_num > 0) && (msg_last == cb_par.msg_num)) break;
      else msg_last = cb_par.msg_num;
    }
    eCAL::Process::SleepMS(1000);
  }
  // detach callback
  sub_pkg.RemReceiveCallback();

  // finalize eCAL API
  eCAL::Finalize();

  // calculate receive time over all received messages
  long long sum_time = std::accumulate(cb_par.diff_array.begin(), cb_par.diff_array.end(), 0LL);
  long long avg_time = sum_time / cb_par.msg_num;
  std::cout << "-------------------------------"    << std::endl;
  std::cout << " LATENCY / THROUGHPUT TEST     "    << std::endl;
  std::cout << "-------------------------------"    << std::endl;
  std::cout << "Received buffer size          :   " << cb_par.rec_size / 1024 << " kB" << std::endl;
  std::cout << "Received packages             :   " << cb_par.msg_num << std::endl;
  std::cout << "Message average receive time  :   " << avg_time << " us" << std::endl;
  std::cout << "Throughput                    :   " << static_cast<int>(((cb_par.rec_size*cb_par.msg_num) / 1024.0) / (sum_time / 1000.0 / 1000.0)) << " kB/s" << std::endl;
  std::cout << "                              :   " << static_cast<int>(((cb_par.rec_size*cb_par.msg_num) / 1024.0 / 1024.0) / (sum_time / 1000.0 / 1000.0)) << " MB/s" << std::endl;
  std::cout << "                              :   " << static_cast<int>(cb_par.msg_num / (sum_time / 1000.0 / 1000.0)) << " Msg/s" << std::endl << std::endl;
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
