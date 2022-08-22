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

#include "config_file_parser.h"

ConfigFileParser::ConfigFileParser(const std::string& configuration_file_path):
  _configuration_file_path(configuration_file_path)
{
}

bool ConfigFileParser::isConfigFileValid()
{
  // check if file exists
  auto file_status = EcalUtils::Filesystem::FileStatus(_configuration_file_path, EcalUtils::Filesystem::OsStyle::Current);
  if (!file_status.IsOk())
  {
    eCALMeasCutterUtils::printError("Failed loading configuration file from \"" + _configuration_file_path + "\": File not found.");
    return false;
  }

  // check if file is empty
  // need this check because for some reason, an empty file is parsed successfully by yaml-cpp
  auto file_size = file_status.FileSize();
  if (file_size == 0)
  {
    eCALMeasCutterUtils::printError("Empty config file given at \"" + _configuration_file_path + "\".");
    return false;
  }

#ifdef WIN32
  std::ifstream config_file_stream(EcalUtils::StrConvert::Utf8ToWide(_configuration_file_path));
#else
  std::ifstream config_file_stream(_configuration_file_path);
#endif // WIN32

 
  // check if it is a valid yaml file
  try
  {
    _loaded_file = YAML::Load(config_file_stream);
  }
  catch (std::exception& e)
  {
    eCALMeasCutterUtils::printError("Failed loading configuration file from \"" + _configuration_file_path + "\": " + e.what() + ".");
    return false;
  }
  return true;
}

void ConfigFileParser::parseTrimOperations(eCALMeasCutterUtils::TrimOperation& trim_operation)
{
  if (_loaded_file["trim"])
  {
    YAML::Node trim_node = _loaded_file["trim"];

    trim_node >> trim_operation;
  }
}

void ConfigFileParser::parseChannelsOperations(std::list<std::string>& channel_list, std::list<std::string>& channel_list_regex, eCALMeasCutterUtils::ChannelOperationType operation_type)
{
  try
  {
    switch (operation_type)
    {
    case eCALMeasCutterUtils::ChannelOperationType::include:
    {
      if (_loaded_file["include"])
      {
        YAML::Node include_node = _loaded_file["include"];

        for (YAML::iterator it = include_node.begin(); it != include_node.end(); ++it)
        {
          if (it->IsMap())
          {
            auto node = it->begin();
            channel_list_regex.push_back(node->second.as<std::string>());
          }
          else
          {
            channel_list.push_back(it->as<std::string>());
          } 
        }
      }
      break;
    }
    case eCALMeasCutterUtils::ChannelOperationType::exclude:
    {
      if (_loaded_file["exclude"])
      {
        YAML::Node exclude_node = _loaded_file["exclude"];

        for (YAML::iterator it = exclude_node.begin(); it != exclude_node.end(); ++it)
        {
          if (it->IsMap())
          {
            auto node = it->begin();
            channel_list_regex.push_back(node->second.as<std::string>());
          }
          else
          {
            channel_list.push_back(it->as<std::string>());
          }
        }
        
      }
      break;
    }
    default:
      break;
    }
  }
  catch (const YAML::BadConversion& e)
  {
    eCALMeasCutterUtils::printError(e.what());
    std::exit(EXIT_FAILURE);
  }
}

void ConfigFileParser::parseConversionParameters(std::string& basename, size_t& max_size_per_file, bool& is_base_name_mentioned_in_config, bool& is_max_file_size_mentioned_in_config)
{
  try
  {
    if (_loaded_file["basename"])
    {
      if (_loaded_file["basename"].as<std::string>().compare("null") != 0)
      {
        basename = _loaded_file["basename"].as<std::string>();
        is_base_name_mentioned_in_config = true;
      }
    }
    if (_loaded_file["splitsize"])
    {
      if (_loaded_file["splitsize"].as<double>() < 0.0)
      {
        eCALMeasCutterUtils::printError("Negative maximum size per file.");
      }
      else
      {
        max_size_per_file = static_cast<size_t>(_loaded_file["splitsize"].as<double>());
        is_max_file_size_mentioned_in_config = true;
      }
    }
  }
  catch (const YAML::BadConversion& e)
  {
    eCALMeasCutterUtils::printError(e.what());
    std::exit(EXIT_FAILURE);
  }
}


