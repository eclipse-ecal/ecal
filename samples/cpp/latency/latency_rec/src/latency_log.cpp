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

#include "latency_log.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <numeric>
#include <sstream>

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
  ss << "Messages received             : " << sum_msg << std::endl;
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
    ss << "Message size received         : " << rec_size_ / 1024 << " kB" << std::endl;
    ss << "Message average latency       : " << avg_time << " us" << std::endl;
    ss << "Message min latency           : " << min_time << " us @ " << min_pos << std::endl;
    ss << "Message max latency           : " << max_time << " us @ " << max_pos << std::endl;
    ss << "Throughput                    : " << static_cast<int>(((rec_size_ * sum_msg) / 1024.0) / (sum_time / 1000.0 / 1000.0)) << " kB/s" << std::endl;
    ss << "                              : " << static_cast<int>(((rec_size_ * sum_msg) / 1024.0 / 1024.0) / (sum_time / 1000.0 / 1000.0)) << " MB/s" << std::endl;
    ss << "                              : " << static_cast<int>(sum_msg / (sum_time / 1000.0 / 1000.0)) << " Msg/s" << std::endl;
  }
  ss << "--------------------------------------------" << std::endl;

  // log to console
  std::cout << ss.str();

  // log into logfile (append)
  if (!log_file_.empty())
  {
    std::ofstream ofile;
    ofile.open(log_file_, std::ios::out | std::ios::app);
    ofile << ss.str();
  }
}

void log2file(std::vector<long long>& lat_arr_, size_t rec_size_, std::string& log_file_)
{
  if (!log_file_.empty())
  {
    std::stringstream ss;
    ss << std::setw(6) << std::setfill('0') << rec_size_ / 1024;
    std::string rec_size_s = ss.str();

    std::ofstream ofile(rec_size_s + "-" + log_file_);
    std::ostream_iterator<long long>output_iterator(ofile, "\n");
    std::copy(lat_arr_.begin(), lat_arr_.end(), output_iterator);
  }
}
