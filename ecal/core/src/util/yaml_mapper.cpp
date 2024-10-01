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

#include "yaml_mapper.h"

#include <iostream>

#include <yaml-cpp/yaml.h>

namespace eCAL
{
  namespace Util
  {
    std::unordered_map<std::string, std::string> YamlMapper::GetMapFromYamlFile(const std::string& file_)
    {
      std::unordered_map<std::string, std::string> map;
      YAML::Node node;
      try {
        node = YAML::LoadFile(file_);
      }
      catch (const YAML::BadFile& e) {
        // std::cerr << "Error reading yaml file: " << file_ << "\n";
        // std::cerr << "Error message: " << e.what() << std::endl;
        return map;
      }

      if(node.IsMap())
      {
        for (const auto& it : node)
        {
          if (it.first.IsScalar() && it.second.IsScalar())
          {
            map[it.first.as<std::string>()] = it.second.as<std::string>();
          }
        }
      }

      return map;
    }
  }
}