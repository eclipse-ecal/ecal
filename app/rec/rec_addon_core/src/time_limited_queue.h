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

template <class T>
class TimeLimtedQueue : public std::enable_shared_from_this<TimeLimtedQueue<T>>
{
public:
  TimeLimtedQueue() = default;
  ~TimeLimtedQueue()
  {
    DisableFlushing();
  }

  void Add(const std::shared_ptr<T>& value)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.emplace_back(std::make_pair(std::chrono::steady_clock::now(), value));
    cv_.notify_one();
    ShrinkQueue();
  }

  void SetFlushingCallback(const std::function<bool(const std::shared_ptr<T>&)>& flushing_callback)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    flushing_callback_ = flushing_callback;
  }

  void RemoveFlushingCallback()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    flushing_callback_ = [](const std::shared_ptr<T>&) -> bool { return false; };
  }

  bool EnableFlushing()
  {
    std::lock_guard<std::mutex> lock(mutex_);

    if (flushing_) return false;
    flushing_ = true;
    std::thread thread([me = this->shared_from_this()]()
    {
      while (me->flushing_)
      {
        std::unique_lock<std::mutex> lock(me->mutex_);

        if (me->cv_.wait_for(lock, std::chrono::milliseconds(100), [me]() {return !me->queue_.empty(); }))
        {
          auto frame = me->queue_.front().second;
          me->queue_.pop_front();
          lock.unlock();
          if (!me->flushing_callback_(frame))
          {
            me->flushing_ = false;
            return;
          }
        }
      }
    });

    thread.detach();

    return true;
  }

  bool DisableFlushing()
  {
    std::unique_lock<std::mutex> lock(mutex_);

    if (flushing_) {
      flushing_ = false;
      return true;
    }
    else
      return false;
  }

  void SetLength(std::chrono::steady_clock::duration length)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    length_ = length;
  }

  std::chrono::steady_clock::duration GetLength() const 
  {
    return length_;
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

  void Copy(TimeLimtedQueue& time_limited_queue)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    std::lock_guard<std::mutex> lock2(time_limited_queue.mutex_);
    time_limited_queue.queue_ = queue_;
  }

private:
  void ShrinkQueue()
  {
    auto now = std::chrono::steady_clock::now();

    while (!queue_.empty())
    {
      if ((now - length_) > queue_.front().first)
        queue_.pop_front();
      else
        break;
    }
  }

  std::deque<std::pair<std::chrono::steady_clock::time_point, std::shared_ptr<T>>> queue_;
  mutable std::mutex mutex_;
  std::function<bool(const std::shared_ptr<T>&)> flushing_callback_ = [](const std::shared_ptr<T>&) -> bool { return false; };
  std::condition_variable cv_;
  bool flushing_ = false;
  std::chrono::steady_clock::duration length_;
};
