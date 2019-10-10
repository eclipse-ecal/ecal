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

#pragma once

#include <chrono>
#include <mutex>
#include <condition_variable>
#include <atomic>

/**
* @brief A class that is capable of sleeping the (approximately) correct amount of time while the rate of time is changing
*/
class DynamicSleeper
{
public:

  /**
  * @brief Creates an instance with a rate of 1
  *
  * The rate is the relative speed of time based on the system time.
  */
  DynamicSleeper() :
    rate_(1.0),
    last_sim_time_(0)
  {
    last_sim_time_local_timestamp_ = std::chrono::steady_clock::now();
  }

  ~DynamicSleeper() {}

  /**
  * @brief Blocks for the given amount of nanoseconds.
  *
  * How long the function actually blocks depends on the current rate at which
  * the time is supposed to run
  *
  * @param duration the duration in nanoseconds
  */
  void SleepFor(std::chrono::nanoseconds duration) {
    auto now_real = std::chrono::steady_clock::now();
    std::chrono::nanoseconds now_sim_nsecs;
    {
      std::lock_guard<std::mutex> modify_time_lock(modify_time_mutex_);
      now_sim_nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>((now_real - last_sim_time_local_timestamp_) * rate_) + last_sim_time_;
    }
    SleepUntil(now_sim_nsecs + duration);
  }

  /**
  * @brief Blocks until the given point in sim time is reached
  *
  * How long the function actually blocks depends on the current rate at which
  * the time is supposed to run
  *
  * @param time_since_epoch the simulation time in nanoseconds
  */
  void SleepUntil(std::chrono::nanoseconds time_since_epoch) {
    {
      // Clear the interrupt state
      std::unique_lock<std::mutex> wait_lock(wait_mutex_);
      interrupt_sleeping_ = false;
    }

    auto                     start_realtime = std::chrono::steady_clock::now();
    std::chrono::nanoseconds loop_start_sim_time;
    double                   original_rate;
    std::chrono::nanoseconds original_last_sim_time;

    {
      std::lock_guard<std::mutex> modify_time_lock(modify_time_mutex_);
      loop_start_sim_time    = std::chrono::duration_cast<std::chrono::nanoseconds>((start_realtime - last_sim_time_local_timestamp_) * rate_) + last_sim_time_;
      original_rate          = rate_;
      original_last_sim_time = last_sim_time_;
    }

    while (loop_start_sim_time < time_since_epoch) {
      std::unique_lock<std::mutex> wait_lock(wait_mutex_);

      if (interrupt_sleeping_)
      {
        // If the sleeping has been interrupted, we don't continue sleeping
        return;
      }

      if (original_rate > 0) {
        std::chrono::nanoseconds real_time_to_sleep = std::chrono::duration_cast<std::chrono::nanoseconds>((time_since_epoch - loop_start_sim_time) / original_rate);

        if (wait_cv_.wait_for(wait_lock, real_time_to_sleep) == std::cv_status::timeout) {
          return;
        }
      }
      else {
        wait_cv_.wait(wait_lock);
      }

      {
        std::lock_guard<std::mutex> modify_time_lock(modify_time_mutex_);
        if (last_sim_time_ < original_last_sim_time) {
          return;
        }
        start_realtime         = std::chrono::steady_clock::now();
        loop_start_sim_time    = std::chrono::duration_cast<std::chrono::nanoseconds>((start_realtime - last_sim_time_local_timestamp_) * rate_) + last_sim_time_;
        original_last_sim_time = last_sim_time_;
        original_rate          = rate_;
      }
    }
  }

  //TODO: Document
  void SleepForever()
  {
    {
      std::unique_lock<std::mutex> wait_lock(wait_mutex_);
      interrupt_sleeping_ = false;
    }

    for (;;)
    {
      std::unique_lock<std::mutex> wait_lock(wait_mutex_);

      if (interrupt_sleeping_)
      {
        // If the sleeping has been interrupted, we don't continue sleeping
        return;
      }

      wait_cv_.wait(wait_lock);
    }
  }

  /**
  * @brief sets the rate at which the time is supposed to run.
  *
  * All threads that are waiting in the sleep function will automatically
  * adjust their actual time to sleep based on the new rate.
  *
  * @param rate the rate of time relative to the system time
  */
  void SetRate(double rate) {
    auto now = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> modify_time_lock(modify_time_mutex_);

    std::chrono::nanoseconds passed_system_time = now - last_sim_time_local_timestamp_;
    std::chrono::nanoseconds passed_sim_time    = std::chrono::duration_cast<std::chrono::nanoseconds>(passed_system_time * rate_);

    last_sim_time_local_timestamp_ = now;
    last_sim_time_ += passed_sim_time;
    rate_ = rate;
    wait_cv_.notify_all();
  }

  /**
  * @brief Sets the sim time to the given value.
  *
  * If the given time is lower than the time that was given the last time or
  * lower than the time that was computed the last time the rate was changed,
  * all waiting threads will abort waiting and return.
  *
  * @param new_time_since_epoch the new simulation time in nanoseconds
  */
  void SetTime(std::chrono::nanoseconds new_time_since_epoch) {
    auto now = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> modify_time_lock(modify_time_mutex_);
    last_sim_time_local_timestamp_ = now;
    last_sim_time_ = new_time_since_epoch;
    wait_cv_.notify_all();
  }

  /**
  * @brief Sets the sim time and the rate of time to the given values.
  *
  * If the given time is lower than the time that was given the last time or
  * lower than the time that was computed the last time the rate was changed,
  * all waiting threads will abort waiting and return.
  *
  * @param new_time_since_epoch the new simulatoin time in nanoseconds
  * @param rate the rate of the simulation time relative to the system clock
  */
  void SetTimeAndRate(std::chrono::nanoseconds new_time_since_epoch, double rate) {
    auto now = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> modify_time_lock(modify_time_mutex_);
    last_sim_time_local_timestamp_ = now;
    last_sim_time_ = new_time_since_epoch;
    rate_ = rate;
    wait_cv_.notify_all();
  }

  //TODO: Document
  double GetRate()
  {
    std::lock_guard<std::mutex> modify_time_lock(modify_time_mutex_);
    return rate_;
  }

  //TODO: Document
  void InterruptSleeping()
  {
    std::unique_lock<std::mutex> interrupt_mutex(wait_mutex_);
    interrupt_sleeping_ = true;
    wait_cv_.notify_all();
  }

  /**
  * @brief getter for the internal simulation time used for computing how long to sleep
  * @return the current simulation time
  */
  std::chrono::nanoseconds GetCurrentInternalSimTime() {
    std::lock_guard<std::mutex> modify_time_lock(modify_time_mutex_);
    auto now = std::chrono::steady_clock::now();

    return std::chrono::duration_cast<std::chrono::nanoseconds>((now - last_sim_time_local_timestamp_) * rate_) + last_sim_time_;
  }

private:
  std::mutex               wait_mutex_;                                         /**< The mutex used for the condition variable that is used for sleeping*/
  std::condition_variable  wait_cv_;                                            /**< The condition variable that is used for sleeping and notifying the sleeping thread of a rate-change*/
  bool                     interrupt_sleeping_;                                 /**< If true, the sleeping has been interrupted and shall not continue, indepentent of the current time*/

  std::mutex               modify_time_mutex_;                                  /**< The Mutex for protecting variables while modifying the time point or the rate of time*/
  double                   rate_;                                               /**< The current rate at which the time proceeds*/
  std::chrono::nanoseconds last_sim_time_;                                      /**< The last know time in the simulation*/
  std::chrono::steady_clock::time_point last_sim_time_local_timestamp_;         /**< The local time when we received the last simulation time*/
};
