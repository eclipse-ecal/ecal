/* ========================= eCAL LICENSE =================================
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
 * ========================= eCAL LICENSE =================================
*/

#include <ecal/config/shm_mutex.h>

#include "io/mtx/shm_mutex_resolution.h"

namespace eCAL::Config::SHM
{
  std::string_view ToString(const eMutexType type)
  {
    switch (type)
    {
    case eMutexType::mutex:
      return "mutex";
    case eMutexType::recoverable_mutex:
      return "recoverable_mutex";
    default:
      return "mutex";
    }
  }

  std::optional<eMutexType> FromString(const std::string_view text)
  {
    if (text == "mutex" || text == "pthread_mutex")
      return eMutexType::mutex;

    if (text == "recoverable_mutex" || text == "pthread_robust_mutex" || text == "winapi_mutex")
      return eMutexType::recoverable_mutex;

    return std::nullopt;
  }

  eMutexType DefaultMutexType()
  {
    return detail::DefaultSemanticMutexType();
  }

  bool IsSupported(const eMutexType type)
  {
    return detail::IsSupported(type);
  }
}
