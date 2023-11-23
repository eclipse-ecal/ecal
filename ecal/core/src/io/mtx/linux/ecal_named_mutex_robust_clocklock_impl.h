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

#pragma once

#include "io/mtx/ecal_named_mutex_base.h"

typedef struct named_mutex named_mutex_t;

namespace eCAL
{
  class CNamedMutexRobustClockLockImpl : public CNamedMutexImplBase
  {
  public:
    CNamedMutexRobustClockLockImpl(const std::string &name_, bool recoverable_);
    ~CNamedMutexRobustClockLockImpl();
    
    CNamedMutexRobustClockLockImpl(const CNamedMutexRobustClockLockImpl&) = delete;
    CNamedMutexRobustClockLockImpl& operator=(const CNamedMutexRobustClockLockImpl&) = delete;
    CNamedMutexRobustClockLockImpl(CNamedMutexRobustClockLockImpl&&) = delete;
    CNamedMutexRobustClockLockImpl& operator=(CNamedMutexRobustClockLockImpl&&) = delete;


    bool IsCreated() const final;
    bool IsRecoverable() const final;
    bool WasRecovered() const final;
    bool HasOwnership() const final;

    void DropOwnership() final;

    bool Lock(int64_t timeout_) final;
    void Unlock() final;

  private:
    named_mutex_t* m_mutex_handle;
    std::string m_named;
    bool m_recoverable;
    bool m_was_recovered;
    bool m_has_ownership;
  };
}