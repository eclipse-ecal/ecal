/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * @file   util.cpp
 * @brief  eCAL utility c interface
**/

#include <ecal/ecal.h>
#include <ecal_c/util.h>

#include "common.h"

#include <cstring>
#include <cassert>

extern "C"
{
  ECALC_API char* eCAL_Util_GeteCALDataDir()
  {
    return Clone_CString(eCAL::Util::GeteCALDataDir().c_str());
  }

  ECALC_API char* eCAL_Util_GeteCALLogDir()
  {
    return Clone_CString(eCAL::Util::GeteCALLogDir().c_str());
  }

  ECALC_API void eCAL_Util_ShutdownProcess(const char* unit_name_)
  {
    assert(unit_name_ != NULL);
    const std::string unit_name = unit_name_;
    eCAL::Util::ShutdownProcess(unit_name);
  }

  ECALC_API void eCAL_Util_ShutdownProcessId(int process_id_)
  {
    eCAL::Util::ShutdownProcess(process_id_);
  }

  ECALC_API void eCAL_Util_ShutdownProcesses()
  {
    eCAL::Util::ShutdownProcesses();
  }
}
