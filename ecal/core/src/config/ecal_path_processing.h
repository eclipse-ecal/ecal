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

#include <ecal/config.h>

namespace eCAL
{
  namespace Util
  {
    class IDirManager
    {
      public:
        virtual ~IDirManager() = default;

        IDirManager() = default;
        IDirManager(const IDirManager&) = default;
        IDirManager(IDirManager&&) = default;
        IDirManager& operator=(const IDirManager&) = default;
        IDirManager& operator=(IDirManager&&) = default;

        virtual bool dirExists(const std::string& path_) const = 0;
        virtual bool createDir(const std::string& path_) const = 0;
        virtual bool dirExistsOrCreate(const std::string& path_) const = 0;
        virtual bool createEcalDirStructure(const std::string& path_) const = 0;
        virtual bool canWriteToDirectory(const std::string& path_) const = 0;
        virtual std::string getDirectoryPath(const std::string& file_path_) const = 0;

        virtual std::string findFileInPaths(const std::vector<std::string>& paths_, const std::string& file_name_) const = 0;
    };

    class DirManager : public IDirManager
    {
      public:
        /**
         * @brief Check if the specified directory exists.
         *        Wrapper function for eCAL Utils Filesystem functionality.
         * 
         * @param path_ The path to the directory.
         * 
         * @return true if the directory exists, false otherwise.
         */
        bool dirExists(const std::string& path_) const override;

        /**
         * @brief Create the specified directory.
         *        Wrapper function for eCAL Utils Filesystem functionality.
         * 
         * @param path_ The path to the directory.
         * 
         * @return true if the directory was created, false otherwise.
         */
        bool createDir(const std::string& path_) const override;

        /**
         * @brief Check if the specified directory exists or create it.
         * 
         * @param path_ The path to the directory.
         * 
         * @return true if the directory exists or was created, false otherwise and when path_ is empty.
         */
        bool dirExistsOrCreate(const std::string& path_) const override;

        /**
         * @brief Create the eCAL directory structure.
         * 
         *        The function creates the specified directory and the logs subdirectory.
         * 
         * @param path_ The path to the eCAL directory.
         * 
         * @return true if the directory structure was created, false otherwise.
         */
        bool createEcalDirStructure(const std::string& path_) const override;

        /**
         * @brief Find the specified file in the provided paths.
         * 
         * @param paths_     The paths to search in.
         * @param file_name_ The name of the file to find.
         * 
         * @return std::string The first valid complete path to the file.
         *         Returns empty string if no valid path could be found.
         */
        std::string findFileInPaths(const std::vector<std::string>& paths_, const std::string& file_name_) const override;

        /**
         * @brief Check if the specified directory is writable.
         * 
         * @param path_ The path to the directory.
         * 
         * @return true if the directory is writable, false otherwise.
         */
        bool canWriteToDirectory(const std::string& path_) const override;

        /**
         * @brief Get the directory path of the specified file.
         * 
         * @param filePath The path to the file.
         * 
         * @return std::string The directory path of the file.
         */
        std::string getDirectoryPath(const std::string& file_path_) const override;
    };

    class IDirProvider 
    {
      public:
        virtual ~IDirProvider() = default;

        IDirProvider() = default;
        IDirProvider(const IDirProvider&) = default;
        IDirProvider(IDirProvider&&) = default;
        IDirProvider& operator=(const IDirProvider&) = default;
        IDirProvider& operator=(IDirProvider&&) = default;

        virtual std::string eCALEnvVar(const std::string& var_) const = 0;
        virtual std::string eCALLocalUserDir() const = 0;
        virtual std::string eCALDataSystemDir(const eCAL::Util::IDirManager& dir_manager_) const = 0;
        virtual std::string uniqueTmpDir(const eCAL::Util::IDirManager& dir_manager_) const = 0;
    };

    class DirProvider : public IDirProvider
    {
      public:
        /**
         * @brief Get the value of the specified environment variable.
         * 
         * @param var_ The name of the environment variable.
         * 
         * @return The value of the environment variable.
         */
        std::string eCALEnvVar(const std::string& var_) const override;
        
        /**
         * @brief The path to the local user settings directory if it exists.
         * 
         *        E.g. AppData/Local/eCAL [win], ~/.ecal [unix]
         * 
         * @return The path to the local user settings directory.
         *         Returns empty string if the path does not exist.
         */
        std::string eCALLocalUserDir() const override;

        /**
         * @brief The default path to the eCAL data system directory if it exists.
         * 
         *        E.g. ProgramData/eCAL [win], /etc/ecal [unix]
         * 
         * @returns The path to the eCAL data system directory.
         *          Returns empty string if the path does not exist.
         */
        std::string eCALDataSystemDir(const Util::IDirManager& dir_manager_) const override;

        /**
         * @brief Returns a unique temporary folder.
         * 
         *        The folder will be created and returned.
         * 
         * @returns The unique temporary folder directory.
         */
        std::string uniqueTmpDir(const eCAL::Util::IDirManager& dir_manager_) const override;
    };
  } // namespace Util

  namespace Config
  {
   /**
   * @brief Check the following paths if the specified config file exists and return the first valid complete path
   * 
   *          The function checks the following paths in order:
   *          1. ECAL_DATA environment variable path
   *          2. Current working directory
   *          3. Local user path
   *          4. Global system path
   * 
   * @param config_file_ The name of the configuration file to check.
   * 
   * @return std::string The first valid complete path to the configuration file.
   *                     Returns empty string if non could be found.
   */
    std::string checkForValidConfigFilePath(const std::string& config_file_, const Util::DirProvider& dir_provider_ = Util::DirProvider(), const Util::DirManager& dir_manager_ = Util::DirManager());  

    /**
     * @brief Returns the path to the eCAL log directory. Searches in following order:
     *
     *        1. Environment variable ECAL_LOG_DIR
     *        2. Environment variable ECAL_DATA (also checking for logs subdirectory)
     *        3. The path provided from the configuration
     *        4. The path where ecal.yaml was loaded from (also checking for logs subdirectory)
     *        5. The temporary directory (e.g. /tmp [unix], Appdata/local/Temp [win])
     *        6. Fallback path /ecal_tmp
     * 
     *        In case of 5/6, a unique temporary folder will be created.
     *        
     * @returns The path to the eCAL log directory. The subdirectory logs might not exist yet.
     *          Returns empty string if no root path could be found.
     */
    std::string GeteCALLogDirImpl(const Util::IDirProvider& dir_provider_ = Util::DirProvider(), const Util::IDirManager& dir_manager_ = Util::DirManager(), const eCAL::Configuration& config_ = eCAL::GetConfiguration());

    /**
     * @brief Returns the path to the eCAL data directory. Searches in following order:
     *
     *       1. Environment variable ECAL_DATA
     *       2. Local user path
     *       3. System path like /etc/ecal, ProgramData/eCAL
     * 
     * @returns The path to the eCAL data directory.
     *          Returns empty string no valid path was or the path does not exist yet.
     */
    std::string GeteCALDataDirImpl(const Util::IDirProvider& dir_provider_ = Util::DirProvider(), const Util::IDirManager& dir_manager_  = Util::DirManager());

    /**
     * @brief Returns the default paths for possible ecal configurations in following order:
     *
     *        1. ECAL_DATA environment varible path if set
     *        2. local user path
     *        3. system path like etc, ProgramData
     * 
     * @returns std::vector<std::string> The default paths for possible ecal configurations.
     */
    std::vector<std::string> getEcalDefaultPaths(const Util::IDirProvider& dir_provider_, const Util::IDirManager& dir_manager_ );
  } // namespace Config
}