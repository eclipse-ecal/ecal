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

#pragma once

#include <ecal/os.h>
#include <ecal/config/transport_layer.h>
#include <optional>
#include <stdexcept>

namespace eCAL::detail
{
  enum class eResolvedMutexType : std::uint8_t
  {
    winapi_mutex,
    pthread_mutex,
    pthread_robust_mutex
  };

  template<eCAL::TransportLayer::SHM::eMutexType T>
  struct shm_mutex_traits;

  template<>
  struct shm_mutex_traits<eCAL::TransportLayer::SHM::eMutexType::mutex>
  {
    static constexpr bool supported = true;
#ifdef ECAL_OS_WINDOWS
    static constexpr eResolvedMutexType resolved = eResolvedMutexType::winapi_mutex;
#else
    static constexpr eResolvedMutexType resolved = eResolvedMutexType::pthread_mutex;
#endif
  };

  template<>
  struct shm_mutex_traits<eCAL::TransportLayer::SHM::eMutexType::recoverable_mutex>
  {
#ifdef ECAL_OS_WINDOWS
    static constexpr bool supported = true;
    static constexpr eResolvedMutexType resolved = eResolvedMutexType::winapi_mutex;
#elif defined(ECAL_HAS_ROBUST_MUTEX) || defined(ECAL_HAS_CLOCKLOCK_MUTEX)
    static constexpr bool supported = true;
    static constexpr eResolvedMutexType resolved = eResolvedMutexType::pthread_robust_mutex;
#else
    static constexpr bool supported = false;
    static constexpr eResolvedMutexType resolved = eResolvedMutexType::pthread_mutex;
#endif
  };

  inline bool IsSupported(const eCAL::TransportLayer::SHM::eMutexType type)
  {
    switch (type)
    {
    case eCAL::TransportLayer::SHM::eMutexType::mutex:
      return shm_mutex_traits<eCAL::TransportLayer::SHM::eMutexType::mutex>::supported;
    case eCAL::TransportLayer::SHM::eMutexType::recoverable_mutex:
      return shm_mutex_traits<eCAL::TransportLayer::SHM::eMutexType::recoverable_mutex>::supported;
    default:
      return false;
    }
  }
    
  inline eCAL::TransportLayer::SHM::eMutexType DefaultSemanticMutexType()
  {
    return IsSupported(eCAL::TransportLayer::SHM::eMutexType::recoverable_mutex)
      ? eCAL::TransportLayer::SHM::eMutexType::recoverable_mutex
      : eCAL::TransportLayer::SHM::eMutexType::mutex;
  }

  inline eResolvedMutexType Resolve(const eCAL::TransportLayer::SHM::eMutexType type)
  {
    switch (type)
    {
    case eCAL::TransportLayer::SHM::eMutexType::mutex:
      return shm_mutex_traits<eCAL::TransportLayer::SHM::eMutexType::mutex>::resolved;
    case eCAL::TransportLayer::SHM::eMutexType::recoverable_mutex:
      if (!shm_mutex_traits<eCAL::TransportLayer::SHM::eMutexType::recoverable_mutex>::supported)
      {
        throw std::runtime_error("recoverable_mutex is not supported by this build");
      }
      return shm_mutex_traits<eCAL::TransportLayer::SHM::eMutexType::recoverable_mutex>::resolved;
    default:
      throw std::runtime_error("Unknown SHM mutex type");
    }
  }
}
