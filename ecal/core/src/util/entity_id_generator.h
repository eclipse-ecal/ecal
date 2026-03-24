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
      inline std::uint32_t SecureRandom24()
      {
        std::random_device rd;

        auto random_value = rd();
        return ((static_cast<std::uint32_t>(rd()) << 16) ^ static_cast<std::uint32_t>(rd())) & 0xFFFFFFu;
      }
    }


    inline EntityIdT GenerateUniqueEntityId()
    {
      static const uint32_t process_namespace = Internal::SecureRandom24();            // 24 bit for process namespace.
      static const uint32_t pid = eCAL::Process::GetProcessID() & 0xFFFFFFu; // 24 bit for PID.
      static std::atomic<uint16_t> counter{ 0 };                             // 16 bit for counter

      uint64_t id =
        (static_cast<uint64_t>(process_namespace) << 40) |
        (static_cast<uint64_t>(pid) << 16) |
        static_cast<uint64_t>(counter.fetch_add(1, std::memory_order_relaxed));
    }
  } // namespace Util
} // namespace eCAL
