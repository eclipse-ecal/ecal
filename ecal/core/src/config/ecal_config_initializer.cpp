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
#include "default_config.h"

#include "ecal/ecal_process.h"
#include "config/ecal_cmd_parser.h"
#include "configuration_to_yaml.h"
#include "configuration_reader.h"

// for cwd
#ifdef ECAL_OS_WINDOWS
  #include <direct.h>
  // to remove deprecated warning
  #define getcwd _getcwd
#endif
#ifdef ECAL_OS_LINUX
  #include <unistd.h>
#endif

#include "ecal_utils/filesystem.h"
#include "util/getenvvar.h"

#include <algorithm>

namespace
{
  // copied and adapted from ecal_config_reader.cpp
#ifdef ECAL_OS_WINDOWS
  const char path_separator('\\');
#endif /* ECAL_OS_WINDOWS */
#ifdef ECAL_OS_LINUX
  const char path_separator('/');
#endif /* ECAL_OS_LINUX */

  bool setPathSep(std::string& file_path_)
  {
    if (!file_path_.empty())
    {
      if (file_path_.back() != path_separator)
      {
        file_path_ += path_separator;
      }
      return true;
    }

    return false;
  }

  std::string eCALDataEnvPath()
  {
    std::string ecal_data_path = getEnvVar("ECAL_DATA");
    setPathSep(ecal_data_path);
    return ecal_data_path;
  }

  std::string cwdPath()
  {
    std::string cwd_path = { getcwd(nullptr, 0) };
            
    setPathSep(cwd_path);
    return cwd_path;
  }

  std::string eCALDataCMakePath()
  {
    std::string cmake_data_path;
#ifdef ECAL_OS_LINUX
    const std::string ecal_install_config_dir(ECAL_INSTALL_CONFIG_DIR);
    const std::string ecal_install_prefix(ECAL_INSTALL_PREFIX);

    if ((!ecal_install_config_dir.empty() && (ecal_install_config_dir[0] == path_separator))
      || ecal_install_prefix.empty())
    {
      cmake_data_path = ecal_install_config_dir;
    }
    else if (!ecal_install_prefix.empty())
    {
      cmake_data_path = ecal_install_prefix + path_separator + ecal_install_config_dir;
    }
    setPathSep(cmake_data_path);
#endif /* ECAL_OS_LINUX */  
    return cmake_data_path;
  }

  std::string eCALDataSystemPath()
  {
    std::string system_data_path;
#ifdef ECAL_OS_WINDOWS
    system_data_path = getEnvVar("ProgramData");
    if(setPathSep(system_data_path))
    {
        system_data_path += std::string("eCAL");
        setPathSep(system_data_path);
    }
#endif /* ECAL_OS_WINDOWS */

#ifdef ECAL_OS_LINUX
    system_data_path = "/etc/ecal";
    setPathSep(system_data_path);
#endif /* ECAL_OS_LINUX */
    return system_data_path;
  }

  void appendFileNameToPathIfPathIsValid(std::string& path_, const std::string& file_name_)
  {
    if (!path_.empty())
      path_ += file_name_;
  }


  bool isValidConfigFilePath(const std::string& file_path_)
  {
    // check existence of user defined file
    const EcalUtils::Filesystem::FileStatus ecal_ini_status(file_path_, EcalUtils::Filesystem::Current);
    return ecal_ini_status.IsOk() && (ecal_ini_status.GetType() == EcalUtils::Filesystem::Type::RegularFile);
  }

  std::string checkForValidConfigFilePath(const std::string& config_file_)
    {
      // differences to ecal_config_reader implementation are:
      //    1. does not use the default ini file name, instead uses the specified file
      //    2. searches relative to the executable path and takes it as highest priority

      // -----------------------------------------------------------
      // precedence 1: relative path to executable
      // -----------------------------------------------------------
      std::string cwd_directory_path = cwdPath();
      appendFileNameToPathIfPathIsValid(cwd_directory_path, config_file_);

      // -----------------------------------------------------------
      // precedence 2: ECAL_DATA variable (windows and linux)
      // -----------------------------------------------------------
      std::string ecal_data_path = eCALDataEnvPath();
      appendFileNameToPathIfPathIsValid(ecal_data_path, config_file_);
      
      // -----------------------------------------------------------
      // precedence 3:  cmake configured data paths (linux only)
      // -----------------------------------------------------------
      std::string cmake_data_path = eCALDataCMakePath();
      appendFileNameToPathIfPathIsValid(cmake_data_path, config_file_);

      // -----------------------------------------------------------
      // precedence 4: system data path 
      // -----------------------------------------------------------
      std::string system_data_path = eCALDataSystemPath();
      appendFileNameToPathIfPathIsValid(system_data_path, config_file_);

      // Check for first directory which contains the ini file.
      std::vector<std::string> search_directories{ cwd_directory_path, ecal_data_path, cmake_data_path, system_data_path };

      auto it = std::find_if(search_directories.begin(), search_directories.end(), isValidConfigFilePath);
      // We should have encountered a valid path
      if (it != search_directories.end())
        return (*it);

      // Check if user specified complete path, in case all other precedence paths exist
      if (isValidConfigFilePath(config_file_))
      {
        return std::string(config_file_);
      }

      // If valid path is not encountered, return empty string
      return std::string("");
    }


}

namespace eCAL 
{
    void Configuration::InitConfigFromFile(std::string yaml_path_)
    {
      std::string yaml_path = checkForValidConfigFilePath(yaml_path_);
      if (!yaml_path.empty())
      {
        eCAL::Config::YamlFileToConfig(yaml_path, *this);
        ecal_yaml_file_path = yaml_path;
      }
      else
      {
        std::cout << "Specified yaml configuration path not valid: " << "\"" << yaml_path_ << "\"" << " Using default configuration." << "\n";
        InitConfigWithDefaults();
      }
    };

    void Configuration::InitConfigWithDefaults()
    {
      eCAL::Config::YamlStringToConfig(default_config, *this);
    }

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
      
      command_line_arguments.user_yaml   = parser.getUserIni();
      command_line_arguments.dump_config = parser.getDumpConfig();

      if (command_line_arguments.user_yaml.empty())
      {
        InitConfigWithDefaultYaml();
      }
      else
      {
        InitConfigFromFile(command_line_arguments.user_yaml);
      }
    }

    void Configuration::InitConfigWithDefaultYaml()
    {
      InitConfigFromFile(g_default_ini_file);
    }

    Configuration::Configuration()
    {
      InitConfigWithDefaults();
    }

    std::string Configuration::GetYamlFilePath()
    {
      return ecal_yaml_file_path;
    }

    Configuration& GetConfiguration()
    {
      return g_ecal_configuration;
    };
}