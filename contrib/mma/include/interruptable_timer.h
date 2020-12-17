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

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <assert.h>

/**
 * @brief A timer that starts a thread and periodically calls a given callack
 */
class InteruptableTimer {
  typedef std::function<void(void)> TimerCallbackT;

public:
  /**
  * @brief Creates and a Timer instance that does nothing until the start function is called
  */
  InteruptableTimer() :
    running(false),
    stopping(false)
  {
  }


  /**
   * @brief Creates and starts a Timer instance that calls the given callback
   * @param period_   the delay betwen each calls of the callback
   * @param callback_ the callback function that shall be called periodically
   */
  template<class _Rep, class _Period>
  InteruptableTimer(std::chrono::duration<_Rep, _Period> period_, TimerCallbackT callback_):
    running(false),
    stopping(false)
  {
    start(period_, callback_);
  }

  /**
   * @brief Stops the timer. If the callback function is currently being called, this will block until the callback returns.
   */
  void stop() {
    stopping = true;
    stopCv.notify_all();
    workerThread.join();
    running = false;
  }

  /**
   * @brief Starts the Thread that calls the given callback at the given rate
   * @param period_   the delay betwen each calls of the callback
   * @param callback_ the callback function that shall be called periodically
   * @return true if the thread could be started, false otherwise.
   */
  template<class _Rep, class _Period>
  bool start(std::chrono::duration<_Rep, _Period> period_, TimerCallbackT callback_) {
    assert(running == false);
    if (running) {
      return(false);
    }
    stopping = false;
    workerThread = std::thread(&InteruptableTimer::workerFunction, this, std::chrono::duration_cast<std::chrono::nanoseconds>(period_), callback_);
    return true;
  }

private:
  bool running;   /**< Indicates if the timer is running*/
  bool stopping;  /**< Indicates if a stop request has been made*/

  std::thread             workerThread; /**< the actual thrad used for calling the callback */
  std::mutex              stopMutex;    /**< A Mutex used for that condidition variable that is used for interrupting the waiting */
  std::condition_variable stopCv;       /**< A Condidition variable that is used for interrupting the waitings*/

  /**
   * @brief The function that calles the callback at the given intervall. This function must run in a thread.
   * @param period_   the delay betwen each calls of the callback
   * @param callback_ the callback function that shall be called periodically
   */
  void workerFunction(std::chrono::nanoseconds period_, TimerCallbackT callback_) {
    running = true;
    while (!stopping) {
      auto start = std::chrono::steady_clock::now();
      (callback_)();
      auto end = std::chrono::steady_clock::now();
      auto sleep_duration = period_ - (end - start);

      std::unique_lock<std::mutex> stopLock(stopMutex);
      stopCv.wait_for(stopLock, sleep_duration, [=] {return stopping; });
    }
  }
  
};
