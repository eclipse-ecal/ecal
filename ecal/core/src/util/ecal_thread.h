/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

/**
 * @brief  eCAL threading helper class
**/

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <iostream>

#pragma once

namespace eCAL
{
  /**
   * @brief A class that encapsulates threaded functionality with a callback interface.
   */
  class CCallbackThread
  {
  public:
    /**
     * @brief Constructor for the CallbackThread class.
     * @param callback A callback function to be executed in the CallbackThread thread.
     */
    CCallbackThread(std::function<void()> callback)
      : callback_(callback) {}

    ~CCallbackThread()
    {
      stop();
    }

    CCallbackThread(const CCallbackThread&) = delete;
    CCallbackThread& operator=(const CCallbackThread&) = delete;
    CCallbackThread(CCallbackThread&& rhs) = delete;
    CCallbackThread& operator=(CCallbackThread&& rhs) = delete;

    /**
     * @brief Start the callback thread with a specified timeout.
     * @param timeout The timeout duration for waiting in the callback thread.
     */
    template <typename DurationType>
    void start(DurationType timeout)
    {
      stopThread_.store(false, std::memory_order_relaxed);
      callbackThread_ = std::thread(&CCallbackThread::callbackFunction<DurationType>, this, timeout);
    }

    /**
     * @brief Stop the callback thread.
     * Waits for the callback thread to finish its work.
     */
    void stop()
    {
      bool expected = false;
      // Set the flag to signal the callback thread to stop
      if (stopThread_.compare_exchange_strong(expected, true, std::memory_order_relaxed))
      {
        // Notify the callback thread to wake up and check the flag
        cv_.notify_one();
      }

      // Wait for the callback thread to finish
      if (callbackThread_.joinable()) {
        callbackThread_.join();
      }
    }

    /**
     * @brief Trigger the callback thread to interrupt the current sleep without stopping it.
     * The callback function will be executed immediately.
     */
    void trigger()
    {
      {
        // Set the flag to signal the callback thread to trigger
        triggerThread_.store(true, std::memory_order_relaxed);
        // Notify the callback thread to wake up and check the flag
        cv_.notify_one();
      }
    }

  private:
    std::thread callbackThread_;      /**< The callback thread object. */
    std::function<void()> callback_;  /**< The callback function to be executed in the callback thread. */
    std::mutex mtx_;                  /**< Mutex for thread synchronization. */
    std::condition_variable cv_;      /**< Condition variable for signaling between threads. */
    std::atomic<bool> stopThread_{ false };     /**< Flag to indicate whether the callback thread should stop. */
    std::atomic<bool> triggerThread_{ false };  /**< Flag to indicate whether the callback thread should be triggered. */

    /**
     * @brief Callback function that runs in the callback thread.
     * Periodically checks the stopThread flag and executes the callback function.
     * @tparam DurationType The type of the timeout duration (e.g., std::chrono::seconds, std::chrono::milliseconds).
     * @param timeout The timeout duration for waiting in the callback thread.
     */
    template <typename DurationType>
    void callbackFunction(DurationType timeout)
    {
      while (true)
      {
        {
          std::unique_lock<std::mutex> lock(mtx_);
          // Wait for a signal or a timeout
          if(cv_.wait_for(lock, timeout, [this] { return stopThread_.load(std::memory_order_relaxed) || triggerThread_.load(std::memory_order_relaxed); }))
          {
            if (stopThread_.load(std::memory_order_relaxed)) 
            {
              // If the stopThread flag is true, break out of the loop
              break;
            }

            // TO ASK: DO we always want to trigger the callback, or only when the triggerThread variable is true?
            // Previous logic:
            triggerThread_.store(false, std::memory_order_relaxed);
              }
        }

        // Do some work in the callback thread
        if (callback_)
        {
          callback_();
        }

        // revised logic:
        // if(triggerThread_.exchange(false, std::memory_order_relaxed))
        // {
        //    if (callback_) callback_();
        // }
      }
    }
  };
}
