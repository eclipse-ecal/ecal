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

/**
 * @brief The Stopwatch can measure a duration that can be paused at any time.
 *
 * All methods of this class are thread safe.
 */
class Stopwatch
{
public:
  /**
   * @brief Creates a new stopwatch that is not running (i.e. it is paused)
   */
  Stopwatch();

  ~Stopwatch();

  /**
   * @brief Pauses the stopwatch without discarding the already measured duration
   */
  void Pause();

  /**
   * @brief Resume the stopwatch without discarding a duration that has been measured previously
   */
  void Resume();

  /**
   * @brief Checks whether the stopwatch is paused
   * @return True if the stopwatch is currently paused
   */
  bool IsPaused();

  /**
   * @brief Getter for the measured duration since the stopwatch has been restarted
   * @return The duration that the stopwatch has been running since the last restart
   */
  std::chrono::nanoseconds GetElapsedTime();

  /**
   * @brief Restarts the stopwatch from 0. The stopwatch will start running, even it has been paused before.
   */
  void Restart();

  /**
   * @brief Atomically returns the measured duration, resets the stopwach and starts it.
   * @return The duration that the stopwatch has been running since the last restart
   */
  std::chrono::nanoseconds GetElapsedTimeAndRestart();

private:
  bool paused_;                                                                 /**< True if the stopwatch is currently paused (i.e. not measuring time) */
  std::chrono::nanoseconds elapsed_time_;                                       /**< The accumulated duration. Every time the stopwatch is paused, the measured duration is added to this variable. */

  std::chrono::steady_clock::time_point last_start_;                            /**< The time when the stopwatch has been started / resumed / restarted the last time */

  std::mutex stopwatch_mutex_;                                                  /**< A mutex to protect all methods of this class */
};

