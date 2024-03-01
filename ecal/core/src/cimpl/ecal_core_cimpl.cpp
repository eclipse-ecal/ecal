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
 * @file   ecal_core_cimpl.cpp
 * @brief  eCAL core function c interface
**/

#include <ecal/ecal.h>
#include <ecal/cimpl/ecal_core_cimpl.h>

extern "C"
{
  ECALC_API const char* eCAL_GetVersionString()
  {
    return(ECAL_VERSION);
  }

  ECALC_API const char* eCAL_GetVersionDateString()
  {
    return(ECAL_DATE);
  }

  ECALC_API int eCAL_GetVersion(int* major_, int* minor_, int* patch_)
  {
    if ((major_ == nullptr) || (minor_ == nullptr) || (patch_ == nullptr)) return(-1);
    *major_ = ECAL_VERSION_MAJOR;
    *minor_ = ECAL_VERSION_MINOR;
    *patch_ = ECAL_VERSION_PATCH;
    return(0);
  }

  ECALC_API int eCAL_Initialize(int argc_, char** argv_, const char* unit_name_, unsigned int components_)
  {
    return(eCAL::Initialize(argc_, argv_, unit_name_, components_));
  }

  ECALC_API int eCAL_SetUnitName(const char* unit_name_)
  {
    return(eCAL::SetUnitName(unit_name_));
  }

  ECALC_API int eCAL_Finalize(unsigned int components_)
  {
    return(eCAL::Finalize(components_));
  }

  ECALC_API int eCAL_IsInitialized(unsigned int component_)
  {
    return(eCAL::IsInitialized(component_));
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
