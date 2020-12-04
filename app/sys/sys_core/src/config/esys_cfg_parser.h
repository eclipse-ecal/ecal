/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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
 * eCALSys config file parser
**/

#pragma once

#include <string>

#include "esys_cfg.h"

namespace eCAL
{
  namespace Sys
  {
    namespace Config
    {
      /**
      * @brief                        Open eCALSys configuration file
      *
      * @param path                   Path to configuration file
      * @param configuration          Configuration to be filled with data from the file
      * @param exception_msg          Message to display in case of exception
      * @param import                 True if the configuration is imported, false otherwise
      *
      * @return                       True if succeeds, false if it fails
      **/
      bool Open(const std::string& path, CConfiguration& configuration, std::string& exception_msg, bool import = false);

      /**
      * @brief                        Save eCALSys configuration
      *
      * @param path                   Path to save the configuration file to
      * @param configuration          Configuration to be saved
      *
      * @return                       True if succeeds, false if it fails
      **/
      bool Save(const std::string& path, CConfiguration& configuration);
    }  // namespace Config
  }  // namespace Sys
}  // namespace eCAL
