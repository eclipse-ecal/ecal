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
 * @file   monitoring.cpp
 * @brief  eCAL monitoring c interface
**/

#include <ecal/ecal.h>
#include <ecal_c/monitoring.h>

#include "common.h"

#if ECAL_CORE_MONITORING
extern "C"
{
  ECALC_API int eCAL_Monitoring_GetMonitoring(void* buf_, int buf_len_)
  {
    std::string buf;
    if (eCAL::Monitoring::GetMonitoring(buf))
    {
      return(CopyBuffer(buf_, buf_len_, buf));
    }
    return(0);
  }
}
#endif // ECAL_CORE_MONITORING
