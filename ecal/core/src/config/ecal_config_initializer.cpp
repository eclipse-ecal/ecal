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
 * @brief  Function definitions for setting default config values
**/

#include "ecal/ecal_config.h"

#include "ecal_global_accessors.h"
#include "ecal_def.h"
#include "config/ecal_config_reader.h"

#include "ecal/ecal_process.h"
#include "config/ecal_cmd_parser.h"
#include "configuration_to_yaml.h"
#include "configuration_reader.h"

#include <algorithm>

constexpr const char* COMMON       = "common";
constexpr const char* MONITORING   = "monitoring";
constexpr const char* NETWORK      = "network";
constexpr const char* EXPERIMENTAL = "experimental";
constexpr const char* PUBLISHER    = "publisher";
constexpr const char* SYS          = "sys";
constexpr const char* TIME         = "time";
constexpr const char* SERVICE      = "service";
constexpr const char* PROCESS      = "process";

namespace {
  void tokenize(const std::string& str, std::vector<std::string>& tokens,
      const std::string& delimiters = " ", bool trimEmpty = false)
  {
    std::string::size_type pos = 0;
    std::string::size_type lastPos = 0;

    for (;;)
    {
      pos = str.find_first_of(delimiters, lastPos);
      if (pos == std::string::npos)
      {
        pos = str.length();
        if (pos != lastPos || !trimEmpty)
        {
          tokens.emplace_back(str.data() + lastPos, pos - lastPos);
        }
        break;
      }
      else
      {
        if (pos != lastPos || !trimEmpty)
        {
          tokens.emplace_back(str.data() + lastPos, pos - lastPos);
        }
      }
      lastPos = pos + 1;
    }
  }

  eCAL_Logging_Filter ParseLogLevel(const std::string& filter_)
  {
    // tokenize it
    std::vector<std::string> token_filter_;
    tokenize(filter_, token_filter_, " ,;");
    // create excluding filter list
    char filter_mask = log_level_none;
    for (auto& it : token_filter_)
    {
      if (it == "all")     filter_mask |= log_level_all;
      if (it == "info")    filter_mask |= log_level_info;
      if (it == "warning") filter_mask |= log_level_warning;
      if (it == "error")   filter_mask |= log_level_error;
      if (it == "fatal")   filter_mask |= log_level_fatal;
      if (it == "debug1")  filter_mask |= log_level_debug1;
      if (it == "debug2")  filter_mask |= log_level_debug2;
      if (it == "debug3")  filter_mask |= log_level_debug3;
      if (it == "debug4")  filter_mask |= log_level_debug4;
    }
    return(filter_mask);
  };
}

namespace eCAL 
{
    void Configuration::InitConfig(std::string ini_path_ /*= std::string("")*/)
    {
      if (!command_line_arguments.config_keys.empty())
      {
        // needs new implementation here
      }

      if (!ini_path_.empty())
      {
        *this = eCAL::Config::ParseYamlFromFile(ini_path_);
        ecal_ini_file_path = ini_path_;
      }
    };

    Configuration::Configuration(int argc_ , char **argv_)
    {
      std::vector<std::string> arguments;
      if (argc_ > 0 && argv_ != nullptr)
      {
        for (size_t i = 0; i < static_cast<size_t>(argc_); ++i) 
          if (argv_[i] != nullptr) 
            arguments.emplace_back(argv_[i]);
      } 
      Init(arguments);
    }

    Configuration::Configuration(std::vector<std::string>& args_)
    {      
      Init(args_);
    }

    void Configuration::Init(std::vector<std::string>& arguments_)
    {
      Config::CmdParser parser(arguments_);
      
      command_line_arguments.config_keys       = parser.getConfigKeys();
      command_line_arguments.specified_config  = parser.getUserIni();
      command_line_arguments.dump_config       = parser.getDumpConfig();
      command_line_arguments.config_keys_map   = parser.getConfigKeysMap();

      InitConfig(command_line_arguments.specified_config);
    }

    void Configuration::InitConfigWithDefaultIni()
    {
      InitConfig(g_default_ini_file);
    }

    Configuration::Configuration()
    : Configuration(0, nullptr)
    {
      InitConfig();
    }

    std::string Configuration::GetIniFilePath()
    {
      return ecal_ini_file_path;
    }

    Configuration& GetConfiguration()
    {
      return g_ecal_configuration;
    };
}