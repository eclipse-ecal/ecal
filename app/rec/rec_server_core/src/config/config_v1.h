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

#pragma once

#include <string>
#include "config.h"

#include <rec_server_core/rec_server_config.h>

#include <tinyxml2.h>

namespace eCAL
{
  namespace rec_server
  {
    namespace config_v1
    {
      constexpr const int NATIVE_CONFIG_VERSION = 1;

      constexpr const char* ELEMENT_NAME_MAIN_CONFIG                  = "eCALRecGUI";
      constexpr const char* ELEMENT_NAME_ROOT_DIR                     = "rootFolder";
      constexpr const char* ELEMENT_NAME_MEAS_NAME_1                  = "baseNamePrefix";
      constexpr const char* ELEMENT_NAME_MEAS_NAME_2                  = "baseName";
      constexpr const char* ELEMENT_NAME_MEAS_NAME_3                  = "baseNameSuffix";
      constexpr const char* ELEMENT_NAME_DESCRIPTION                  = "description";
      constexpr const char* ELEMENT_NAME_MAX_FILE_SIZE_MIB            = "splitSize";
      constexpr const char* ELEMENT_NAME_PRE_BUFFER_LENGTH_SECS       = "preTrigger";
      constexpr const char* ELEMENT_NAME_LOCAL_HOSTFILTER             = "local";
      constexpr const char* ELEMENT_NAME_TOPIC_LIST                   = "filters";
      constexpr const char* ELEMENT_NAME_TOPIC_LIST_BLACKLIST_ENABLED = "filterTopicsEnabled";
      constexpr const char* ELEMENT_NAME_TOPIC_LIST_WHITELIST         = "source";
      constexpr const char* ELEMENT_NAME_TOPIC_LIST_BLACKLIST         = "excluded";
      constexpr const char* ELEMENT_NAME_TOPIC_LIST_ACTUAL_LIST       = "host";
      constexpr const char* ELEMENT_NAME_TOPIC_LIST_ENTRY             = "topic";
      constexpr const char* ELEMENT_NAME_ENABLED_RECORDERS            = "nodes";
      constexpr const char* ELEMENT_NAME_ENABLED_RECORDER_ENTRY       = "node";

      eCAL::rec_server::RecServerConfig readConfig(const tinyxml2::XMLElement* const main_config_element);
    }
  }
}

