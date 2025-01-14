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
 * @brief  eCAL config writer functions
**/

#include "configuration_writer.h"
#include "default_configuration.h"
#include "ecal_utils/filesystem.h"
#include "ecal_utils/string.h"

#include <fstream>

namespace eCAL
{
  namespace Config
  {
    bool dumpToFile(const std::stringstream& ss_, const std::string& file_path_)
    {
      std::ofstream file(file_path_);
      if (!file.is_open())
      {
        return false;
      }

      file << ss_.str();
      file.close();
      return true;
    }
    
    bool dumpConfigToFile(const eCAL::Configuration& config_, const std::string& file_path_)
    {
      const std::stringstream stream = getConfigAsYamlSS(config_);

      return dumpToFile(stream, file_path_);
    }

    bool dumpDefaultConfig(const std::string& config_dir_)
    {
      const std::string path_sep = std::string(1, EcalUtils::Filesystem::NativeSeparator());

      const std::string file_path = EcalUtils::String::Join(path_sep, std::vector<std::string>{config_dir_, ECAL_DEFAULT_CFG});
      const eCAL::Configuration default_config{};

      return dumpConfigToFile(default_config, file_path);
    }
  }
}