/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2023 Continental Corporation
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

#pragma once

#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <ratio>
#include <sstream>
#include <string>
#include <vector>

#include <ecal/ecal_time.h>

struct SubscribedMessage
{
  std::chrono::steady_clock::time_point local_receive_time;
  eCAL::Time::ecal_clock::time_point    ecal_send_time;
  eCAL::Time::ecal_clock::time_point    ecal_receive_time;
  unsigned long long                    size_bytes{};
  unsigned long long                    ecal_counter{};
};

using SubscriberStatistics = std::vector<SubscribedMessage>;

inline void printStatistics(const SubscriberStatistics& statistics, bool print_verbose_times)
{
  // Compute entire entire_duration from first to last message and the mean
  const auto entire_duration         = statistics.back().local_receive_time - statistics.front().local_receive_time;

  // The first message is from the previous loop run and only exists to count lost messages properly and to compute the delay of the actual first message.
  const int received_msgs        = static_cast<int>(statistics.size()) - 1;

  // Check if the ecal_counter is continous. If not, we have lost messages. Count them.
  bool ecal_counter_is_monotinc = true;
  unsigned long long lost_msgs  = 0;
  for (size_t i = 1; i < statistics.size(); ++i)
  {
    if (statistics[i].ecal_counter <= statistics[i - 1].ecal_counter)
    {
      ecal_counter_is_monotinc = false;
      break;
    }
    lost_msgs += statistics[i].ecal_counter - statistics[i - 1].ecal_counter - 1;
  }

  // Get the minimum and maximum delay time between two messages
  auto msg_dt_min = std::chrono::steady_clock::duration::max();
  auto msg_dt_max = std::chrono::steady_clock::duration::min();
  for (size_t i = 1; i < statistics.size(); ++i)
  {
    auto delay = statistics[i].local_receive_time - statistics[i - 1].local_receive_time;
    if (delay < msg_dt_min)
      msg_dt_min = delay;
    if (delay > msg_dt_max)
      msg_dt_max = delay;
  }
  auto msg_dt_mean             = entire_duration / (statistics.size() - 1);

  auto msg_frequency           = 1.0 / std::chrono::duration_cast<std::chrono::duration<double>>(msg_dt_mean).count();
  
  // Print mean entire_duration and rmse in a single line in milliseconds
  {
    std::stringstream ss;
    ss << std::right << std::fixed;
    ss << "["                     << std::setprecision(3) << std::setw(10) << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now().time_since_epoch()).count() << "]";
    ss << " | cnt:"               << std::setprecision(3) << std::setw(5) << received_msgs;
    ss << " | lost:"              << std::setprecision(3) << std::setw(5) << (ecal_counter_is_monotinc ? std::to_string(lost_msgs) : "???");
    ss << " | msg_dt(ms) mean:"   << std::setprecision(3) << std::setw(8) << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg_dt_mean).count();
    ss << " ["                    << std::setprecision(3) << std::setw(8) << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg_dt_min).count();
    ss << ","                     << std::setprecision(3) << std::setw(8) << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg_dt_max).count();
    ss << "]";
    ss << " | msg_freq(Hz):"      << std::setprecision(1) << std::setw(7) << msg_frequency;

    std::cerr << ss.str() << std::endl;
  }

  // Print verbose times
  if (print_verbose_times)
  {
    std::stringstream ss;
    ss << std::right << std::fixed;
    ss << "  msg_dt(ms): ";
    for (size_t i = 1; i < statistics.size(); ++i)
    {
      if (i > 1)
        ss << " ";

      auto delay = statistics[i].local_receive_time - statistics[i - 1].local_receive_time;
      ss << std::setprecision(1) << std::setw(5) << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(delay).count();
    }

    std::cerr << ss.str() << std::endl;
  }
}
