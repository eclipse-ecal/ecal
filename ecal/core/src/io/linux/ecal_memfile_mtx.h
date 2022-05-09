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

struct alignas(8) named_mutex
{
  pthread_rwlock_t rwlock;
};
typedef struct named_mutex  named_mutex_t;

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

    //create rwlock
    pthread_rwlockattr_t shrwlock;
    pthread_rwlockattr_init(&shrwlock);
    pthread_rwlockattr_setpshared(&shrwlock, PTHREAD_PROCESS_SHARED);
    pthread_rwlockattr_setkind_np(&shrwlock, PTHREAD_RWLOCK_PREFER_READER_NP);

    // map them into shared memory
    named_mutex_t* mtx = static_cast<named_mutex_t*>(mmap(nullptr, sizeof(named_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    ::close(fd);

    // initialize rwlock
    pthread_rwlock_init(&mtx->rwlock, &shrwlock);

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

  void named_mutex_close(named_mutex_t* mtx_) {
    // unmap condition mutex from shared memory file
    munmap(static_cast<void *>(mtx_), sizeof(named_mutex_t));
  }

  bool named_mutex_lock(named_mutex_t* mtx_, struct timespec* ts_)
  {
    if (ts_)
      return (pthread_rwlock_clockwrlock(&mtx_->rwlock, CLOCK_MONOTONIC, ts_) == 0);
    pthread_rwlock_wrlock(&mtx_->rwlock);

    return true;
  }

  bool named_mutex_trylock(named_mutex_t* mtx_)
  {
    return (pthread_rwlock_trywrlock(&mtx_->rwlock) == 0);
  }

  void named_mutex_unlock(named_mutex_t* mtx_)
  {
    pthread_rwlock_unlock(&mtx_->rwlock);
  }

  bool named_mutex_lock_shared(named_mutex_t* mtx_, struct timespec* ts_)
  {
    if (ts_)
      return (pthread_rwlock_clockrdlock(&mtx_->rwlock, CLOCK_MONOTONIC, ts_) == 0);
    pthread_rwlock_rdlock(&mtx_->rwlock);
    return true;
  }

  void named_mutex_unlock_shared(named_mutex_t* mtx_)
  {
    pthread_rwlock_unlock(&mtx_->rwlock);
  }

  bool named_mutex_trylock_shared(named_mutex_t* mtx_)
  {
    return (pthread_rwlock_tryrdlock(&mtx_->rwlock) == 0);
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
  inline bool CreateMtx(const std::string& name_, MutexT& mutex_handle_)
  {
    if(name_.empty()) return(false);

    // build shm file name
    std::string mutex_name = named_mutex_buildname(name_);

    // we try to open an existing mutex first
    mutex_handle_ = named_mutex_open(mutex_name.c_str());

    // if we could not open it we create a new one
    if(mutex_handle_ == nullptr)
    {
      mutex_handle_ = named_mutex_create(mutex_name.c_str());
    }

    return(mutex_handle_ != nullptr);
  }

  inline bool DestroyMtx(MutexT* mutex_handle_)
  {
    // check mutex handle
    if(mutex_handle_ == nullptr) return(false);

    // unlock mutex
    named_mutex_unlock(*mutex_handle_);

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

  inline bool LockMtx(MutexT* mutex_handle_, const int timeout_)
  {
    // check mutex handle
    if (mutex_handle_ == nullptr) return(false);

    // timeout_ < 0 -> wait infinite
    if (timeout_ < 0)
    {
      return(named_mutex_lock(*mutex_handle_, nullptr));
    }
    // timeout_ == 0 -> check lock state only
    else if (timeout_ == 0)
    {
      return(named_mutex_trylock(*mutex_handle_));
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
      return(named_mutex_lock(*mutex_handle_, &abstime));
    }
  }

  inline bool UnlockMtx(MutexT* mutex_handle_)
  {
    // check mutex handle
    if(mutex_handle_ == nullptr) return(false);

    // unlock the mutex
    named_mutex_unlock(*mutex_handle_);

    return(true);
  }

  inline bool LockSharedMtx(MutexT* mutex_handle_, const int timeout_)
  {
    // check mutex handle
    if (mutex_handle_ == nullptr) return(false);

    // timeout_ < 0 -> wait infinite
    if (timeout_ < 0)
    {
      return(named_mutex_lock_shared(*mutex_handle_, nullptr));
    }
      // timeout_ == 0 -> check lock state only
    else if (timeout_ == 0)
    {
      return(named_mutex_trylock_shared(*mutex_handle_));
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
      return(named_mutex_lock_shared(*mutex_handle_, &abstime));
    }
  }

  inline bool UnlockSharedMtx(MutexT* mutex_handle_)
  {
    // check mutex handle
    if(mutex_handle_ == nullptr) return(false);

    // unlock the mutex
    named_mutex_unlock_shared(*mutex_handle_);

    return(true);
  }
}
