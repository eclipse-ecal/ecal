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

struct alignas(8) named_mutex
{
  pthread_mutex_t  mtx;
};
typedef struct named_mutex named_mutex_t;

namespace
{
  named_mutex_t *named_mutex_create(const char *mutex_name_, bool recoverable_ = false) 
  {
    // create shared memory file
    int previous_umask = umask(000);  // set umask to nothing, so we can create files with all possible permission bits
    int fd = ::shm_open(mutex_name_, O_RDWR | O_CREAT | O_EXCL,
                        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    umask(previous_umask);            // reset umask to previous permissions
    if (fd < 0) return nullptr;

    // set size to size of named mutex struct
    if (ftruncate(fd, sizeof(named_mutex_t)) == -1) {
      ::close(fd);
      return nullptr;
    }

    // create mutex attribute
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

#ifdef ECAL_HAS_ROBUST_MUTEX
    if (recoverable_)
      pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
#endif

    // map them into shared memory
    named_mutex_t *mtx = static_cast<named_mutex_t *>(mmap(nullptr, sizeof(named_mutex_t), PROT_READ | PROT_WRITE,
                                                           MAP_SHARED, fd, 0));
    ::close(fd);

    // initialize mutex
    pthread_mutex_init(&mtx->mtx, &attr);

    // return new mutex
    return mtx;
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

  int named_mutex_destroy(const char* mutex_name_)
  {
    // destroy (unlink) shared memory file
    return(::shm_unlink(mutex_name_));
  }

  named_mutex_t* named_mutex_open(const char* mutex_name_)
  {
    // try to open existing shared memory file
    int fd = ::shm_open(mutex_name_, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd < 0) return nullptr;

    // map file content to mutex
    named_mutex_t* mtx = static_cast<named_mutex_t*>(mmap(nullptr, sizeof(named_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    ::close(fd);

    // return opened mutex
    return mtx;
  }

  void named_mutex_close(named_mutex_t* mtx_)
  {
    // unmap condition mutex from shared memory file
    munmap(static_cast<void*>(mtx_), sizeof(named_mutex_t));
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
  CNamedMutexRobustClockLockImpl::CNamedMutexRobustClockLockImpl(const std::string &name_, bool recoverable_) : m_mutex_handle(nullptr), m_named(name_), m_recoverable(false), m_was_recovered(false), m_has_ownership(false)
  {
    if(name_.empty())
      return;

#ifdef ECAL_HAS_ROBUST_MUTEX
    m_recoverable = recoverable_;
#endif

    // build shm file name
    const std::string mutex_name = named_mutex_buildname(m_named);

    // we try to open an existing mutex first
    m_mutex_handle = named_mutex_open(mutex_name.c_str());

    // if we could not open it we create a new one
    if(m_mutex_handle == nullptr)
    {
      m_mutex_handle = named_mutex_create(mutex_name.c_str(), m_recoverable);
      m_has_ownership = true;
    }
  }

  CNamedMutexRobustClockLockImpl::~CNamedMutexRobustClockLockImpl()
  {
    // check mutex handle
    if(m_mutex_handle == nullptr) return;

    // unlock mutex
    named_mutex_unlock(m_mutex_handle);

    // close mutex
    named_mutex_close(m_mutex_handle);

    // clean-up if mutex instance has ownership
    if(m_has_ownership)
      named_mutex_destroy(named_mutex_buildname(m_named).c_str());
  }

  bool CNamedMutexRobustClockLockImpl::IsCreated() const
  {
    return m_mutex_handle != nullptr;
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
    return m_has_ownership;
  }

  void CNamedMutexRobustClockLockImpl::DropOwnership()
  {
    m_has_ownership = false;
  }

  bool CNamedMutexRobustClockLockImpl::Lock(int64_t timeout_)
  {
    // check mutex handle
    if (m_mutex_handle == nullptr)
      return false;

    // reset was recovered state
    m_was_recovered = false;

    // timeout_ < 0 -> wait infinite
    if (timeout_ < 0)
    {
      return(named_mutex_lock(m_mutex_handle, &m_was_recovered));
    }
      // timeout_ == 0 -> check lock state only
    else if (timeout_ == 0)
    {
      return(named_mutex_trylock(m_mutex_handle, &m_was_recovered));
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
      return(named_mutex_timedlock(m_mutex_handle, &abstime, &m_was_recovered));
    }
  }

  void CNamedMutexRobustClockLockImpl::Unlock()
  {
    // check mutex handle
    if(m_mutex_handle == nullptr)
      return;

    // unlock the mutex
    named_mutex_unlock(m_mutex_handle);
  }
}
