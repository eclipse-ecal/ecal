/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2022 Continental Corporation
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

/**
 * @brief  eCAL named mutex
**/

#include <ecal/ecal_os.h>

#include "ecal_named_mutex.h"

#ifdef ECAL_OS_LINUX
#include "linux/ecal_named_mutex_impl.h"
#if defined(ECAL_HAS_ROBUST_MUTEX) || defined(ECAL_HAS_CLOCKLOCK_MUTEX)
#include "linux/ecal_named_mutex_robust_clocklock_impl.h"
#endif
#endif

#ifdef ECAL_OS_WINDOWS
#include "win32/ecal_named_mutex_impl.h"
#endif

#include <iostream>

namespace eCAL
{
  CNamedMutex::CNamedMutex(const std::string& name_, bool recoverable_) : CNamedMutex()
  {
    Create(name_, recoverable_);
  }

  CNamedMutex::CNamedMutex()
  {
    m_impl = std::make_unique<CNamedMutexStubImpl>();
  }

  CNamedMutex::~CNamedMutex()
  {
  }

  CNamedMutex::CNamedMutex(CNamedMutex&& named_mutex)
  {
    m_impl.swap(named_mutex.m_impl);
  }

  CNamedMutex& CNamedMutex::operator=(CNamedMutex&& named_mutex)
  {
    m_impl.swap(named_mutex.m_impl);
    named_mutex.m_impl.reset();
    return *this;
  }

  bool CNamedMutex::Create(const std::string& name_, bool recoverable_)
  {
#ifdef ECAL_OS_LINUX
#if !defined(ECAL_USE_CLOCKLOCK_MUTEX) && defined(ECAL_HAS_ROBUST_MUTEX)
    if(recoverable_)
      m_impl = std::make_unique<CNamedMutexRobustClockLockImpl>(name_, true);
    else
      m_impl = std::make_unique<CNamedMutexImpl>(name_, false);
#elif defined(ECAL_USE_CLOCKLOCK_MUTEX) && defined(ECAL_HAS_CLOCKLOCK_MUTEX)
    m_impl = std::make_unique<CNamedMutexRobustClockLockImpl>(name_, recoverable_);
#else
    m_impl = std::make_unique<CNamedMutexImpl>(name_, recoverable_);
#endif
#endif

#ifdef ECAL_OS_WINDOWS
    m_impl = std::make_unique<CNamedMutexImpl>(name_, recoverable_);
#endif
    return IsCreated();
  }

  void CNamedMutex::Destroy()
  {
    m_impl = std::make_unique<CNamedMutexStubImpl>();
  }

  bool CNamedMutex::IsCreated() const
  {
    return m_impl->IsCreated();
  }

  bool CNamedMutex::IsRecoverable() const
  {
    return m_impl->IsRecoverable();
  }

  bool CNamedMutex::WasRecovered() const
  {
    return m_impl->WasRecovered();
  }

  bool CNamedMutex::HasOwnership() const
  {
    return m_impl->HasOwnership();
  }

  void CNamedMutex::DropOwnership()
  {
    m_impl->DropOwnership();
  }

  bool CNamedMutex::Lock(int64_t timeout_)
  {
    return m_impl->Lock(timeout_);
  }

  void CNamedMutex::Unlock()
  {
    m_impl->Unlock();
  }
}

