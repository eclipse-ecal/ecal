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

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>
#include <mutex>
#include <numeric>
#include <vector>

#include <tclap/CmdLine.h>

// warmup runs not to measure
const int warmups(10);

// helper
long long get_microseconds();
void evaluate(std::vector<long long>& lat_arr_, size_t rec_size_, size_t warmups_, std::string& log_file_);
void log2file(std::vector<long long>& lat_arr_, size_t rec_size_, std::string& log_file_);

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
void on_receive(const struct eCAL::SReceiveCallbackData* data_, SCallbackPar* par_, int delay_, std::string& log_file_)
{
  // get receive time stamp
  auto rec_time = get_microseconds();

  // update latency, size and msg number
  std::lock_guard<std::mutex> lock(par_->mtx);
  par_->latency_array.push_back(rec_time - data_->time);
  par_->rec_size = data_->size;
  par_->msg_num++;

  if(delay_ > 0) eCAL::Process::SleepMS(delay_);
}

// single test run
void do_run(int delay_, std::string& log_file_)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "latency_rec");

  // subscriber
  eCAL::CSubscriber sub("ping");

  // apply subscriber callback function
  SCallbackPar cb_par;
  auto callback = std::bind(on_receive, std::placeholders::_2, &cb_par, delay_, log_file_);
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
    TCLAP::ValueArg<int>         delay(   "d", "delay",    "Messages send delay in ms.",    false,  0, "int");
    TCLAP::ValueArg<std::string> log_file("l", "log_file", "File to export latency array.", false, "", "string");
    cmd.add(delay);
    cmd.add(log_file);
    cmd.parse(argc, argv);

    // run tests
    do { do_run(delay.getValue(), log_file.getValue()); } while (eCAL::Ok());
  }
  catch (TCLAP::ArgException& e)  // catch any exceptions
  {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return EXIT_FAILURE;
  }

  return(0);
}

// time getter
long long get_microseconds()
{
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  return(std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count());
}

// evaluation
void evaluate(std::vector<long long>& lat_arr_, size_t rec_size_, size_t warmups_, std::string& log_file_)
{
  std::stringstream ss;

  // remove warmup runs
  if (lat_arr_.size() >= warmups_)
  {
    lat_arr_.erase(lat_arr_.begin(), lat_arr_.begin() + warmups_);
  }

  // evaluate all
  size_t sum_msg = lat_arr_.size();
  ss << "--------------------------------------------" << std::endl;
  ss << "Messages received             : " << sum_msg  << std::endl;
  if (sum_msg > warmups_)
  {
    long long sum_time = std::accumulate(lat_arr_.begin(), lat_arr_.end(), 0LL);
    long long avg_time = sum_time / sum_msg;
    auto      min_it = std::min_element(lat_arr_.begin(), lat_arr_.end());
    auto      max_it = std::max_element(lat_arr_.begin(), lat_arr_.end());
    size_t    min_pos = min_it - lat_arr_.begin();
    size_t    max_pos = max_it - lat_arr_.begin();
    long long min_time = *min_it;
    long long max_time = *max_it;
    ss << "Message size received         : " << rec_size_ / 1024 << " kB"       << std::endl;
    ss << "Message average latency       : " << avg_time << " us"               << std::endl;
    ss << "Message min latency           : " << min_time << " us @ " << min_pos << std::endl;
    ss << "Message max latency           : " << max_time << " us @ " << max_pos << std::endl;
    ss << "Throughput                    : " << static_cast<int>(((rec_size_ * sum_msg) / 1024.0) / (sum_time / 1000.0 / 1000.0))          << " kB/s"  << std::endl;
    ss << "                              : " << static_cast<int>(((rec_size_ * sum_msg) / 1024.0 / 1024.0) / (sum_time / 1000.0 / 1000.0)) << " MB/s"  << std::endl;
    ss << "                              : " << static_cast<int>(sum_msg / (sum_time / 1000.0 / 1000.0))                                   << " Msg/s" << std::endl;
  }
  ss << "--------------------------------------------" << std::endl;

  // log to console
  std::cout << ss.str();

  // log into logfile (append)
  std::ofstream ofile;
  ofile.open(log_file_, std::ios::out | std::ios::app);
  ofile << ss.str();
}

void log2file(std::vector<long long>& lat_arr_, size_t rec_size_, std::string& log_file_)
{
  std::stringstream ss;
  ss << std::setw(6) << std::setfill('0') << rec_size_/1024;
  std::string rec_size_s = ss.str();

  std::ofstream ofile(rec_size_s + "-" +  log_file_);
  std::ostream_iterator<long long>output_iterator(ofile, "\n");
  std::copy(lat_arr_.begin(), lat_arr_.end(), output_iterator);
}
