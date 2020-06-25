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

#include <queue>
#include <mutex>
#include <memory>
#include <chrono>
#include <thread>
#include <functional>
#include <condition_variable>

template <class T>
class ConcurrentQueue
{
public:
  ConcurrentQueue() = default;
  ~ConcurrentQueue()
  {
    DisableFlushing();
  }

  void Add(const T& value)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push_back(value);
    cv_.notify_one();
  }

  void SetFlushingCallback(const std::function<void(const T&)>& flushing_callback)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    flushing_callback_ = flushing_callback;
  }

  void RemoveFlushingCallback()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    flushing_callback_ = [](const T&) -> bool { return false; };
  }

  bool EnableFlushing()
  {
    std::lock_guard<std::mutex> lock(mutex_);

    if (flushing_) return false;
    flushing_ = true;
    std::thread thread([this]()
    {
      while (flushing_)
      {
        std::unique_lock<std::mutex> lock(mutex_);

        if (cv_.wait_for(lock, std::chrono::milliseconds(100), [this]() {return !queue_.empty(); }))
        {
          auto frame = queue_.front();
          queue_.pop_front();
          lock.unlock();
          flushing_callback_(frame);
        }
      }
    });

    std::swap(thread, thread_);

    return true;
  }

  bool DisableFlushing()
  {
    std::unique_lock<std::mutex> lock(mutex_);

    if (flushing_) {
      flushing_ = false;
      lock.unlock();
      thread_.join();
      return true;
    }
    else
      return false;
  }

  bool IsFlushing() const
  {
    return flushing_;
  }

  void Clear()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.clear();
  }

  size_t Count() const
  {
    return queue_.size();
  }

private:
  std::deque<T> queue_;
  mutable std::mutex mutex_;
  std::function<void(const T&)> flushing_callback_ = [](const T&) {};
  std::condition_variable cv_;
  std::thread thread_;
  bool flushing_ = false;
};
