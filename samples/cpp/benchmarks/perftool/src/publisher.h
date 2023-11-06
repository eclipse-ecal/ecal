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

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <ecal/ecal_publisher.h>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "publisher_statistics.h"

class Publisher
{
//////////////////////////////////////
/// Publisher, Destructor
//////////////////////////////////////
public:
  // Constructor that gets a frequency in Hz
  Publisher(const std::string& topic_name, double frequency, std::size_t payload_size, bool quiet, bool log_print_verbose_times);

  // Delete copy
  Publisher(const Publisher&)                = delete;
  Publisher& operator=(const Publisher&)     = delete;

  // Delete move (the class uses a this reference)
  Publisher(Publisher&&) noexcept            = delete;
  Publisher& operator=(Publisher&&) noexcept = delete;

  // Destructor
  ~Publisher();

//////////////////////////////////////
/// Implementation
//////////////////////////////////////
private:
  void loop();
  void statisticsLoop();

  bool preciseWaitUntil(std::chrono::steady_clock::time_point time) const;

//////////////////////////////////////
/// Member variables
//////////////////////////////////////
private:
  eCAL::CPublisher                      ecal_pub;
  const double                          frequency_;
  std::vector<char>                     payload_;

  std::unique_ptr<std::thread>          publisher_thread_;
  std::unique_ptr<std::thread>          statistics_thread_;

  std::chrono::nanoseconds              period_;
  std::chrono::steady_clock::time_point next_deadline_;

  mutable std::mutex                    mutex_;
  mutable std::condition_variable       condition_variable_;
  std::atomic<bool>                     is_interrupted_;
  PublisherStatistics                   statistics_;

  const bool                            log_print_verbose_times_;
};
