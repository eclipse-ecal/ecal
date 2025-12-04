/* ========================= eCAL LICENSE =================================
 *
 * Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class DynamicThreadPoolImpl
{
public:
  DynamicThreadPoolImpl();
  explicit DynamicThreadPoolImpl(size_t max_size_);

  ~DynamicThreadPoolImpl();

  // delete copy and move
  DynamicThreadPoolImpl(const DynamicThreadPoolImpl&) = delete;
  DynamicThreadPoolImpl& operator=(const DynamicThreadPoolImpl&) = delete;
  DynamicThreadPoolImpl(DynamicThreadPoolImpl&&) = delete;
  DynamicThreadPoolImpl& operator=(DynamicThreadPoolImpl&&) = delete;

  bool Post(const std::function<void()>& task_);

  size_t GetSize() const;
  size_t GetMaxSize() const;
  size_t GetIdleCount() const;

  void Shutdown();
  void Join();

private:
  mutable std::mutex      mutex;
  std::condition_variable cv;

  std::vector<std::thread>          workers;
  size_t                            max_size   = 0;
  size_t                            idle_count = 0;
  bool                              shutdown   = false;
  std::queue<std::function<void()>> tasks;
};