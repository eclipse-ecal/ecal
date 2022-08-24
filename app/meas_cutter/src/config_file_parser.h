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
#include <iostream>
#include <ecal_utils/filesystem.h>

#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>

#include "utils.h"

class ConfigFileParser
{
public:
  ConfigFileParser(const std::string& configuration_file_path);

  bool isConfigFileValid();
  void parseTrimOperations(eCALMeasCutterUtils::TrimOperation& trim_operation);
  void parseChannelsOperations(std::list<std::string>& channel_list, std::list<std::string>& channel_list_regex, eCALMeasCutterUtils::ChannelOperationType);
  void parseConversionParameters(std::string& basename, size_t& max_size_per_file, bool& is_base_name_mentioned_in_config, bool& is_max_file_size_mentioned_in_config);

private:
  std::string _configuration_file_path;
  YAML::Node  _loaded_file;
};

