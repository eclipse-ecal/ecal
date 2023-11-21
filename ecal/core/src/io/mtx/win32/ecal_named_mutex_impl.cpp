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

#include "ecal_named_mutex_impl.h"

#include "ecal_win_main.h"

namespace eCAL
{
  CNamedMutexImpl::CNamedMutexImpl(const std::string &name_, bool recoverable_) : m_mutex_handle(nullptr), m_recoverable(recoverable_), m_was_recovered(false)
  {
    const std::string mutex_name = name_ + "_mtx";
    m_mutex_handle = ::CreateMutex(
      nullptr,              // no security descriptor
      false,                // mutex not owned
      mutex_name.c_str());  // object name
  }

  CNamedMutexImpl::~CNamedMutexImpl()
  {
    // check mutex handle
    if(m_mutex_handle == nullptr)
      return;

    // release it
    ReleaseMutex(m_mutex_handle);

    // close it
    CloseHandle(m_mutex_handle);
  }

  bool CNamedMutexImpl::IsCreated() const
  {
    return m_mutex_handle != nullptr;
  }

  bool CNamedMutexImpl::IsRecoverable() const
  {
    return m_recoverable;
  }
  bool CNamedMutexImpl::WasRecovered() const
  {
    return m_was_recovered;
  }

  bool CNamedMutexImpl::HasOwnership() const
  {
    return false;
  }

  void CNamedMutexImpl::DropOwnership()
  {
  }

  bool CNamedMutexImpl::Lock(int64_t timeout_)
  {
    // check mutex handle
    if(m_mutex_handle == nullptr)
      return false;

    m_was_recovered = false;

    // wait for access
    const DWORD result = WaitForSingleObject(m_mutex_handle, static_cast<DWORD>(timeout_));
    if (result == WAIT_OBJECT_0)
      return true;
    else if (result == WAIT_ABANDONED)
    {
      if (m_recoverable)
      {
        m_was_recovered = true;
        return true;
      }
      return false;
    }
    else
      return false;
  }

  void CNamedMutexImpl::Unlock()
  {
    // check mutex handle
    if(m_mutex_handle == nullptr)
      return;

    // release it
    ReleaseMutex(m_mutex_handle);
  }
}
