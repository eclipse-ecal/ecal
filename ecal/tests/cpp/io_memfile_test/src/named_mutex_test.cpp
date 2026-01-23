/* ========================= eCAL LICENSE =================================
 *
 * Copyright 2026 AUMOVIO and subsidiaries. All rights reserved.
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

#include "io/mtx/ecal_named_mutex.h"

#include <gtest/gtest.h>
#include <random>
#include <string>
#include <thread>

#include <ecal_utils/barrier.h>


namespace
{
  // Best-effort unique name for each test instance.
  inline std::string RandomMutexName20()
  {
    static constexpr char alphabet[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    static constexpr size_t alphabet_size = sizeof(alphabet) - 1;
    static constexpr size_t name_length = 20;

    thread_local std::mt19937_64 rng{ std::random_device{}() };
    std::uniform_int_distribution<size_t> dist(0, alphabet_size - 1);

    std::string name;
    name.reserve(name_length);

    for (size_t i = 0; i < name_length; ++i)
      name.push_back(alphabet[dist(rng)]);

    return name;
  }
}

TEST(core_cpp_io, MutexLockUnlock)
{
  const std::string mutex_name = RandomMutexName20();
  eCAL::CNamedMutex mutex(mutex_name);
  EXPECT_EQ(true, mutex.Lock(0));
  mutex.Unlock();
}

/**
 * Verifies that two synchronized threads can concurrently create, lock,
 * and destroy named mutex instances without contention or failure.
 *
 * Each thread attempts to lock a uniquely named mutex in every iteration
 * and must succeed exactly once per iteration.
 */
TEST(core_cpp_io, MutexParallelCreate)
{
  // parameter
  const std::string mutex_name = RandomMutexName20();
  const int runs = 1000;

  Barrier barrier(2);

  auto mutex_create_function = [&barrier, &mutex_name, runs](int& number_times_locked)
    {
      for (int i = 0; i < runs; ++i)
      {
        barrier.wait();
        eCAL::CNamedMutex mutex(mutex_name + "_" + std::to_string(i));
        barrier.wait();
        if (mutex.Lock(1000))
        {
          ++number_times_locked;
          mutex.Unlock();
        }
        barrier.wait();
      }
    };

  int number_times_locked_1{ 0 };
  int number_times_locked_2{ 0 };

  std::thread mutex_thread_1(mutex_create_function, std::ref(number_times_locked_1));
  std::thread mutex_thread_2(mutex_create_function, std::ref(number_times_locked_2));
  mutex_thread_1.join();
  mutex_thread_2.join();

  ASSERT_EQ(number_times_locked_1, runs);
  ASSERT_EQ(number_times_locked_2, runs);
}

// Only one thread should be able to lock the mutex at a time.
TEST(core_cpp_io, MutexParallelLockUnlock)
{
  // parameter
  const std::string mutex_name = RandomMutexName20();
  const int runs = 10000;

  Barrier barrier(3);
  std::atomic<int> lock_count{ 0 };

  auto mutex = [&barrier, &mutex_name, runs, &lock_count]()
    {
      barrier.wait();
      eCAL::CNamedMutex mutex(mutex_name);
      for (int i = 1; i <= runs; ++i)
      {
        barrier.wait();
        if (mutex.Lock(0))
        {
          lock_count.fetch_add(1);
          barrier.wait();
          mutex.Unlock();
        }
        else
        {
          // Lock failed
          barrier.wait();
        }
        barrier.wait();
        barrier.wait();
      }
    };

  auto run_gtest = [&barrier, runs, &lock_count]()
  {
    barrier.wait();
    for (int i = 1; i <= runs; ++i)
    {
      barrier.wait();
      barrier.wait();
      barrier.wait();
      EXPECT_EQ(lock_count.load(), i);
      barrier.wait();
    }
  };

  std::thread mutex_thread_1(mutex);
  std::thread mutex_thread_2(mutex);
  std::thread gtest_thread(run_gtest);
  mutex_thread_1.join();
  mutex_thread_2.join();
  gtest_thread.join();
}