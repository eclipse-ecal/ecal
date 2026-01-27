/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include "ecal_named_mutex_robust_clocklock_impl.h"

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <cstdint>
#include <string>

struct alignas(8) eCAL::CNamedMutexRobustClockLockImpl::named_mutex_t
{
  pthread_mutex_t  mtx;
};
using named_mutex_t = eCAL::CNamedMutexRobustClockLockImpl::named_mutex_t;

namespace
{
  bool named_mutex_initialize(named_mutex_t* mtx, bool recoverable_ = false) 
  {
    // create mutex attribute
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

#ifdef ECAL_HAS_ROBUST_MUTEX
    if (recoverable_)
      pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
#endif
    // initialize mutex
    pthread_mutex_init(&mtx->mtx, &attr);

    // clean up initialization resources
    pthread_mutexattr_destroy(&attr);

    // return new mutex
    return true;
  }

  bool named_mutex_timedlock(named_mutex_t *mtx_, struct timespec *ts_, bool *recovered_ = nullptr) 
  {
#ifdef ECAL_HAS_CLOCKLOCK_MUTEX
    // wait with monotonic clock
    int lock_result = pthread_mutex_clocklock(&mtx_->mtx, CLOCK_MONOTONIC, ts_);
#else
    // fallback if monotonic clock is not available
    int lock_result = pthread_mutex_timedlock(&mtx_->mtx, ts_);
#endif
    if (lock_result == 0)
      return true;
      // check if previous mutex owner is dead
#ifdef ECAL_HAS_ROBUST_MUTEX
    else if (lock_result == EOWNERDEAD)
    {
      pthread_mutex_consistent(&mtx_->mtx);
      if (recovered_)
        *recovered_ = true;
      return true;
    }
#endif

    return false;
  }
  
  bool named_mutex_lock(named_mutex_t *mtx_, bool *recovered_ = nullptr) 
  {
    // wait blocking
    int lock_result = pthread_mutex_lock(&mtx_->mtx);
    
    if (lock_result == 0)
      return true;
      // check if previous mutex owner is dead
#ifdef ECAL_HAS_ROBUST_MUTEX
    else if (lock_result == EOWNERDEAD)
    {
      pthread_mutex_consistent(&mtx_->mtx);
      if (recovered_)
        *recovered_ = true;
      return true;
    }
#endif

    return false;
  }

  bool named_mutex_trylock(named_mutex_t *mtx_, bool *recovered_ = nullptr) {
    // wait with monotonic clock
    int lock_result = pthread_mutex_trylock(&mtx_->mtx);
    if (lock_result == 0)
      return true;
      // check if previous mutex owner is dead
#ifdef ECAL_HAS_ROBUST_MUTEX
    else if (lock_result == EOWNERDEAD)
    {
      pthread_mutex_consistent(&mtx_->mtx);
      if (recovered_)
        *recovered_ = true;
      return true;
    }
#endif

    return false;
  }

  void named_mutex_unlock(named_mutex_t *mtx_) 
  {
    // unlock the mutex
    pthread_mutex_unlock(&mtx_->mtx);
  }

  std::string named_mutex_buildname(const std::string& mutex_name_)
  {
    // build shm file name
    std::string mutex_name;
    if(mutex_name_[0] != '/') mutex_name = "/";
    mutex_name += mutex_name_;
    mutex_name += "_mtx";

    return(mutex_name);
  }
}

namespace eCAL
{
  CNamedMutexRobustClockLockImpl::CNamedMutexRobustClockLockImpl(const std::string &name_, bool recoverable_) : m_recoverable(false), m_was_recovered(false)
  {
    if(name_.empty())
      return;

#ifdef ECAL_HAS_ROBUST_MUTEX
    m_recoverable = recoverable_;
#endif

    // build shm file name
    const std::string mutex_name = named_mutex_buildname(name_);

    m_shm_region = eCAL::posix::open_or_create_mapped_region<named_mutex_t>(mutex_name, 
      [recoverable_](named_mutex_t* mtx) -> bool
      { 
        return named_mutex_initialize(mtx, recoverable_);
      });
  }

  CNamedMutexRobustClockLockImpl::~CNamedMutexRobustClockLockImpl()
  {
    // check mutex handle
    if(m_shm_region.ptr() == nullptr) return;

    // unlock mutex
    named_mutex_unlock(m_shm_region.ptr());

    // close mutex
    eCAL::posix::close_region(m_shm_region);

    // clean-up if mutex instance has ownership
    if(m_shm_region.owner())
      eCAL::posix::unlink_region(m_shm_region);

  }

  bool CNamedMutexRobustClockLockImpl::IsCreated() const
  {
    return m_shm_region.ptr() != nullptr;
  }

  bool CNamedMutexRobustClockLockImpl::IsRecoverable() const
  {
    return m_recoverable;
  }
  bool CNamedMutexRobustClockLockImpl::WasRecovered() const
  {
    return m_was_recovered;
  }

  bool CNamedMutexRobustClockLockImpl::HasOwnership() const
  {
    return m_shm_region.owner();
  }

  void CNamedMutexRobustClockLockImpl::DropOwnership()
  {
    // TODO: can we just do this? what are the implications?
    m_shm_region.region.owner = false;
  }

  bool CNamedMutexRobustClockLockImpl::Lock(int64_t timeout_)
  {
    // check mutex handle
    if (m_shm_region.ptr() == nullptr)
      return false;

    // reset was recovered state
    m_was_recovered = false;

    // timeout_ < 0 -> wait infinite
    if (timeout_ < 0)
    {
      return(named_mutex_lock(m_shm_region.ptr(), &m_was_recovered));
    }
      // timeout_ == 0 -> check lock state only
    else if (timeout_ == 0)
    {
      return(named_mutex_trylock(m_shm_region.ptr(), &m_was_recovered));
    }
      // timeout_ > 0 -> wait timeout_ ms
    else
    {
      struct timespec abstime {};
      clock_gettime(CLOCK_MONOTONIC, &abstime);

      abstime.tv_sec = abstime.tv_sec + timeout_ / 1000;
      abstime.tv_nsec = abstime.tv_nsec + (timeout_ % 1000) * 1000000;
      while (abstime.tv_nsec >= 1000000000)
      {
        abstime.tv_nsec -= 1000000000;
        abstime.tv_sec++;
      }
      return(named_mutex_timedlock(m_shm_region.ptr(), &abstime, &m_was_recovered));
    }
  }

  void CNamedMutexRobustClockLockImpl::Unlock()
  {
    // check mutex handle
    if(m_shm_region.ptr() == nullptr)
      return;

    // unlock the mutex
    named_mutex_unlock(m_shm_region.ptr());
  }
}
