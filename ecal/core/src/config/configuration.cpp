/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include "ecal/config.h"

#include "ecal/util.h"

#include "ecal_path_processing.h"
#include "ecal_global_accessors.h"


#ifdef ECAL_CORE_CONFIGURATION
  #include "configuration_reader.h"
  #include "configuration_to_yaml.h"
#endif

namespace eCAL 
{
    void Configuration::InitFromFile(const std::string& yaml_path_)
    {
      const std::string yaml_path = eCAL::Config::checkForValidConfigFilePath(yaml_path_);
      if (!yaml_path.empty())
      {
#ifdef ECAL_CORE_CONFIGURATION
        eCAL::Config::YamlFileToConfig(yaml_path, *this);
        ecal_yaml_file_path = yaml_path;
        std::cout << "[eCAL][Config] Yaml configuration loaded from \"" << yaml_path << "\"." << "\n";
#else
        std::cout << "[eCAL][Config] Yaml file found at \"" << yaml_path << "\" but eCAL core configuration is not enabled." << "\n";
#endif
      }
      else
      {
        std::cout << "[eCAL][Config] Specified yaml configuration path not valid:\"" << yaml_path_ << "\". Using default configuration." << "\n";
      }
    }

    void Configuration::InitFromConfig()
    {
      InitFromFile(g_default_ini_file);
    }

    Configuration::Configuration() = default;

    std::string Configuration::GetConfigurationFilePath() const
    {
      return ecal_yaml_file_path;
    }

    const Configuration& GetConfiguration()
    {
      return g_ecal_configuration;
    }
}