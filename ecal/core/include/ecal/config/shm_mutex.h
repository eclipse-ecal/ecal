/* =========================== LICENSE =================================
 *
 * Copyright (C) 2016 - 2026 Continental Corporation
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
 * =========================== LICENSE =================================
 */

#pragma once

#include <ecal/os.h>

#include <cstdint>
#include <optional>
#include <string_view>

namespace eCAL::Config::SHM
{
  enum class eMutexType : std::uint8_t
  {
    mutex,
    recoverable_mutex
  };

  ECAL_API std::string_view ToString(eMutexType type);
  ECAL_API std::optional<eMutexType> FromString(std::string_view text);
  ECAL_API eMutexType DefaultMutexType();
  ECAL_API bool IsSupported(eMutexType type);
}
