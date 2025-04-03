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

#include <ecal/os.h>

#include "ecal_named_mutex_impl.h"

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
  pthread_cond_t   cvar;
  uint8_t          locked;
};
typedef struct named_mutex named_mutex_t;

namespace
{
  named_mutex_t* named_mutex_create(const char* mutex_name_)
  {
    // create shared memory file
    int previous_umask = umask(000);  // set umask to nothing, so we can create files with all possible permission bits
    int fd = ::shm_open(mutex_name_, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    umask(previous_umask);            // reset umask to previous permissions
    if (fd < 0) return nullptr;

    // set size to size of named mutex struct
    if(ftruncate(fd, sizeof(named_mutex_t)) == -1)
    {
      ::close(fd);
      return nullptr;
    }

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


    // map them into shared memory
    named_mutex_t* mtx = static_cast<named_mutex_t*>(mmap(nullptr, sizeof(named_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    ::close(fd);

    // initialize mutex and condition
    pthread_mutex_init(&mtx->mtx, &shmtx);
    pthread_cond_init(&mtx->cvar, &shattr);

    // start with unlocked mutex
    mtx->locked = 0;

    // return new mutex
    return mtx;
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

  CNamedMutexImpl::CNamedMutexImpl(const std::string &name_, bool /*recoverable_*/) : m_mutex_handle(nullptr), m_named(name_), m_has_ownership(false)
  {
    if(name_.empty())
      return;

    // build shm file name
    const std::string mutex_name = named_mutex_buildname(m_named);

    // we try to open an existing mutex first
    m_mutex_handle = named_mutex_open(mutex_name.c_str());

    // if we could not open it we create a new one
    if(m_mutex_handle == nullptr)
    {
      m_mutex_handle = named_mutex_create(mutex_name.c_str());
      if(m_mutex_handle)
        m_has_ownership = true;
    }
  }

  CNamedMutexImpl::~CNamedMutexImpl()
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

  bool CNamedMutexImpl::IsCreated() const
  {
    return m_mutex_handle != nullptr;
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
    return m_has_ownership;
  }

  void CNamedMutexImpl::DropOwnership()
  {
    m_has_ownership = false;
  }

  bool CNamedMutexImpl::Lock(int64_t timeout_)
  {
    // check mutex handle
    if (m_mutex_handle == nullptr)
      return false;

    // timeout_ < 0 -> wait infinite
    if (timeout_ < 0)
    {
      return(named_mutex_lock(m_mutex_handle, nullptr));
    }
      // timeout_ == 0 -> check lock state only
    else if (timeout_ == 0)
    {
      return(named_mutex_trylock(m_mutex_handle));
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
      return(named_mutex_lock(m_mutex_handle, &abstime));
    }
  }

  void CNamedMutexImpl::Unlock()
  {
    // check mutex handle
    if(m_mutex_handle == nullptr)
      return;

    // unlock the mutex
    named_mutex_unlock(m_mutex_handle);
  }
}
