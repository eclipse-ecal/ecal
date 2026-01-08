/* ========================= eCAL LICENSE =================================
 *
 * Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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

#include "yaml_functions.h"

#include <stack>

namespace
{
  void MergeHandleScalar(YAML::Node& base_, const YAML::Node& other_)
  {

  }

  void HandleNodeIterator(YAML::Node& base_, const YAML::const_iterator& it_)
  {
    switch (it_->Type())
    {
    case YAML::NodeType::Undefined:
      
      break;
    case YAML::NodeType::Scalar:
      break;

    case YAML::NodeType::Sequence:
      break;

    case YAML::NodeType::Map:
      base_[it_->first] = it_->second;
      break;
    
    default:
      break;
    }
  }
}

namespace eCAL
{
  namespace Util
  {
    void MergeNodes(YAML::Node& base_, const YAML::Node& other_)
    {
      YAML::Node new_node;

      for (auto it = other_.begin(); it != other_.end(); it++)
      {
        if (it->Tag().empty())
        HandleNodeIterator(base_, it);
      }

    }

    void MergeYamlNodes(YAML::Node& base, const YAML::Node& other) 
    {
      std::stack<std::pair<YAML::Node, YAML::Node>> nodes;
      nodes.emplace(base, other);

      while (!nodes.empty()) 
      {
        const std::pair<YAML::Node, YAML::Node> nodePair = nodes.top();
        nodes.pop();

        YAML::Node baseNode = nodePair.first;
        YAML::Node otherNode = nodePair.second;

        for (YAML::const_iterator it = otherNode.begin(); it != otherNode.end(); ++it) 
        {
          const YAML::Node key = it->first;
          const YAML::Node value = it->second;

          std::string key_as_string;

          switch (key.Type())
          {
            case YAML::NodeType::Scalar:
              key_as_string = key.as<std::string>();
              break;
            default:
              continue;
              break;
          }

          if (baseNode[key_as_string]) 
          {
            if (value.IsMap() && baseNode[key_as_string].IsMap()) 
            {
              nodes.emplace(baseNode[key_as_string], value); // Push nested nodes to stack
            } 
            else 
            {
              baseNode[key_as_string] = value; // Overwrite value for non-map nodes
            }
          } 
          else 
          {
            baseNode[key_as_string] = value; // Add new key-value pairs
          }
        }
      }
    }
  }
}