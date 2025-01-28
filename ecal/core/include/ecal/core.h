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
 * @file   core.h
 * @brief  eCAL core function interface
**/

#pragma once

#include <ecal/init.h>

#include <string>
#include <vector>
#include <utility>

#include <ecal/os.h>
#include <ecal/config/configuration.h>
#include <ecal/types.h>

namespace eCAL
{
  /**
   * @brief  Get eCAL version string. 
   *
   * @return  Full eCAL version string. 
  **/
  ECAL_API std::string GetVersionString();

  /**
   * @brief  Get eCAL version date. 
   *
   * @return  Full eCAL version date string. 
  **/
  ECAL_API std::string GetVersionDateString();

  /**
   * @brief  Get eCAL version as separated integer values.
   *
   * @return struct SVersion that contains major, minor and patch value.
  **/
  ECAL_API SVersion GetVersion();

  /**
   * @brief Initialize eCAL API.
   *
   * @param unit_name_   Defines the name of the eCAL unit.
   * @param components_  Defines which component to initialize.
   *
   * @return True.
  **/
  ECAL_API bool Initialize(const std::string& unit_name_ = "", unsigned int components_ = Init::Default);

  /**
   * @brief Initialize eCAL API.
   *
   * @param config_      User defined configuration object.
   * @param unit_name_   Defines the name of the eCAL unit.
   * @param components_  Defines which component to initialize.     
   * 
   * @return True if succeeded.
  **/
  ECAL_API bool Initialize(eCAL::Configuration& config_, const std::string& unit_name_ = "", unsigned int components_ = Init::Default);

  /**
   * @brief Finalize eCAL API.
   *
   * @return True if succeeded.
  **/
  ECAL_API bool Finalize();

  /**
   * @brief Check eCAL initialize state.
   *
   * @return True if eCAL is initialized.
  **/
  ECAL_API bool IsInitialized();

  /**
   * @brief Check initialization state for a specific component.
   *
   * @param component_  Specific component to check
   *
   * @return True if eCAL component is initialized.
  **/
  ECAL_API bool IsInitialized(unsigned int component_);

  /**
   * @brief Return the eCAL process state.
   *
   * @return  True if eCAL is in proper state. 
  **/
  ECAL_API bool Ok();
}

