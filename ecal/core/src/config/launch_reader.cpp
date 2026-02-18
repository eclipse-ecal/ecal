/* ========================= eCAL LICENSE =================================
 *
 * Copyright 2026 AUMOVIO and subsidiaries. All rights reserved.
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

#include "launch_reader.h"

#include "util/yaml_functions.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace
{
  bool checkIfFileExists(const std::string& path_to_file_)
  {
    if (std::filesystem::exists(path_to_file_) && std::filesystem::is_regular_file(path_to_file_))
      return true;

    return false;
  }

  // some forward declarations
  void mergeNodesOfSameType(YAML::Node& base_sequence_, const YAML::Node& other_);

  // merge two sequences
  // second node will be merged into first node
  void mergeSequences(YAML::Node& base_sequence_, const YAML::Node& other_)
  {
    if (!base_sequence_.IsSequence() && !other_.IsSequence()) return;

    for (const auto& part : other_)
    {
      switch (part.Type())
      {
        case YAML::NodeType::Map:
          // if there is a map, we simply add it do the sequence
        case YAML::NodeType::Sequence:
          // let's simply add sequences to the sequence in the rare case of nested sequences
          // is there a usecase?
          base_sequence_.push_back(part);          
          break;

          // just add a scalar that is new
        case YAML::NodeType::Scalar:
          {
            bool item_found = false;
            for (const auto& item : base_sequence_)
            {
              if (item.IsScalar() && part.Scalar() == item.Scalar()) 
              {
                item_found = true;
                break;
              }
            }

            if (!item_found) base_sequence_.push_back(part);
          }
          break;

        default:
          break;
      }

    }
  }

  // merge two maps
  // second node will be merged into first node
  // in case both maps have the same key:
  //    scalar of first node will be kept
  //    complex nodes will be merged
  void mergeMaps(YAML::Node& base_map_, const YAML::Node& other_)
  {
    if (!base_map_.IsMap() && !other_.IsMap()) return;

    for (auto it = other_.begin(); it != other_.end(); it++)
    {      
      auto key = it->first.as<std::string>();
      // take the new values that were not there in the base map
      if (!base_map_[key]) base_map_[key] = it->second;
      // we don't want to overwrite the value from the base map
      else if (base_map_[key].IsScalar()) continue;
      // means value is either a sequence, map or null
      else
      {
        auto base_value = base_map_[key];
        mergeNodesOfSameType(base_value, it->second);
      }
    }
  }

  // merge two nodes of the same type
  void mergeNodesOfSameType(YAML::Node& base_node_, const YAML::Node& other_)
  {
    // return when the node types are different or one is empty
    if (other_.Type() == YAML::NodeType::Null || base_node_.Type() != other_.Type()) return;

    switch (other_.Type())
    {
      case YAML::NodeType::Map:
        mergeMaps(base_node_, other_);
        break;
      
      case YAML::NodeType::Sequence:
        mergeSequences(base_node_, other_);
        break;

      case YAML::NodeType::Scalar:
        // do nothing, as we want to keep the base node value
        break;
      
      default:
        break;
    }
  }

  YAML::Node ReadLaunchYamlImpl(const std::string& path_, std::vector<std::string>& include_yaml_list_);

  // Node should be a sequence
  std::vector<YAML::Node> handleBaseIncludes(YAML::Node& base_includes_, std::vector<std::string>& include_yaml_list_)
  {
    if (!base_includes_.IsSequence()) return {};

    std::vector<YAML::Node> return_vector;
    for (auto it = base_includes_.begin(); it != base_includes_.end(); it++)
    {
      auto file_name = it->as<std::string>();
      if (std::find(include_yaml_list_.begin(), include_yaml_list_.end(), file_name) == include_yaml_list_.end())
      {
        const auto absolute_path = std::filesystem::absolute(file_name).generic_string();

        if (checkIfFileExists(absolute_path))
        {
          include_yaml_list_.push_back(absolute_path);
          return_vector.push_back(ReadLaunchYamlImpl(include_yaml_list_.back(), include_yaml_list_));
        }
      }
    }

    return return_vector;
  }

  // Read a launch yaml file, handle includes and return the merged yaml node
  // include_yaml_list_ is used to avoid circular includes
  // TODO: handle relative paths and use absolute paths only (C++ 17)
  YAML::Node ReadLaunchYamlImpl(const std::string& path_, std::vector<std::string>& include_yaml_list_)
  {
    YAML::Node launch_yaml;
    try
    {
      const auto absolute_path = std::filesystem::absolute(path_).generic_string();
      if (checkIfFileExists(absolute_path))
        launch_yaml = YAML::LoadFile(absolute_path);
    }
    catch(const std::exception& e)
    {
      // TODO: consider error handling for recursive yaml read fails
      std::cerr << e.what() << '\n';
      return YAML::Node{};
    }

    auto base_includes = launch_yaml["includes"];
    if (base_includes && base_includes.IsSequence())
    {
      auto include_nodes = handleBaseIncludes(base_includes, include_yaml_list_);
      launch_yaml.remove("includes");

      for (const auto& include_node : include_nodes)
        mergeNodesOfSameType(launch_yaml, include_node);
    }

    return launch_yaml;
  }
}

namespace eCAL
{
  namespace Config
  {
    YAML::Node ReadLaunchYaml(const std::string& path_)
    {
      std::vector<std::string> yaml_include_list;
      return ReadLaunchYamlImpl(path_, yaml_include_list);
    }
  } // Config
} // eCAL