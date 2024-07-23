/* ========================= eCAL LICENSE =================================
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
 * ========================= eCAL LICENSE =================================
*/

/**
 * @brief  Global eCAL configuration interface
**/

#ifndef ECAL_CONFIGURATION_READER
#define ECAL_CONFIGURATION_READER

#include <ecal/config/configuration.h>
#ifndef YAML_CPP_STATIC_DEFINE 
#define YAML_CPP_STATIC_DEFINE 
#endif
#include <yaml-cpp/yaml.h>

#include "configuration_to_yaml.h"

#include <fstream>
#include <stack>
#include <utility>

namespace eCAL
{
  namespace Config
  {    
    // Read a yaml file and convert it to an eCAL configuration
    void YamlFileToConfig(const std::string& filename_, eCAL::Configuration& config_);

    // Read a yaml string and convert it to an eCAL configuration
    void YamlStringToConfig(const std::string& yaml_string_, eCAL::Configuration& config_);

    // Write an eCAL configuration to a yaml file
    bool ConfigToYamlFile(const std::string& file_name_, const eCAL::Configuration& config_);

    // Merge two yaml nodes, priority second parameter
    void MergeYamlNodes(YAML::Node& base, const YAML::Node& other);

    // Merge a yaml file into an existing eCAL configuration
    bool MergeYamlIntoConfiguration(const std::string& file_name_ , eCAL::Configuration& config_);
  }  
}

#endif // ECAL_CONFIGURATION_READER