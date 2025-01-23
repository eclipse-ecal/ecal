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

#pragma once

#include "ecal/config/configuration.h"
#include "ecal/config.h"
#include "ecal_def.h"

#include <fstream>

namespace eCAL
{
  namespace Config
  {
    /**
     * @brief Dump the given configuration to a file.
     * 
     * @param config_    The configuration to dump.
     * @param file_path_ The complete path to the file to write the configuration to.
     * 
     * @return bool      True if the configuration was successfully written to the file, false otherwise.
     */
    bool dumpConfigToFile(const eCAL::Configuration& config_ = eCAL::GetConfiguration(), const std::string& file_path_ = ECAL_DEFAULT_CFG);

    /**
     * @brief Dump the default configuration to default file (ecal.yaml).
     * 
     * @param config_dir_ The directory to write the default configuration file to.
     * 
     * @return bool       True if the default configuration was successfully written to the file, false otherwise.
     */
    bool dumpDefaultConfig(const std::string& config_dir_);

    /**
     * @brief Dump the given stringstream to a file.
     * 
     * @param ss_        The stringstream to dump.
     * @param file_path_ The complete path to the file to write the stringstream to.
     * 
     * @return bool      True if the stringstream was successfully written to the file, false otherwise.
     */
    bool dumpToFile(const std::stringstream& ss_, const std::string& file_path_);
  }
}