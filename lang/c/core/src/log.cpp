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
 * @file   log.cpp
 * @brief  eCAL logging c interface
**/

#include <ecal/ecal.h>
#include <ecal_c/log.h>

#include "common.h"

extern "C"
{
  ECALC_API void eCAL_Logging_Log(enum eCAL_Logging_eLogLevel level_, const char* const msg_)
  {
    // This is dangerous. We need to make sure to keep both enum types in synch.
    eCAL::Logging::Log(static_cast<eCAL::Logging::eLogLevel>(level_), msg_);
  }

  ECALC_API int eCAL_Logging_GetLogging(void* buf_, int buf_len_)
  {
    std::string buf;
    if (eCAL::Logging::GetLogging(buf))
    {
      return(CopyBuffer(buf_, buf_len_, buf));
    }
    return(0);
  }
}
