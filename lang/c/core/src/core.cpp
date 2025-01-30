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
 * @file   core.cpp
 * @brief  eCAL core function c interface
**/

#include <ecal/ecal.h>
#include <ecal_c/core.h>

extern "C"
{
  ECALC_API const char* eCAL_GetVersionString()
  {
    return ECAL_VERSION;
  }

  ECALC_API const char* eCAL_GetVersionDateString()
  {
    return ECAL_DATE;
  }

  ECALC_API eCAL_SVersion eCAL_GetVersion()
  {
    return eCAL_SVersion{ eCAL::GetVersion().major, eCAL::GetVersion().minor, eCAL::GetVersion().patch };
  }

  ECALC_API int eCAL_Initialize(const char* unit_name_, unsigned int components_)
  {
    const std::string unit_name = (unit_name_ != nullptr) ? std::string(unit_name_) : std::string("");
    return static_cast<int>(!eCAL::Initialize(unit_name, components_));
  }

  ECALC_API int eCAL_Finalize()
  {
    return static_cast<int>(!eCAL::Finalize());
  }

  ECALC_API int eCAL_IsInitialized()
  {
    return static_cast<int>(eCAL::IsInitialized());
  }

  ECALC_API int eCAL_Ok()
  {
    return static_cast<int>(eCAL::Ok());
  }

  ECALC_API void eCAL_FreeMem(void* mem_)
  {
    free(mem_); // NOLINT(*-owning-memory, *-no-malloc)
  }
}
