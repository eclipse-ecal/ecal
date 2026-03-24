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

#include <unordered_set>

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

