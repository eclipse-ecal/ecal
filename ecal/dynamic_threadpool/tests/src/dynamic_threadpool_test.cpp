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

#include <gtest/gtest.h>

#include <dynamic_threadpool/dynamic_threadpool.h>

#include "atomic_signalable.h"

#include <thread>
#include <chrono>

TEST(DynamicThreadpool, CreateAndDelete)
{
  // Create and delete an empty thread pool
  {
    auto thread_pool = std::make_unique<DynamicThreadPool>();

    EXPECT_EQ(thread_pool->GetSize(), 0);
    EXPECT_EQ(thread_pool->GetIdleCount(), 0);

    thread_pool.reset();
  }

  // Create and delete a thread pool with max size
  {
    const unsigned int max_size = 4;
    auto thread_pool = std::make_unique<DynamicThreadPool>(max_size);
    EXPECT_EQ(thread_pool->GetSize(), 0);
    EXPECT_EQ(thread_pool->GetIdleCount(), 0);
    thread_pool.reset();
  }
}

#if 1
// Create a theadpool and let it execute a task
TEST(DynamicThreadPool, BasicExecution)
{
  DynamicThreadPool thread_pool;

  EXPECT_EQ(thread_pool.GetIdleCount(), 0);
  EXPECT_EQ(thread_pool.GetSize(), 0);

  // Execute a single non-blocking task
  atomic_signalable<int> finished_tasks(0);

  thread_pool.Post([&finished_tasks]()
                    {
                      finished_tasks++;
                    });

  // Wait for the task to finish
  finished_tasks.wait_for([](int val) {return val >= 1; }, std::chrono::milliseconds(1000));

  // Wait a short time for the threadpool internal workerthread to also go into idle
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  EXPECT_EQ(finished_tasks.get(),       1);
  EXPECT_EQ(thread_pool.GetSize(),      1);
  EXPECT_EQ(thread_pool.GetIdleCount(), 1);
}
#endif

#if 1
// Execute multiple tasks that are blocking for a short time
TEST(DynamicThreadPool, ParallelExecution)
{
  constexpr int number_of_tasks = 4;
  const std::chrono::milliseconds delay(500);

  DynamicThreadPool thread_pool;

  atomic_signalable<int> finished_tasks(0);
  
  auto start = std::chrono::steady_clock::now();

  for (int i = 0; i < number_of_tasks; i++)
  {
    thread_pool.Post([&finished_tasks, delay]()
                      {
                        std::this_thread::sleep_for(delay);
                        finished_tasks++;
                      });

  }

  // Wait a short time, so that the tasks have not finished yet
  std::this_thread::sleep_for(delay / 2);

  EXPECT_EQ(finished_tasks.get(),       0);
  EXPECT_EQ(thread_pool.GetSize(),      number_of_tasks);
  EXPECT_EQ(thread_pool.GetIdleCount(), 0);

  // Wait for tasks to finish
  finished_tasks.wait_for([number_of_tasks](int val) { return val >= number_of_tasks; }, delay * 4);

  auto end = std::chrono::steady_clock::now();

  // Wait a short time for the threadpool internal workerthread to also go into idle
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  EXPECT_EQ(finished_tasks.get(),       number_of_tasks);
  EXPECT_EQ(thread_pool.GetSize(),      number_of_tasks);
  EXPECT_EQ(thread_pool.GetIdleCount(), number_of_tasks);

  // Check that the total time taken is "not much" more than the delay (i.e. tasks were executed in parallel)
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  EXPECT_GE(duration, delay);
  EXPECT_LT(duration, delay * 2);

  // Post again to verify that the threadpool reuses threads
  finished_tasks = 0;
  for (int i = 0; i < number_of_tasks; i++)
  {
    thread_pool.Post([&finished_tasks, delay]()
                      {
                          std::this_thread::sleep_for(delay);
                          finished_tasks++;
                      });
    
  }
  
  finished_tasks.wait_for([number_of_tasks](int val) { return val >= number_of_tasks; }, delay * 4);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  EXPECT_EQ(finished_tasks.get(),       number_of_tasks);
  EXPECT_EQ(thread_pool.GetSize(),      number_of_tasks);
  EXPECT_EQ(thread_pool.GetIdleCount(), number_of_tasks);
}
#endif

#if 1
// Limit with threadpool max size
TEST(DynamicThreadPool, MaxSize)
{
  constexpr size_t max_threadpool_size = 4;
  constexpr int number_of_tasks        = max_threadpool_size * 2;

  const std::chrono::milliseconds delay(500);

  DynamicThreadPool thread_pool(max_threadpool_size);

  atomic_signalable<int> finished_tasks(0);
  auto start = std::chrono::steady_clock::now();
  for (int i = 0; i < number_of_tasks; i++)
  {
    thread_pool.Post([&finished_tasks, delay]()
                      {
                          std::this_thread::sleep_for(delay);
                          finished_tasks++;
                      });
  }

  // Wait a short time, so that the tasks have not finished yet
  std::this_thread::sleep_for(delay / 2);

  EXPECT_EQ(finished_tasks.get(),       0);
  EXPECT_EQ(thread_pool.GetSize(),      max_threadpool_size);
  EXPECT_EQ(thread_pool.GetIdleCount(), 0);

  // Wait for tasks to finish
  finished_tasks.wait_for([number_of_tasks](int val) { return val >= number_of_tasks; }, delay * 4 * 2);

  auto end = std::chrono::steady_clock::now();

  // Wait a short time for the threadpool internal workerthread to also go into idle
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  EXPECT_EQ(finished_tasks.get(),       number_of_tasks);
  EXPECT_EQ(thread_pool.GetSize(),      max_threadpool_size);
  EXPECT_EQ(thread_pool.GetIdleCount(), max_threadpool_size);

  // Check that the total time taken is "not much" more than two times the delay (We had twice amount of tasks than max threads)
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  EXPECT_GE(duration, delay * 2);
  EXPECT_LT(duration, delay * 3);
}
#endif

#if 1
// Shutdown while tasks are running
TEST(DynamicThreadPool, Shutdown)
{
  constexpr int number_of_tasks = 2;
  const std::chrono::milliseconds delay(500);

  DynamicThreadPool thread_pool(1); // Max size 1 to ensure tasks are queued

  atomic_signalable<int> finished_tasks(0);

  auto start = std::chrono::steady_clock::now();

  for (int i = 0; i < number_of_tasks; i++)
  {
    thread_pool.Post([&finished_tasks, delay]()
                      {
                          std::this_thread::sleep_for(delay);
                          finished_tasks++;
                      });
  }

  // Wait a short time, so that the tasks have not finished yet
  std::this_thread::sleep_for(delay / 2);
  
  EXPECT_EQ(finished_tasks.get(),       0);
  EXPECT_EQ(thread_pool.GetSize(),      1);
  EXPECT_EQ(thread_pool.GetIdleCount(), 0);
  
  // Shutdown the threadpool while tasks are running
  thread_pool.Shutdown();

  // Test that adding another task now fails
  bool success = thread_pool.Post([&finished_tasks, delay]()
                                  {
                                      std::this_thread::sleep_for(delay);
                                      finished_tasks++;
                                  });

  EXPECT_FALSE(success);

  // Join the thread-pool, which makes this thread block until all remaining tasks are executed
  thread_pool.Join();

  auto end = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  EXPECT_EQ(finished_tasks.get(),       number_of_tasks); // All queued tasks should have been executed
  EXPECT_EQ(thread_pool.GetSize(),      0);
  EXPECT_EQ(thread_pool.GetIdleCount(), 0);

  // Check that the total time taken is "not much" more than all tasks combined
  EXPECT_GE(duration, delay * number_of_tasks);
  EXPECT_LT(duration, delay * (number_of_tasks + 1));
}
#endif
