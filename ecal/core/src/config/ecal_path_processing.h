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
 * @brief  eCAL config path processing function collection
**/

#pragma once

#include <string>
#include <vector>

namespace eCAL
{
  namespace Util
  {
    class IEnvProvider 
    {
      public:
        virtual ~IEnvProvider() = default;

        virtual std::string eCALEnvVar(const std::string& var_) const = 0;
    };

    class EnvProvider : public IEnvProvider
    {
      public:
        std::string eCALEnvVar(const std::string& var_) const override;
    };

    class IDirManager
    {
      public:
        virtual ~IDirManager() = default;

        virtual bool dirExists(const std::string& path_) const = 0;
        virtual bool createDir(const std::string& path_) const = 0;
        virtual bool dirExistsOrCreate(const std::string& path_) const = 0;
        virtual bool createEcalDirStructure(const std::string& path_) const = 0;
    };

    class DirManager : public IDirManager
    {
      public:
        bool dirExists(const std::string& path_) const override;
        bool createDir(const std::string& path_) const override;
        bool dirExistsOrCreate(const std::string& path_) const override;
        bool createEcalDirStructure(const std::string& path_) const override;
    };

    std::vector<std::string> getEcalDefaultPaths(const IEnvProvider& env_provider_ = EnvProvider());
  } // namespace Util

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
     * @brief The path to the local user settings directory if it exists.
     * 
     *        E.g. AppData/Local/eCAL [win], ~/.ecal [unix]
     * 
     * @return The path to the local user settings directory.
     *         Returns empty string if the path does not exist.
     */
    std::string eCALLocalUserDir();

    /**
     * @brief The default path to the eCAL data system directory if it exists.
     * 
     *        E.g. ProgramData/eCAL [win], /etc/ecal [unix]
     * 
     * @returns The path to the eCAL data system directory.
     *          Returns empty string if the path does not exist.
     */
    std::string eCALDataSystemDir();

    /**
     * @brief Returns the path to the eCAL log directory based on the ecal data directories in following order:
     * 
     *        1. Environment variable ECAL_LOG_DIR
     *        2. Environment variable ECAL_DATA
     *        3. The path provided from the configuration
     *        4. The path to the local eCAL data directory
     *        5. For windows: the system data path if available (ProgramData/eCAL)
     *        6. The temporary directory (e.g. /tmp [unix], Appdata/local/Temp [win])
     *        7. Fallback path /ecal_tmp
     * 
     *        In case of 6/7, a unique temporary folder will be created.
     *        
     * @returns The path to the eCAL log directory. The subdirectory logs might not exist yet.
     *          Returns empty string if no root path could be found.
     */
    std::string eCALLogDir(const Util::IEnvProvider& env_provider_ = Util::EnvProvider(), const Util::IDirManager& dir_manager_ = Util::DirManager());

    /**
     * @brief Creates in the specified directory the logs subdirectory.
     * 
     * @param path_ The eCAL config/logs directory
     * 
     * @returns True if the directory structure could be created successfully.
     */
  } // namespace Config
}