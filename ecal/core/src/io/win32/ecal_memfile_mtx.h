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

#include "ecal_win_main.h"

typedef void*  MutexT;

namespace eCAL
{
  inline bool CreateMtx(const std::string& name_, MutexT& mutex_handle_, bool robust_mutex_ = false)
  {
    // not needed parameter
    robust_mutex_ = false;
    std::string mutex_name = name_;
    mutex_name += "_mtx";
    mutex_handle_ = ::CreateMutex(
      nullptr,              // no security descriptor
      false,                // mutex not owned
      mutex_name.c_str());  // object name

    return(mutex_handle_ != nullptr);
  }

  inline bool DestroyMtx(MutexT* mutex_handle_, bool* is_process_crashed_ = nullptr)
  {
    // check mutex handle
    if(mutex_handle_ == nullptr) return(false);

    // initialize output param:
    if (nullptr != is_process_crashed_)
      *is_process_crashed_ = false;

    // release it
    ReleaseMutex(*mutex_handle_);

    // close it
    CloseHandle(*mutex_handle_);

    return(true);
  }

  inline bool CleanupMtx(const std::string& /*name_*/)
  {
    // no need for windows platform
    return(true);
  }

  inline bool LockMtx(MutexT* mutex_handle_, const int timeout_, bool* is_process_crashed_)
  {
    // check mutex handle
    if(mutex_handle_ == nullptr) return(false);

    // initialize output param:
    if (nullptr != is_process_crashed_)
      *is_process_crashed_ = false;

    // wait for access
    auto wait_result = WaitForSingleObject(*mutex_handle_, timeout_);
    if (WAIT_ABANDONED == wait_result)
    {
      // the process/thread that was locking the mutex was crashed while locking it.
      if (nullptr != is_process_crashed_)
        *is_process_crashed_ = true;
      return true;
    }
    else
      return(wait_result == WAIT_OBJECT_0);
  }

  inline bool UnlockMtx(MutexT* mutex_handle_, bool* is_process_crashed_ = nullptr)
  {
    // check mutex handle
    if(mutex_handle_ == nullptr) return(false);

    // initialize output param:
    if (nullptr != is_process_crashed_)
      *is_process_crashed_ = false;

    // release it
    ReleaseMutex(*mutex_handle_);

    return(true);
  }
}
