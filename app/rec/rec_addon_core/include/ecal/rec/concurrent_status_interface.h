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

#include <mutex>
#include <unordered_map>
#include <thread>

template <class T>
class ConcurrentStatusInterface
{
public:
  T GetLastStatus()
  {
    std::lock_guard<std::mutex> lock(last_status_mutex_);
    auto iter = last_status_.find(std::this_thread::get_id());
    if (iter == last_status_.end()) return T();
    return iter->second;
  }

protected:
  void SetLastStatus(const T& status)
  {
    std::lock_guard<std::mutex> lock(last_status_mutex_);
    auto result = last_status_.insert(std::pair<std::thread::id, std::string>(std::this_thread::get_id(), status));
    if (!result.second)
      result.first->second = status;
  }

  void ClearStatus()
  {
    std::lock_guard<std::mutex> lock(last_status_mutex_);
    last_status_.erase(std::this_thread::get_id());
  }

private:
  std::mutex last_status_mutex_;
  std::unordered_map<std::thread::id, T> last_status_;
};