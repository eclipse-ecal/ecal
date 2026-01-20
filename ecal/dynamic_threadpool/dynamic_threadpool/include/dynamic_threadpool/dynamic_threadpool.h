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

#include <cstddef>
#include <functional>
#include <memory>

/**
 * @brief Class that implements a dynamically growing thread pool that can execute tasks in parallel.
 */
class DynamicThreadPool
{
public:
  /**
   * @brief Create a new thread pool with unlimited maximum size.
   */
  DynamicThreadPool();

  /**
   * @brief Create a new thread pool with a maximum size.
   * 
   * Tasks that are posted to the threadpool may be queued if all threads are
   * busy and the maximum size is reached. When threads become idle again,
   * queued tasks will be executed.
   * On shutdown, all queued tasks will still be executed before the threadpool
   * is fully shut down.
   * 
   * @param max_size_ Maximum number of threads in the pool.
   */
  explicit DynamicThreadPool(unsigned int max_size_);

  ~DynamicThreadPool();

  // delete copy
  DynamicThreadPool(const DynamicThreadPool&) = delete;
  DynamicThreadPool& operator=(const DynamicThreadPool&) = delete;

  // default move
  DynamicThreadPool(DynamicThreadPool&&) = default;
  DynamicThreadPool& operator=(DynamicThreadPool&&) = default;

  /**
   * @brief Posts a task to the thread pool for execution.
   * 
   * This method may either execute the task immediately in an idle thread if
   * available, or create a new thread if the maximum size is not reached, yet.
   * If the maximum size is reached and all threads are busy, the task will be
   * queued
   *
   * If shutdown has been called on the thread pool, posting a new task will fail.
   *
   * @param task_ The task to be executed by the thread pool.
   * @return True if the task was successfully posted; otherwise, false.
   */
  bool Post(const std::function<void()>& task_);

  /**
   * @brief Get the current number of threads in the pool.
   * @return The number of threads in the pool.
   */
  [[nodiscard]] size_t GetSize() const;

  /**
    * @brief Get the current number of idle threads in the pool.
    * @return The number of idle threads in the pool.
    */
  [[nodiscard]] size_t GetIdleCount() const;

  /**
   * @brief Shuts down the thread pool, allowing all queued tasks to finish.
   * 
   * After calling this method, no new tasks can be posted to the thread pool.
   * This method does not block; to wait for all threads to finish, call Join()
   * after Shutdown().
   */
  void Shutdown();

  /**
   * @brief Waits for all threads to finish execution.
   * 
   * This method must be called *after* Shutdown().
   */
  void Join();

private:
  std::unique_ptr<class DynamicThreadPoolImpl> impl;

};