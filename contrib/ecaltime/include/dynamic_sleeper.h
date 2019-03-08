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
class CDynamicSleeper
{
public:

  /**
  * @brief Creates an instance with a rate of 1
  *
  * The rate is the relative speed of time based on the system time.
  */
  CDynamicSleeper() :
    rate(1.0),
    lastSimTime(0)
  {
    lastSimTimeLocalTimestamp = std::chrono::steady_clock::now();
  }

  /**
  * @brief Blocks for the given amount of nanoseconds.
  *
  * How long the function actually blocks depends on the current rate at which
  * the time is supposed to run
  *
  * @param durationNsecs_ the duration in nanoseconds
  */
  void sleepFor(long long durationNsecs_) {
    auto nowReal = std::chrono::steady_clock::now();
    long long nowSim;
    {
      std::unique_lock<std::mutex> modifyTimeLock(modifyTimeMutex);
      nowSim = (long long)((double)((nowReal - lastSimTimeLocalTimestamp).count()) * rate) + lastSimTime;
    }
    sleepUntil(nowSim + durationNsecs_);
  }

  /**
  * @brief Blocks until the given point in sim time is reached
  *
  * How long the function actually blocks depends on the current rate at which
  * the time is supposed to run
  *
  * @param sleepUntilTimeNsecs_ the simulation time in nanoseconds
  */
  void sleepUntil(long long sleepUntilTimeNsecs_) {
    auto      startRealtime = std::chrono::steady_clock::now();
    long long loopStartSimTime;
    double    originalRate;
    long long originalLastSimtime;

    {
      std::unique_lock<std::mutex> modifyTimeLock(modifyTimeMutex);
      loopStartSimTime = (long long)((double)((startRealtime - lastSimTimeLocalTimestamp).count()) * rate) + lastSimTime;
      originalRate = rate;
      originalLastSimtime = lastSimTime;
    }

    while (loopStartSimTime < sleepUntilTimeNsecs_) {
      std::unique_lock<std::mutex> waitLck(waitMutex);
      if (originalRate > 0) {
        std::chrono::duration<long long, std::nano> realTimeToSleep((long long)((double)(sleepUntilTimeNsecs_ - loopStartSimTime) / originalRate));
        if (waitCv.wait_for(waitLck, realTimeToSleep) == std::cv_status::timeout) {
          return;
        }
      }
      else {
        waitCv.wait(waitLck);
      }

      {
        std::unique_lock<std::mutex> modifyTimeLock(modifyTimeMutex);
        if (lastSimTime < originalLastSimtime) {
          return;
        }
        startRealtime = std::chrono::steady_clock::now();
        loopStartSimTime = (long long)((double)((startRealtime - lastSimTimeLocalTimestamp).count()) * rate) + lastSimTime;
        originalLastSimtime = lastSimTime;
        originalRate = rate;
      }
    }
  }

  /**
  * @brief sets the rate at which the time is supposed to run.
  *
  * All threads that are waiting in the sleep function will automatically
  * adjust their actual time to sleep based on the new rate.
  *
  * @param rate_ the rate of time relative to the system time
  */
  void setRate(double rate_) {
    auto now = std::chrono::steady_clock::now();
    std::unique_lock<std::mutex> modifyTimeLock(modifyTimeMutex);
    long long passedSystemNsecs = (now - lastSimTimeLocalTimestamp).count();
    long long passedSimtimeNsecs = (long long)((double)passedSystemNsecs * rate);
    lastSimTimeLocalTimestamp = now;
    lastSimTime += passedSimtimeNsecs;
    rate = rate_;
    waitCv.notify_all();
  }

  /**
  * @brief Sets the sim time to the given value.
  *
  * If the given time is lower than the time that was given the last time or
  * lower than the time that was computed the last time the rate was changed,
  * all waiting threads will abort waiting and return.
  *
  * @param nsecs_ the new simulation time in nanoseconds
  */
  void setTime(long long nsecs_) {
    auto now = std::chrono::steady_clock::now();
    std::unique_lock<std::mutex> modifyTimeLock(modifyTimeMutex);
    lastSimTimeLocalTimestamp = now;
    lastSimTime = nsecs_;
    waitCv.notify_all();
  }

  /**
  * @brief Sets the sim time and the rate of time to the given values.
  *
  * If the given time is lower than the time that was given the last time or
  * lower than the time that was computed the last time the rate was changed,
  * all waiting threads will abort waiting and return.
  *
  * @param time_nsecs_ the new simulatoin time in nanoseconds
  * @param rate_ the rate of the simulation time relative to the system clock
  */
  void setTimeAndRate(long long time_nsecs_, double rate_) {
    auto now = std::chrono::steady_clock::now();
    std::unique_lock<std::mutex> modifyTimeLock(modifyTimeMutex);
    lastSimTimeLocalTimestamp = now;
    lastSimTime = time_nsecs_;
    rate = rate_;
    waitCv.notify_all();
  }

  /**
  * @brief getter for the internal simulation time used for computing how long to sleep
  * @return the current simulation time
  */
  long long getCurrentInternalSimTime() {
    std::unique_lock<std::mutex> modifyTimeLock(modifyTimeMutex);
    auto now = std::chrono::steady_clock::now();
    return (long long)((double)((now - lastSimTimeLocalTimestamp).count()) * rate) + lastSimTime;
  }

  ~CDynamicSleeper() {}

private:
  std::mutex waitMutex;             /**< The mutex used for the condition variable that is used for sleeping*/
  std::condition_variable waitCv;   /**< The condition variable that is used for sleeping and notifying the sleeping thread of a rate-change*/

  std::mutex modifyTimeMutex;       /**< The Mutex for protecting variables while modifying the time point or the rate of time*/
  double rate;                      /**< The current rate at which the time proceeds*/
  long long lastSimTime;            /**< The last know time in the simulation*/
  std::chrono::time_point<std::chrono::steady_clock> lastSimTimeLocalTimestamp; /**< The local time when we received the last simulation time*/
};
