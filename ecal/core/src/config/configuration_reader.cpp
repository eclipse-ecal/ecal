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

#include "configuration_reader.h"

#include <yaml-cpp/yaml.h>

namespace YAML
{
  template<>
  struct convert<eCAL::Configuration>
  {
    static Node encode(const eCAL::Configuration& config)
    {
      Node node;

      // Convert here to yaml node, e.g. to safe configuration

      return node;
    }

    static bool decode(const Node& node, eCAL::Configuration& config)
    {
      return true;
    }
  };  
}

namespace eCAL
{
  namespace Config
  {
    eCAL::Configuration parseYaml(std::string& filename_)
    {
      YAML::Node config = YAML::LoadFile(filename_);

      return config.as<eCAL::Configuration>();
    }
  }
}