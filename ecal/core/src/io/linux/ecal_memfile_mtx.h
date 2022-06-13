/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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
 * @brief  eCAL memory file mutex
**/

#pragma once

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
  // this mutex is used for protecting the locked var while being changed
  pthread_mutex_t  mtx;
  // this mutex is by the process during the locking,
  // so that by using the mutex state, we can know if the process is crashed.
  pthread_mutex_t  prc_mtx;
  pthread_cond_t   cvar;
  uint8_t          locked;
};
typedef struct named_mutex  named_mutex_t;

namespace
{
  named_mutex_t *named_mutex_create(const char* mutex_name_, bool robust_mutex_ = false)
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
    pthread_mutexattr_t shmtx1, shmtx2;
    pthread_mutexattr_init(&shmtx1);
    pthread_mutexattr_setpshared(&shmtx1, PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_init(&shmtx2);
    pthread_mutexattr_setpshared(&shmtx2, PTHREAD_PROCESS_SHARED);

    if (robust_mutex_)
    {
      // this should be a robust mutex:
#ifndef ECAL_OS_MACOS
      pthread_mutexattr_setrobust(&shmtx1, PTHREAD_MUTEX_ROBUST);
      pthread_mutexattr_setrobust(&shmtx2, PTHREAD_MUTEX_ROBUST);
#endif // ECAL_OS_MACOS
    }

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
    pthread_mutex_init(&mtx->mtx, &shmtx1);
    pthread_mutex_init(&mtx->prc_mtx, &shmtx2);
    pthread_cond_init(&mtx->cvar, &shattr);

    // start with unlocked mutex
    mtx->locked = 0;

    // return new mutex
    return mtx;
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

  bool named_mutex_lock(named_mutex_t* mtx_, struct timespec* ts_, bool* is_process_crashed_)
  {
    // initialize output param:
    if(nullptr != is_process_crashed_)
      *is_process_crashed_ = false;

    // lock condition mutex
    int lock_state = pthread_mutex_lock(&mtx_->mtx);
    if(EOWNERDEAD == lock_state)
    {
      // mutex was locked by another process, but its owner process is crashed.
      // then this process now is the owner of that mutex.
      // firstly, make the mutex consistent, to indicate that the shared area that is guarded by the mutex is consistent.
#ifndef ECAL_OS_MACOS
      pthread_mutex_consistent(&mtx_->mtx);
#endif // ECAL_OS_MACOS

      // report process crash:
      if(nullptr != is_process_crashed_)
        *is_process_crashed_ = true;
    }

    // state is not locked ?, fine !
    if (mtx_->locked == 0)
    {
      // lock the mutex that should be locked while the 'locked' var is enabled
      pthread_mutex_lock(&mtx_->prc_mtx);
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
          // timeout:
          if(0 != ret)
          {
            // make sure that the process that enabled the locked var is not crashed:
            int prc_mtx_lk_state = pthread_mutex_trylock(&mtx_->prc_mtx);
            if(EOWNERDEAD == prc_mtx_lk_state)
            {
              // the process that was enabling the locked var is crashed.
              // make the mutex consistent:
        #ifndef ECAL_OS_MACOS
              pthread_mutex_consistent(&mtx_->prc_mtx);
        #endif // ECAL_OS_MACOS

              // set ret status as success because this process/thread should be the owner now:
              ret = 0;
              // report process crash:
              if(nullptr != is_process_crashed_)
                *is_process_crashed_ = true;
            }
          }
        }
        // blocking wait for unlock signal
        else
        {
          ret = pthread_cond_wait(&mtx_->cvar, &mtx_->mtx);
          if(ret == 0)
          {
            // then we are going to enable the locked var,
            // then lock the proc_mtx here:
            pthread_mutex_lock(&mtx_->prc_mtx);
          }
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

  bool named_mutex_trylock(named_mutex_t* mtx_, bool* is_process_crashed_)
  {
    // initialize output param:
    if(nullptr != is_process_crashed_)
        *is_process_crashed_ = false;

    bool locked(false);
    // lock condition mutex
    int lock_state = pthread_mutex_lock(&mtx_->mtx);
    if(EOWNERDEAD == lock_state)
    {
      // mutex was locked by another process, but its owner process is crashed.
      // then this process now is the owner of that mutex.
      // firstly, make the mutex consistent, to indicate that the shared area that is guarded by the mutex is consistent.
#ifndef ECAL_OS_MACOS
      pthread_mutex_consistent(&mtx_->mtx);
#endif // ECAL_OS_MACOS
      // report process crash:
      if(nullptr != is_process_crashed_)
        *is_process_crashed_ = true;
    }

    // check state
    if (mtx_->locked == 0)
    {
      // lock the mutex that should be locked while the 'locked' var is enabled
      pthread_mutex_lock(&mtx_->prc_mtx);
      // set state to locked
      mtx_->locked = 1;
      locked = true;
    }
    // unlock condition mutex
    pthread_mutex_unlock(&mtx_->mtx);
    // return success
    return locked;
  }

  void named_mutex_unlock(named_mutex_t* mtx_, bool* is_process_crashed_)
  {
    // initialize output param:
    if(nullptr != is_process_crashed_)
      *is_process_crashed_ = false;

    // lock condition mutex
    int lock_state = pthread_mutex_lock(&mtx_->mtx);
    if(EOWNERDEAD == lock_state)
    {
      // mutex was locked by another process, but its owner process is crashed.
      // then this process now is the owner of that mutex.
      // firstly, make the mutex consistent, to indicate that the shared area that is guarded by the mutex is consistent.
#ifndef ECAL_OS_MACOS
      pthread_mutex_consistent(&mtx_->mtx);
#endif // ECAL_OS_MACOS
      // report process crash:
      if(nullptr != is_process_crashed_)
        *is_process_crashed_ = true;
    }

    // state is locked ?
    if (mtx_->locked == 1)
    {
      // set state to unlocked
      mtx_->locked = 0;
      // unlock the mutex that should be locked while the 'locked' var is enabled
      pthread_mutex_unlock(&mtx_->prc_mtx);
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

typedef named_mutex_t*  MutexT;

namespace eCAL
{
  inline bool CreateMtx(const std::string& name_, MutexT& mutex_handle_, bool robust_mutex_ = false)
  {
    if(name_.empty()) return(false);

    // build shm file name
    std::string mutex_name = named_mutex_buildname(name_);

    // we try to open an existing mutex first
    mutex_handle_ = named_mutex_open(mutex_name.c_str());

    // if we could not open it we create a new one
    if(mutex_handle_ == nullptr)
    {
      mutex_handle_ = named_mutex_create(mutex_name.c_str(), robust_mutex_);
    }

    return(mutex_handle_ != nullptr);
  }

  inline bool DestroyMtx(MutexT* mutex_handle_, bool* is_process_crashed_)
  {
    // check mutex handle
    if(mutex_handle_ == nullptr) return(false);

    // unlock mutex
    named_mutex_unlock(*mutex_handle_, is_process_crashed_);

    // close mutex
    named_mutex_close(*mutex_handle_);

    return(true);
  }

  inline bool CleanupMtx(const std::string& name_)
  {
    // build shm file name
    std::string mutex_name = named_mutex_buildname(name_);

    // destroy aka unlink shm file from process
    return(named_mutex_destroy(mutex_name.c_str()) == 0);
  }

  inline bool LockMtx(MutexT* mutex_handle_, const int timeout_, bool* is_process_crashed_)
  {
    // check mutex handle
    if (mutex_handle_ == nullptr) return(false);

    // timeout_ < 0 -> wait infinite
    if (timeout_ < 0)
    {
      return(named_mutex_lock(*mutex_handle_, nullptr, is_process_crashed_));
    }
    // timeout_ == 0 -> check lock state only
    else if (timeout_ == 0)
    {
      return(named_mutex_trylock(*mutex_handle_, is_process_crashed_));
    }
    // timeout_ > 0 -> wait timeout_ ms
    else
    {
      struct timespec abstime;
      clock_gettime(CLOCK_MONOTONIC, &abstime);

      abstime.tv_sec = abstime.tv_sec + timeout_ / 1000;
      abstime.tv_nsec = abstime.tv_nsec + (timeout_ % 1000) * 1000000;
      while (abstime.tv_nsec >= 1000000000)
      {
        abstime.tv_nsec -= 1000000000;
        abstime.tv_sec++;
      }
      return(named_mutex_lock(*mutex_handle_, &abstime, is_process_crashed_));
    }
  }

  inline bool UnlockMtx(MutexT* mutex_handle_, bool* is_process_crashed_)
  {
    // check mutex handle
    if(mutex_handle_ == nullptr) return(false);

    // unlock the mutex
    named_mutex_unlock(*mutex_handle_, is_process_crashed_);

    return(true);
  }
}
