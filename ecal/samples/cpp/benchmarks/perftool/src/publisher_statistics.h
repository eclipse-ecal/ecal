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
#include <vector>

struct PublishedMessage
{
  std::chrono::steady_clock::time_point publish_time;
  std::chrono::steady_clock::duration   send_call_duration;
};

using PublisherStatistics = std::vector<PublishedMessage>;

inline void printStatistics(const PublisherStatistics& statistics, bool print_verbose_times)
{
  // Compute entire entire_duration from first to last message and the mean
  auto entire_duration         = statistics.back().publish_time - statistics.front().publish_time;

  // Get the minium and maximum send_call_duration. Skip the first element, as that one actually is from teh last iteration.
  auto send_call_duration_min  = std::chrono::steady_clock::duration::max();
  auto send_call_duration_max  = std::chrono::steady_clock::duration::min();
  for (size_t i = 1; i < statistics.size(); ++i)
  {
    auto send_call_duration = statistics[i].send_call_duration;
    if (send_call_duration < send_call_duration_min)
      send_call_duration_min = send_call_duration;
    if (send_call_duration > send_call_duration_max)
      send_call_duration_max = send_call_duration;
  }

  // Compute the mean send_call_duration. Skip the first element, as that one actually is from the last iteration.
  auto send_call_duration_mean = std::chrono::steady_clock::duration(0);
  for (size_t i = 1; i < statistics.size(); ++i)
  {
    send_call_duration_mean += statistics[i].send_call_duration;
  }
  send_call_duration_mean /= (statistics.size() - 1);

  // Get the minimum and maximum loop time (based on the publish_time timestamp)
  auto loop_time_min = std::chrono::steady_clock::duration::max();
  auto loop_time_max = std::chrono::steady_clock::duration::min();
  for (size_t i = 1; i < statistics.size(); ++i)
  {
    auto loop_time = statistics[i].publish_time - statistics[i - 1].publish_time;
    if (loop_time < loop_time_min)
      loop_time_min = loop_time;
    if (loop_time > loop_time_max)
      loop_time_max = loop_time;
  }

  // Compute the mean loop time (based on the publish_time timestamp)
  auto loop_time_mean          = entire_duration / (statistics.size() - 1);

  auto loop_frequency          = 1.0 / std::chrono::duration_cast<std::chrono::duration<double>>(loop_time_mean).count();

  
  // Print statistics (mean and min/max)
  {
    std::stringstream ss;
    ss << std::right << std::fixed;
    ss << "["                      << std::setprecision(3) << std::setw(10) << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now().time_since_epoch()).count() << "]";
    ss << " | cnt:"                << std::setprecision(3) << std::setw(5) << statistics.size() - 1;
    ss << " | loop_dt(ms) mean:"   << std::setprecision(3) << std::setw(8) << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(loop_time_mean).count();
    ss << " ["                     << std::setprecision(3) << std::setw(8) << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(loop_time_min).count();
    ss << ","                      << std::setprecision(3) << std::setw(8) << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(loop_time_max).count();
    ss << "]";
    ss << " | loop_freq(Hz):"     << std::setprecision(1) << std::setw(7) << loop_frequency;
    ss << " | snd_dt(ms) mean:"   << std::setprecision(3) << std::setw(8) << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(send_call_duration_mean).count();
    ss << " ["                    << std::setprecision(3) << std::setw(8) << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(send_call_duration_min).count();
    ss << ","                     << std::setprecision(3) << std::setw(8) << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(send_call_duration_max).count();
    ss << "]";

    std::cerr << ss.str() << std::endl;
  }

  // Print all times verbose
  if (print_verbose_times)
  {
    std::stringstream ss;
    ss << std::right << std::fixed;

    ss << "  loop_dt(ms): ";
    for (size_t i = 1; i < statistics.size(); ++i)
    {
      if (i > 1)
        ss << " ";

      auto loop_time = statistics[i].publish_time - statistics[i - 1].publish_time;
      ss << std::setprecision(1) << std::setw(5) << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(loop_time).count();
    }
    ss << std::endl;

    ss << "  snd_dt(ms) : ";
    for (size_t i = 1; i < statistics.size(); ++i)
    {
      if (i > 1)
        ss << " ";
      auto send_call_duration = statistics[i].send_call_duration;
      ss << std::setprecision(1) << std::setw(5) << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(send_call_duration).count();
    }

    std::cerr << ss.str() << std::endl;
  }
}
