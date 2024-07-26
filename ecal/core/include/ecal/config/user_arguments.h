/* =========================== LICENSE =================================
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
 * =========================== LICENSE =================================
 */

/**
 * @file   user_arguments.h
 * @brief  Arguments given by the user via command line
**/

#pragma once

#include <string>
#include <vector>
#include <map>

namespace eCAL
{
  namespace Cli
  {
    using ConfigKey2DMap = std::map<std::string, std::map<std::string, std::string>>; //!< Config key storage: Map[Section][Option] = Value

    struct Configuration
    {
      std::string              user_yaml{};         //!< The used eCAL yaml file (Default: "")
      bool                     dump_config{};       //!< If specified, output configuration via standart output (Default: false)
    };
  }
}