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

#include "subscriber.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "subscriber_statistics.h"

Subscriber::Subscriber(const std::string&                   topic_name
                      , std::chrono::nanoseconds            time_to_waste
                      , bool                                busy_wait
                      , bool                                hickup
                      , std::chrono::steady_clock::duration wait_before_hickup
                      , std::chrono::steady_clock::duration hickup_delay
                      , bool                                quiet
                      , bool                                log_print_verbose_times)
  : ecal_sub                (topic_name)
  , time_to_waste_          (time_to_waste)
  , busy_wait_              (busy_wait)
  , hickup_                 (hickup)
  , wait_before_hickup_     (wait_before_hickup)
  , hickup_time_            (std::chrono::steady_clock::time_point::max())
  , hickup_delay_           (hickup_delay)
  , is_interrupted_         (false)
  , statistics_size_        (100)
  , log_print_verbose_times_(log_print_verbose_times)
{
  statistics_.reserve(statistics_size_);
  
  // create statistics thread
  if (!quiet)
    statistics_thread_ = std::make_unique<std::thread>([this](){ this->statisticsLoop(); });

  ecal_sub.SetReceiveCallback([this](const eCAL::STopicId& /*topic_id_*/, const eCAL::SDataTypeInformation& /*data_type_info_*/, const eCAL::SReceiveCallbackData& data_) { callback(data_); });
}

// Destructor
Subscriber::~Subscriber()
{
  // Interrupt the thread
  {
    const std::unique_lock<std::mutex> lock(mutex_);
    is_interrupted_ = true;
    condition_variable_.notify_all();
  }

  // Join the thread
  if (statistics_thread_)
    statistics_thread_->join();
}

void Subscriber::callback(const eCAL::SReceiveCallbackData& data_)
{
  // Initialize callback timepoint, if necessary
  if (hickup_ && hickup_time_ == std::chrono::steady_clock::time_point::max())
      hickup_time_ = std::chrono::steady_clock::now() + wait_before_hickup_;

  SubscribedMessage message_info;
  message_info.local_receive_time = std::chrono::steady_clock::now();
  message_info.ecal_receive_time  = eCAL::Time::ecal_clock::now();
  message_info.ecal_send_time     = eCAL::Time::ecal_clock::time_point(std::chrono::microseconds(data_.send_timestamp));
  message_info.ecal_counter       = data_.send_clock;
  message_info.size_bytes         = data_.buffer_size;

  std::chrono::steady_clock::duration time_to_waste_this_iteration(time_to_waste_);

  // Check if we need to hickup
  if (hickup_ && std::chrono::steady_clock::now() >= hickup_time_)
  {
    // Reset hickup (we only want to do that once)
    hickup_ = false;

    // use another sleep time for this iteratoin
    time_to_waste_this_iteration = hickup_delay_;
  }

  if (time_to_waste_this_iteration >= std::chrono::nanoseconds::zero())
  {
    if (busy_wait_)
    {
      auto start = std::chrono::high_resolution_clock::now();
      while (std::chrono::high_resolution_clock::now() - start < time_to_waste_this_iteration)
      {
        // Do nothing
      }
    }
    else
    {
      std::this_thread::sleep_for(time_to_waste_this_iteration);
    }
  }

  if (statistics_thread_)
  {
    const std::unique_lock<std::mutex>lock(mutex_);
    statistics_.push_back(message_info);
  }
}

void Subscriber::statisticsLoop()
{
  while (!is_interrupted_)
  {
    SubscriberStatistics statistics;
    statistics.reserve(statistics_size_ * 2);

    {
      std::unique_lock<std::mutex>lock (mutex_);
    
      condition_variable_.wait_for(lock, std::chrono::seconds(1), [this]() { return bool(is_interrupted_); });
    
      if (is_interrupted_)
        return;

      if(statistics_.size() > 1)
      {
        statistics_size_ = std::max(statistics_size_, statistics_.size());
        statistics_.swap(statistics);

        // Initialize the new statistics vector with the last element of the old one. This is important for detecting drops and properly computing the delay of the actual first message.
        statistics_.push_back(statistics.back());
      }
    }

    if (statistics.size() > 1)
      printStatistics(statistics, log_print_verbose_times_);
    else
      std::cerr << "Not enough data" << std::endl;
  }
}
