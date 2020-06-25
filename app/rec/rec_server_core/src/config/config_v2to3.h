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
#include "rec_server_impl.h"
#include "config.h"

#include <tinyxml2.h>

namespace eCAL
{
  namespace rec_server
  {
    namespace config_v2to3
    {
      constexpr const int NATIVE_CONFIG_VERSION = 3;

      constexpr const char* ELEMENT_NAME_MAIN_CONFIG                                = "ecalRecServerConfig";
      constexpr const char* ATTRIBUTE_NAME_MAIN_CONFIG_VERSION                      = "config_version";
      constexpr const char* ELEMENT_NAME_ROOT_DIR                                   = "rootDirectory";
      constexpr const char* ELEMENT_NAME_MEAS_NAME                                  = "measurementName";
      constexpr const char* ELEMENT_NAME_MAX_FILE_SIZE_MIB                          = "maxFileSizeMib";
      constexpr const char* ELEMENT_NAME_DESCRIPTION                                = "description";
      constexpr const char* ELEMENT_NAME_ENABLED_RECORDERS                          = "recorders";
      constexpr const char* ELEMENT_NAME_ENABLED_RECORDER_ENTRY                     = "client";
      constexpr const char* ELEMENT_NAME_ENABLED_RECORDER_ENTRY_NAME                = "name";
      constexpr const char* ELEMENT_NAME_ENABLED_RECORDER_ENTRY_HOSTFILTER          = "hostFilter";
      constexpr const char* ELEMENT_NAME_ENABLED_RECORDER_ENTRY_HOSTFILTER_ENTRY    = "host";
      constexpr const char* ELEMENT_NAME_ENABLED_RECORDER_ENTRY_ENABLED_ADDONS      = "enabledAddons";
      constexpr const char* ELEMENT_NAME_ENABLED_RECORDER_ENTRY_ENABLED_ADDON_ENTRY = "addon";
      constexpr const char* ELEMENT_NAME_PRE_BUFFER_ENABLED                         = "preBufferEnabled";
      constexpr const char* ELEMENT_NAME_PRE_BUFFER_LENGTH_SECS                     = "preBufferLengthSecs";
      constexpr const char* ELEMENT_NAME_BUILT_IN_RECORDER_ENABLED                  = "useBuiltInRecorder";
      constexpr const char* ELEMENT_NAME_TOPIC_LIST                                 = "topicList";
      constexpr const char* ATTRIBUTE_NAME_TOPIC_LIST_TYPE                          = "type";
      constexpr const char* ELEMENT_NAME_TOPIC_LIST_ENTRY                           = "topic";

      constexpr const char* ELEMENT_NAME_UPLOAD_CONFIG                              = "uploadConfig";             // Added in v3
      constexpr const char* ATTRIBUTE_NAME_UPLOAD_PROTOCOL                          = "protocol";                 // Added in v3
      constexpr const char* ELEMENT_NAME_UPLOAD_CONFIG_HOST                         = "host";                     // Added in v3
      constexpr const char* ELEMENT_NAME_UPLOAD_CONFIG_PORT                         = "port";                     // Added in v3
      constexpr const char* ELEMENT_NAME_UPLOAD_CONFIG_USER                         = "username";                 // Added in v3
      constexpr const char* ELEMENT_NAME_UPLOAD_CONFIG_PASSWORD                     = "password";                 // Added in v3
      constexpr const char* ELEMENT_NAME_UPLOAD_CONFIG_ROOT_DIR                     = "rootDirectory";            // Added in v3
      constexpr const char* ELEMENT_NAME_UPLOAD_CONFIG_DELETE_AFTER_UPLOAD          = "deleteAfterUpload";        // Added in v3


      bool writeConfigFile(const eCAL::rec_server::RecServerImpl& rec_server, const std::string& path);

      eCAL::rec_server::RecServerConfig readConfig(const tinyxml2::XMLElement* const main_config_element);
    }
  }
}

