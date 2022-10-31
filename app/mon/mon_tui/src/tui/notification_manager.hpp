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
#include <functional>
#include <mutex>
#include <thread>
#include <condition_variable>

enum class StatusLevel { DEFAULT, ERROR };

struct Status
{
  std::string text;
  StatusLevel level;
};

using NotifyCallback = std::function<void(const Status &status)>;
using NotifyExpiredCallback = std::function<void()>;

class NotificationManager
{
  NotifyCallback notify_callback;
  NotifyExpiredCallback notify_expired_callback;

  std::thread expiration_waiter;
  std::condition_variable expiration_cv;
  std::mutex expiration_waiter_mutex;

  void WaitForExpiration()
  {
    using namespace std::chrono_literals;
    std::unique_lock<std::mutex> lock{expiration_waiter_mutex};
    expiration_cv.wait_for(lock, 5s);
    if(notify_expired_callback) notify_expired_callback();
  }

  void ExpireEarly()
  {
    std::lock_guard<std::mutex> lock{expiration_waiter_mutex};
    expiration_cv.notify_all();
  }

  void StartWaitingForExpiration()
  {
    expiration_waiter = std::thread(std::bind(&NotificationManager::WaitForExpiration, this));
    expiration_waiter.detach();
  }

public:
  void Notify(StatusLevel level, const std::string &message)
  {
    ExpireEarly();
    StartWaitingForExpiration();
    if(notify_callback) notify_callback({message, level});
  }

  void SetNotifyCallback(NotifyCallback callback)
  {
    notify_callback = callback;
  }

  void SetExpiredCallback(NotifyExpiredCallback callback)
  {
    notify_expired_callback = callback;
  }
};
