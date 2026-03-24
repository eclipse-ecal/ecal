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
#include <string>

namespace eCAL
{
  namespace Util
  {
    namespace Internal
    {
      inline std::uint64_t Mix64(std::uint64_t value)
      {
        // splitmix64
        value += 0x9E3779B97F4A7C15ULL;
        value = (value ^ (value >> 30)) * 0xBF58476D1CE4E5B9ULL;
        value = (value ^ (value >> 27)) * 0x94D049BB133111EBULL;
        return value ^ (value >> 31);
      }

      inline std::uint64_t HashHostName(const std::string& host_name_)
      {
        // 64-bit FNV-1a
        std::uint64_t hash = 14695981039346656037ULL;
        for (unsigned char c : host_name_)
        {
          hash ^= c;
          hash *= 1099511628211ULL;
        }
        return hash;
      }

      inline std::uint64_t CreateProcessSeed()
      {
        const std::uint64_t process_id = static_cast<std::uint64_t>(eCAL::Process::GetProcessID());
        const std::uint64_t host_hash  = HashHostName(eCAL::Process::GetHostName());
        return Mix64((host_hash << 1) ^ process_id);
      }
    } // namespace Internal

    inline EntityIdT GenerateUniqueEntityId()
    {
      static const std::uint64_t process_seed = Internal::CreateProcessSeed();
      static std::atomic<std::uint64_t> process_counter{ 0 };

      const std::uint64_t counter = process_counter.fetch_add(1, std::memory_order_relaxed);
      return static_cast<EntityIdT>(Internal::Mix64(process_seed + counter));
    }
  } // namespace Util
} // namespace eCAL
