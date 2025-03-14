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
#include <ecal_c/config/configuration.h>

#include "common.h"
#include "configuration.h"

#include <cassert>

extern "C"
{
  struct eCAL_Configuration_Impl
  {
    eCAL::Configuration handle;
  };

  ECALC_API eCAL_Configuration* eCAL_Configuration_New()
  {
    auto* configuration = reinterpret_cast<eCAL_Configuration*>(std::malloc(sizeof(eCAL_Configuration)));
    if (configuration != NULL)
    {
      configuration->_impl = new eCAL_Configuration_Impl();
      Assign_Configuration(configuration, configuration->_impl->handle);
    }
    return configuration;
  }

  ECALC_API void eCAL_Configuration_Delete(eCAL_Configuration* configuration_)
  {
    assert(configuration_ != NULL);
    delete configuration_->_impl;
    std::free(configuration_);
  }

  ECALC_API void eCAL_Configuration_InitFromConfig(eCAL_Configuration* configuration_)
  {
    assert(configuration_ != NULL);
    configuration_->_impl->handle.InitFromConfig();
    Assign_Configuration(configuration_, configuration_->_impl->handle);
  }

  ECALC_API void eCAL_Configuration_InitFromFile(eCAL_Configuration* configuration_, const char* yaml_path_)
  {
    assert(configuration_ != NULL && yaml_path_ != NULL);
    configuration_->_impl->handle.InitFromFile(yaml_path_);
    Assign_Configuration(configuration_, configuration_->_impl->handle);
  }

  ECALC_API const char* eCAL_Configuration_GetConfigurationFilePath(eCAL_Configuration* configuration_)
  {
    assert(configuration_ != NULL);
    return configuration_->_impl->handle.GetConfigurationFilePath().c_str();
  }
}