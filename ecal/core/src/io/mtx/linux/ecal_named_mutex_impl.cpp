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

#include <ecal/os.h>

#include "ecal_named_mutex_impl.h"
#include "io/shm/linux/posix_shm_region.h"

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <cstdint>
#include <string>

struct alignas(8) eCAL::CNamedMutexImpl::named_mutex_t
{
  pthread_mutex_t  mtx;
  pthread_cond_t   cvar;
  uint8_t          locked;
};
using named_mutex_t = eCAL::CNamedMutexImpl::named_mutex_t;

namespace
{
  bool named_mutex_initialize(named_mutex_t* mtx)
  {
    // create mutex
    pthread_mutexattr_t shmtx;
    pthread_mutexattr_init(&shmtx);
    pthread_mutexattr_setpshared(&shmtx, PTHREAD_PROCESS_SHARED);

    // create condition variable
    pthread_condattr_t shattr;
    pthread_condattr_init(&shattr);
    pthread_condattr_setpshared(&shattr, PTHREAD_PROCESS_SHARED);
#ifndef ECAL_OS_MACOS
    pthread_condattr_setclock(&shattr, CLOCK_MONOTONIC);
#endif // ECAL_OS_MACOS

    // initialize mutex and condition
    pthread_mutex_init(&mtx->mtx, &shmtx);
    pthread_cond_init(&mtx->cvar, &shattr);

    // start with unlocked mutex
    mtx->locked = 0;

    // clean up initialization resources
    pthread_mutexattr_destroy(&shmtx);
    pthread_condattr_destroy(&shattr);

    return true;
  }

  bool named_mutex_lock(named_mutex_t* mtx_, struct timespec* ts_)
  {
    // lock condition mutex
    pthread_mutex_lock(&mtx_->mtx);
    // state is not locked ?, fine !
    if (mtx_->locked == 0)
    {
      // set state to locked
      mtx_->locked = 1;
      // unlock condition mutex
      pthread_mutex_unlock(&mtx_->mtx);
      // return success
      return true;
    }
      // state is locked by anyone else
    else
    {
      // while condition wait did not return failure (or timeout) and
      // state is still locked by another one
      int ret(0);
      while ((ret == 0) && (mtx_->locked == 1))
      {
        // wait with timeout for unlock signal
        if (ts_)
        {
#ifndef ECAL_OS_MACOS
          ret = pthread_cond_timedwait(&mtx_->cvar, &mtx_->mtx, ts_);
#else
          ret = pthread_cond_timedwait_relative_np(&mtx_->cvar, &mtx_->mtx, ts_);
#endif
        }
          // blocking wait for unlock signal
        else
        {
          ret = pthread_cond_wait(&mtx_->cvar, &mtx_->mtx);
        }
      }
      // if wait (with timeout) returned successfully
      // set state to locked
      if (ret == 0) mtx_->locked = 1;
      // unlock condition mutex
      pthread_mutex_unlock(&mtx_->mtx);
      // sucess == wait returned 0
      return (ret == 0);
    }
  }

  bool named_mutex_trylock(named_mutex_t* mtx_)
  {
    bool locked(false);
    // lock condition mutex
    pthread_mutex_lock(&mtx_->mtx);
    // check state
    if (mtx_->locked == 0)
    {
      // set state to locked
      mtx_->locked = 1;
      locked = true;
    }
    // unlock condition mutex
    pthread_mutex_unlock(&mtx_->mtx);
    // return success
    return locked;
  }

  void named_mutex_unlock(named_mutex_t* mtx_)
  {
    // lock condition mutex
    pthread_mutex_lock(&mtx_->mtx);
    // state is locked ?
    if (mtx_->locked == 1)
    {
      // set state to unlocked
      mtx_->locked = 0;
      // and signal to one conditional wait
      // that state can be locked again
      pthread_cond_signal(&mtx_->cvar);
    }
    // unlock condition mutex
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

  CNamedMutexImpl::CNamedMutexImpl(const std::string &name_, bool /*recoverable_*/)
  {
    if(name_.empty())
      return;

    // build shm file name
    const std::string mutex_name = named_mutex_buildname(name_);

    m_shm_region = eCAL::posix::open_or_create_mapped_region<named_mutex_t>(mutex_name, named_mutex_initialize);
  }

  CNamedMutexImpl::~CNamedMutexImpl()
  {
    // check mutex handle
    if(m_shm_region.ptr() == nullptr) return;

    // unlock mutex
    named_mutex_unlock(m_shm_region.ptr());

    // close mutex
    eCAL::posix::close_region(m_shm_region);

    // clean-up if mutex instance has ownership
    if(m_shm_region.owner())
    {
      eCAL::posix::unlink_region(m_shm_region);
    }
  }

  bool CNamedMutexImpl::IsCreated() const
  {
    return m_shm_region.ptr() != nullptr;
  }

  bool CNamedMutexImpl::IsRecoverable() const
  {
    return false;
  }
  bool CNamedMutexImpl::WasRecovered() const
  {
    return false;
  }

  bool CNamedMutexImpl::HasOwnership() const
  {
    return m_shm_region.owner();
  }

  void CNamedMutexImpl::DropOwnership()
  {
    // TODO: can we just do this? what are the implications?
    m_shm_region.region.owner = false;
  }

  bool CNamedMutexImpl::Lock(int64_t timeout_)
  {
    // check mutex handle
    if (m_shm_region.ptr() == nullptr)
      return false;

    // timeout_ < 0 -> wait infinite
    if (timeout_ < 0)
    {
      return(named_mutex_lock(m_shm_region.ptr(), nullptr));
    }
      // timeout_ == 0 -> check lock state only
    else if (timeout_ == 0)
    {
      return(named_mutex_trylock(m_shm_region.ptr()));
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
      return(named_mutex_lock(m_shm_region.ptr(), &abstime));
    }
  }

  void CNamedMutexImpl::Unlock()
  {
    // check mutex handle
    if(m_shm_region.ptr() == nullptr)
      return;

    // unlock the mutex
    named_mutex_unlock(m_shm_region.ptr());
  }
}
