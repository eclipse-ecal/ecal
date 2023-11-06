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
#include <ecal/ecal_subscriber.h>

#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "subscriber_statistics.h"

class Subscriber
{
//////////////////////////////////////
/// Publisher, Destructor
//////////////////////////////////////
public:
  // Constructor that gets a frequency in Hz
  Subscriber(const std::string&                   topic_name
            , std::chrono::nanoseconds            time_to_waste
            , bool                                busy_wait
            , bool                                hickup
            , std::chrono::steady_clock::duration wait_before_hickup
            , std::chrono::steady_clock::duration hickup_delay
            , bool                                quiet
            , bool                                log_print_verbose_times);

    // Delete copy
    Subscriber(const Subscriber&)                = delete;
    Subscriber& operator=(const Subscriber&)     = delete;

    // Delete move (the class uses a this reference)
    Subscriber(Subscriber&&) noexcept            = delete;
    Subscriber& operator=(Subscriber&&) noexcept = delete;

  // Destructor
  ~Subscriber();

//////////////////////////////////////
/// Implementation
//////////////////////////////////////
private:
  void callback(const char* topic_name_, const eCAL::SReceiveCallbackData* data_);

  void statisticsLoop();

//////////////////////////////////////
/// Member variables
//////////////////////////////////////
private:
  eCAL::CSubscriber            ecal_sub;
  std::chrono::nanoseconds     time_to_waste_;
  bool                         busy_wait_;

  bool                                        hickup_;
  const std::chrono::steady_clock::duration   wait_before_hickup_;
  std::chrono::steady_clock::time_point       hickup_time_;
  const std::chrono::steady_clock::duration   hickup_delay_;

  std::unique_ptr<std::thread>    statistics_thread_;
  mutable std::mutex              mutex_;
  mutable std::condition_variable condition_variable_;
  std::atomic<bool>               is_interrupted_;
  SubscriberStatistics            statistics_;
  size_t                          statistics_size_;

  const bool                      log_print_verbose_times_;
};
