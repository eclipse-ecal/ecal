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

#include <ecal/ecal_os.h>
#include <thread>

#ifdef ECAL_OS_WINDOWS

#include "ecal_win_main.h"
#include <string>

typedef void*  MutexT;

namespace eCAL
{
  inline bool CreateMtx(const std::string& name_, MutexT& mutex_handle_)
  {
    std::string mutex_name = name_;
    mutex_name += "_mtx";
    mutex_handle_ = ::CreateMutex(
      nullptr,              // no security descriptor
      false,                // mutex not owned
      mutex_name.c_str());  // object name

    return(mutex_handle_ != nullptr);
  }

  inline bool DestroyMtx(MutexT* mutex_handle_)
  {
    // check mutex handle
    if(mutex_handle_ == nullptr) return(false);

    // release it
    ReleaseMutex(*mutex_handle_);

    // close it
    CloseHandle(*mutex_handle_);

    return(true);
  }

  inline bool LockMtx(MutexT* mutex_handle_, const int timeout_)
  {
    // check mutex handle
    if(mutex_handle_ == nullptr) return(false);

    // wait for access
    return(WaitForSingleObject(*mutex_handle_, timeout_) == WAIT_OBJECT_0);
  }

  inline bool UnlockMtx(MutexT* mutex_handle_)
  {
    // check mutex handle
    if(mutex_handle_ == nullptr) return(false);

    // release it
    ReleaseMutex(*mutex_handle_);

    return(true);
  }
}

#endif /* ECAL_OS_WINDOWS */

#ifdef ECAL_OS_LINUX

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/time.h>

typedef sem_t*  MutexT;

namespace eCAL
{
  inline bool CreateMtx(const std::string& name_, MutexT& mutex_handle_)
  {
    std::string sem_name;
    if(name_[0] != '/')
    {
      sem_name = "/";
    }
    sem_name += name_;

    sem_unlink(sem_name.c_str());

    int previous_umask = umask(000);  // Set umask to nothing, so we can create files with all possible permission bits
    mutex_handle_ = sem_open(sem_name.c_str(), O_CREAT | O_EXCL, (S_IRUSR | S_IWUSR) | (S_IRGRP | S_IWGRP) | (S_IROTH | S_IWOTH), 1);
    umask(previous_umask);            // Reset umask to previous permissions

    if(mutex_handle_ == nullptr)
    {
      mutex_handle_ = sem_open(sem_name.c_str(), 0);
    }

    return(mutex_handle_ != nullptr);
  }

  inline bool DestroyMtx(MutexT* mutex_handle_)
  {
    // check mutex handle
    if(mutex_handle_ == nullptr) return(false);

    sem_post(*mutex_handle_);

    sem_close(*mutex_handle_);

    return(true);
  }

  inline bool LockMtx(MutexT* mutex_handle_, const int timeout_)
  {    
    // check mutex handle
    if(mutex_handle_ == nullptr) return(false);

    if(timeout_ < 0)
    {
      return(sem_wait(*mutex_handle_) == 0);
    }
    else
    {
      auto wait_until = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_);
      
      do
      {
        if (sem_trywait(*mutex_handle_) == 0)
        {
          // The semaphore could be locked!
          return true;
        }
        else if (errno == EAGAIN)
        {
          // The semaphore could not be locked. Let's wait some time and try again.
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
      } while (std::chrono::steady_clock::now() < wait_until);
      
      // When reaching this code, we were never able to lock the semaphore
      return false;
      
      /*
       * Why the code below is commented out:
       * The timeout in the code below is used to compute an absolute time,
       * until the process should wait to lock the semaphore. The time basis
       * used is the CLOCK_REALTIME (gettimeofday), as sem_timedwait() needs
       * an absolute CLOCK_REALTIME time-point.
       * This however causes the whole system to hang, when the system time
       * is set to a time-point in the past, as it may happen when the NPT
       * deamon obtains a new time, or the system time is synchronized with 
       * using PTP.
       */
      
//      struct timespec abstime;
//      struct timeval  tv;
//      gettimeofday(&tv, NULL);
//      abstime.tv_sec  = tv.tv_sec + timeout_ / 1000;
//      abstime.tv_nsec = tv.tv_usec*1000 + (timeout_ % 1000)*1000000;
//      if (abstime.tv_nsec >= 1000000000)
//      {
//        abstime.tv_nsec -= 1000000000;
//        abstime.tv_sec++;
//      }
//      return(sem_timedwait(*mutex_handle_, &abstime) == 0);
    }
  }

  inline bool UnlockMtx(MutexT* mutex_handle_)
  {
    // check mutex handle
    if(mutex_handle_ == nullptr) return(false);

    sem_post(*mutex_handle_);

    return(true);
  }
}

#endif /* ECAL_OS_LINUX */
