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
 * @brief  eCAL config path processing function collection
**/

#include <string>

namespace eCAL
{
  namespace Config
  {
   /**
   * @brief Check the following paths if the specified config file exists and return the first valid complete path
   * 
   *          The function checks the following paths in order:
   *          1. ECAL_CONFIG_DIR environment variable path
   *          2. Current working directory
   *          3. Local user path
   *          4. Global system path
   * 
   * @param config_file_ The name of the configuration file to check.
   * @return std::string The first valid complete path to the configuration file.
   *                     Returns empty string if non could be found.
   */
    std::string checkForValidConfigFilePath(const std::string& config_file_);


    /**
     * @brief Returns the path to the local user settings directory if it exists.
     * 
     * @return std::string The path to the local user settings directory.
     *                     Returns empty string if the path does not exist.
     */
    std::string eCALLocalUserPath();

    /**
     * @brief Returns the default path to the eCAL data system directory if it exists.
     * 
     * @returns std::string The path to the eCAL data system directory.
     *                      Returns empty string if the path does not exist.
     */
    std::string eCALDataSystemPath();

    /**
     * @brief Returns the path to the eCAL directory specified by the environment variable ECAL_CONFIG.
     * 
     * @returns std::string The path to the eCAL config directory.
     *                      Returns empty string if the path does not exist.
     */
    std::string eCALConfigEnvPath();

    /**
     * @brief Returns the path to the eCAL directory specified by the environment variable ECAL_LOG.
     * 
     *        In case the path does not exist yet, the function tries to create it.
     *        (e.g. ProgramData/eCAL/logs [win], ~/.ecal/logs [unix])
     *        If the creation fails, the function returns an empty string.
     * 
     * @returns std::string The path to the eCAL log directory.
     */
    std::string eCALConfigLogPath();

    /**
     * @brief Returns the path to the eCAL temporary directory, which is /ecal_tmp.
     * 
     * @returns std::string The path to the eCAL temporary directory.
     */
    std::string eCALTmpPath();
  }
}