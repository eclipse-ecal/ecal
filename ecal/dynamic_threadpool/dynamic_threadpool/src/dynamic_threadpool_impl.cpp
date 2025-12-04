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

#include "dynamic_threadpool_impl.h"

#include <cassert>

DynamicThreadPoolImpl::DynamicThreadPoolImpl()
{}

DynamicThreadPoolImpl::DynamicThreadPoolImpl(size_t max_size_)
  : max_size(max_size_)
{}

DynamicThreadPoolImpl::~DynamicThreadPoolImpl()
{
  Shutdown();
  Join();
}

bool DynamicThreadPoolImpl::Post(const std::function<void()>& task)
{
  std::unique_lock<std::mutex> const lock(mutex);

  if (shutdown)
  {
      return false;
  }

  // Add the task to the queue
  tasks.push(task);

  // If there are idle workers, notify one of them
  if (idle_count > 0)
  {
    cv.notify_one();
  }
  // If no idle workers and we can create more, spawn a new worker
  else if (max_size <= 0 || (workers.size() < max_size))
  {
    workers.emplace_back([this]()
                          {
                            while (true)
                            {
                              std::function<void()> current_task;
                              {
                                std::unique_lock<std::mutex> lock(mutex);
                                
                                // Wait for a task or shutdown signal
                                idle_count++;
                                cv.wait(lock, [this]() { return !tasks.empty() || shutdown; });
                                idle_count--;
                                
                                if (shutdown && tasks.empty())
                                {
                                  return;
                                }

                                // Get the next task
                                current_task = tasks.front();
                                tasks.pop();
                              }

                              // Execute the task outside the lock
                              current_task();
                            }
                          });
  }
  return true;
}

void DynamicThreadPoolImpl::Shutdown()
{
  {
    std::unique_lock<std::mutex> const lock(mutex);
    shutdown = true;
    cv.notify_all();
  }
}

void DynamicThreadPoolImpl::Join()
{
  assert(shutdown && "Shutdown must be called before Join");

  // Join all worker threads
  for (std::thread& worker : workers)
  {
    if (worker.joinable())
    {
      worker.join();
    }
  }

  // Clear workers vector
  workers.clear();
}

size_t DynamicThreadPoolImpl::GetSize() const
{
  std::unique_lock<std::mutex> const lock(mutex);
  return workers.size();
}

size_t DynamicThreadPoolImpl::GetMaxSize() const
{
  return max_size;
}

size_t DynamicThreadPoolImpl::GetIdleCount() const
{
  std::unique_lock<std::mutex> const lock(mutex);
  return idle_count;
}
