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
#include <ecal_c/init.h>

#include "common.h"
#include "configuration.h"

#include <cassert>
#include <map>

namespace
{
  unsigned int Convert_Components(unsigned int components_c_)
  {
    unsigned int components{ 0 };
    static const std::map<unsigned int, unsigned int> component_map
    {
      {eCAL_Init_None, eCAL::Init::None},
      {eCAL_Init_Logging, eCAL::Init::Logging},
      {eCAL_Init_Monitoring, eCAL::Init::Monitoring},
      {eCAL_Init_Publisher, eCAL::Init::Publisher},
      {eCAL_Init_Service, eCAL::Init::Service},
      {eCAL_Init_Subscriber, eCAL::Init::Subscriber},
      {eCAL_Init_TimeSync, eCAL::Init::TimeSync}
    };

    unsigned int bit_mask = 1 << 0;
    for (std::size_t i = 0; i < sizeof(unsigned int) * 8; ++i)
    {
      components |= component_map.at(bit_mask & components_c_);
      bit_mask <<= 1;
    }

    return components;
  }
}

extern "C"
{
  ECALC_API const char* eCAL_GetVersionString()
  {
    return eCAL::GetVersionString().c_str();
  }

  ECALC_API const char* eCAL_GetVersionDateString()
  {
    return eCAL::GetVersionDateString().c_str();
  }

  ECALC_API struct eCAL_SVersion eCAL_GetVersion()
  {
    return eCAL_SVersion{ eCAL::GetVersion().major, eCAL::GetVersion().minor, eCAL::GetVersion().patch };
  }

  ECALC_API int eCAL_Initialize(const char* unit_name_, const unsigned int* components_, const eCAL_Configuration* config_)
  {
    const std::string unit_name = (unit_name_ != NULL) ? std::string(unit_name_) : std::string("");
    const unsigned int components = (components_ != NULL) ? Convert_Components(*components_) : Convert_Components(eCAL_Init_Default);
    if (config_ != NULL)
    {
      eCAL::Configuration config;
      Assign_Configuration(config, config_);
      return static_cast<int>(!eCAL::Initialize(config, unit_name, components));
    }
    else
      return static_cast<int>(!eCAL::Initialize(unit_name, components));
  }

  ECALC_API int eCAL_Finalize()
  {
    return static_cast<int>(!eCAL::Finalize());
  }

  ECALC_API int eCAL_IsInitialized()
  {
    return static_cast<int>(eCAL::IsInitialized());
  }

  ECALC_API int eCAL_IsComponentInitialized(unsigned int components_)
  {
    return static_cast<int>(eCAL::IsInitialized(Convert_Components(components_)));
  }

  ECALC_API int eCAL_Ok()
  {
    return static_cast<int>(eCAL::Ok());
  }
}
