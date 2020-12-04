/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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

#include "interruptible_thread.h"

/**
 * @brief This class describes a Thread that continuously loops a function at a given rate until it is interrupted.
 *
 * For implementing an Interruptible Loop Thread, there are three important
 * functions:
 *
 * void Loop():
 *    This is the function that has to be implemented. This function will be
 *    looped at the given rate with best effort, once Start() is called.
 *    The rate may slow down when executing Loop() takes longer than the loop
 *    time)
 *
 * bool IsInterrupted():
 *    This function should be checked regularily. By calling interrupt(), an
 *    external thread can interrupt the thread, but the thread has to perform
 *    a gracefull shutdown itself.
 *
 * void SleepFor(dur):
 *    A interruptible thread should not use blocking functions that are not
 *    interruptible. Therefore, the SleepFor() function stops sleeping once any
 *    thread calls Interrupt(). The implementation should always check whether
 *    the thread has been interrupted after sleeping and perform a gracefull
 *    shutdown if that is the case.
 *
 * @author Florian Reimold
 */
class InterruptibleLoopThread : public InterruptibleThread
{
public:
  /**
   * @brief Creates a new InterruptibleLoopThread that will loop with the given loop time
   * @param loop_time the time for each loop
   */
  InterruptibleLoopThread(std::chrono::nanoseconds loop_time)
    : m_loop_time(loop_time)
  {}

  InterruptibleLoopThread(const InterruptibleLoopThread&) = delete;
  InterruptibleLoopThread& operator=(const InterruptibleLoopThread&) = delete;


  virtual ~InterruptibleLoopThread()
  {}

  /**
   * @brief Sets the loop time to the given value.
   *
   * The new loop time will take effect in the next loop.
   *
   * @param loop_time the new loop time
   */
  void SetLoopTime(std::chrono::nanoseconds loop_time)
  {
    m_loop_time = loop_time;
  }

  /**
   * @brief Gets the loop time.
   * @return The current loop time
   */
  std::chrono::nanoseconds GetLoopTime()
  {
    return std::chrono::nanoseconds(m_loop_time);
  }

protected:
  void Run() {
    auto next_loop = std::chrono::steady_clock::time_point(std::chrono::nanoseconds(0));
    while (!IsInterrupted()) {
      auto now = std::chrono::steady_clock::now();
      auto time_to_wait = next_loop - now;
      if (time_to_wait < std::chrono::nanoseconds(0)) {
        // The last iteration has taken too long => We do not wait this time and set "now" as the new baseline for the next iteration
        next_loop = now + GetLoopTime();
      }
      else {
        // Sleep for the remaining time
        SleepFor(time_to_wait);
        next_loop = next_loop + GetLoopTime();
      }
      if (IsInterrupted()) {
        return;
      }
      Loop();
    }
  }

  /**
   * @brief The function that will be looped. The user has to implement this function.
   */
  virtual void Loop() = 0;

private:
  std::atomic<std::chrono::nanoseconds> m_loop_time;    /**< The time at which the thread will loop */
};

