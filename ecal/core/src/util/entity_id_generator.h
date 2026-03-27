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

#pragma once

#include <ecal/process.h>
#include <ecal/types.h>

#include <atomic>
#include <cstdint>
#include <random>

namespace eCAL
{
  namespace Util
  {
    namespace Internal
    {
      inline std::uint64_t Random24BitUint()
      {
        std::random_device rd;

        return static_cast<std::uint64_t>(rd()) & 0xFFFFFFu;
      }
    }

    /*
    * Generates a unique EntityIdT using a combination of the process ID, a random namespace, and an atomic counter.
    * This function is thread-safe.
    * For design decisions, reference ADR-0001-entity-id-generation.md in the eCAL documentation.
    */
    inline EntityIdT GenerateUniqueEntityId()
    {
      // 24 bit for PID.
      static const uint64_t pid = static_cast<uint64_t>(eCAL::Process::GetProcessID() & 0xFFFFFFu) << 40;
      // 24 bit for process namespace.
      static const uint64_t process_namespace = Internal::Random24BitUint() << 16;
      // 16 bit for counter -> might leak into process_namespace once it is larger than 65535, but this is acceptable for our use case.
      static std::atomic<uint64_t> counter{ 0 };

      const EntityIdT id =
        pid + process_namespace + counter.fetch_add(1, std::memory_order_relaxed);

      return id;
    }
  } // namespace Util
} // namespace eCAL
