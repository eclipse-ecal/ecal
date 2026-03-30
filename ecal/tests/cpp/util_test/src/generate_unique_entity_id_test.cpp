/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include <util/entity_id_generator.h>
#include "ecal_utils/barrier.h"

#include <atomic>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <vector>

#include <gtest/gtest.h>

TEST(core_cpp_util, GenerateUniqueEntityId_10000CallsAllUnique)
{
  constexpr std::size_t num_ids = 10000;

  std::unordered_set<eCAL::EntityIdT> ids;
  ids.reserve(num_ids);

  for (std::size_t i = 0; i < num_ids; ++i)
  {
    const auto id = eCAL::Util::GenerateUniqueEntityId();
    const auto inserted = ids.insert(id);
    EXPECT_TRUE(inserted.second) << "Duplicate entity id generated at index " << i << ": " << id;
  }
}

TEST(core_cpp_util, GenerateUniqueEntityId_MultiThreadedAllUnique)
{
  constexpr std::size_t num_threads = 8;
  constexpr std::size_t ids_per_thread = 5000;
  constexpr std::size_t total_ids = num_threads * ids_per_thread;

  std::unordered_set<eCAL::EntityIdT> ids;
  ids.reserve(total_ids);
  
  Barrier barrier(num_threads);

  std::mutex ids_mutex;
  std::vector<std::thread> threads;
  threads.reserve(num_threads);

  for (std::size_t t = 0; t < num_threads; ++t)
  {
    threads.emplace_back([&ids, &ids_mutex, ids_per_thread, &barrier]()
      {
        std::vector<eCAL::EntityIdT> local_ids;
        local_ids.reserve(ids_per_thread);

        barrier.wait();
        for (std::size_t i = 0; i < ids_per_thread; ++i)
        {
          local_ids.push_back(eCAL::Util::GenerateUniqueEntityId());
        }

        std::lock_guard<std::mutex> lock(ids_mutex);
        for (const auto id : local_ids)
        {
          const auto inserted = ids.insert(id);
          EXPECT_TRUE(inserted.second) << "Duplicate entity id generated: " << id;
        }
      });
  }

  for (auto& thread : threads)
  {
    thread.join();
  }

  EXPECT_EQ(ids.size(), total_ids);
}

