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
 * @file   ecal_core.h
 * @brief  eCAL core function interface
**/

#pragma once

#include <ecal/ecal_init.h>

#include <string>
#include <vector>
#include <utility>

namespace eCAL
{
  /**
   * @brief  Get eCAL version string. 
   *
   * @return  Full eCAL version string. 
  **/
  ECAL_API const char* GetVersionString();

  /**
   * @brief  Get eCAL version date. 
   *
   * @return  Full eCAL version date string. 
  **/
  ECAL_API const char* GetVersionDateString();

  /**
   * @brief  Get eCAL version as separated integer values. 
   *
   * @param [out] major_  The eCAL major version number.
   * @param [out] minor_  The eCAL minor version number.
   * @param [out] patch_  The eCAL patch version number.
   *
   * @return  Zero if succeeded.
  **/
  ECAL_API int GetVersion(int* major_, int* minor_, int* patch_);

  /**
   * @brief Initialize eCAL API.
   *
   * @param argc_        Number of command line arguments. 
   * @param argv_        Array of command line arguments. 
   * @param unit_name_   Defines the name of the eCAL unit. 
   * @param components_  Defines which component to initialize.
   *
   * @return Zero if succeeded, 1 if already initialized, -1 if failed.
  **/
  ECAL_API int Initialize(int argc_ = 0, char **argv_ = nullptr, const char *unit_name_ = nullptr, unsigned int components_ = Init::Default);

  /**
   * @brief Initialize eCAL API.
   *
   * @param args_        Vector of config arguments to overwrite (["arg1", "value1", "arg2", "arg3", "value3" ..]).
   * @param unit_name_   Defines the name of the eCAL unit.
   * @param components_  Defines which component to initialize.
   *
   * @return Zero if succeeded, 1 if already initialized, -1 if failed.
  **/
  ECAL_API int Initialize(std::vector<std::string> args_, const char *unit_name_ = nullptr, unsigned int components_ = Init::Default);

  /**
   * @brief Finalize eCAL API.
   *
   * @param components_  Defines which component to finalize (not yet supported).
   *
   * @return Zero if succeeded, 1 if already finalized, -1 if failed.
  **/
  ECAL_API int Finalize(unsigned int components_ = Init::Default);

  /**
   * @brief Check eCAL initialize state.
   *
   * @param component_  Check specific component or 0 for general state of eCAL core.
   *
   * @return 1 if eCAL is initialized.
  **/
  ECAL_API int IsInitialized(unsigned int component_ = 0);

  /**
   * @brief  Set/change the unit name of current module.
   *
   * @param unit_name_  Defines the name of the eCAL unit. 
   *
   * @return  Zero if succeeded.
  **/
  ECAL_API int SetUnitName(const char *unit_name_);

  /**
   * @brief Return the eCAL process state.
   *
   * @return  True if eCAL is in proper state. 
  **/
  ECAL_API bool Ok();
}
