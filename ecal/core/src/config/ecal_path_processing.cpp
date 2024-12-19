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

#include "ecal_path_processing.h"

#include "ecal_def.h"
#include "ecal_utils/ecal_utils.h"
#include "ecal_utils/filesystem.h"
#include "ecal/ecal_config.h"
#include "ecal/ecal_util.h"
#include "util/getenvvar.h"

#include <vector>

// for cwd
#ifdef ECAL_OS_WINDOWS
  #include <windows.h>
  #include <ShlObj.h>
#endif
#ifdef ECAL_OS_LINUX
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <unistd.h>
  #include <.h>
#endif

namespace
{
  // get the path separator from the current OS (win: "\\", unix: "/")
  static const std::string path_separator(1, EcalUtils::Filesystem::NativeSeparator());

  bool direxists(const std::string& path_)
  {
    const EcalUtils::Filesystem::FileStatus status(path_, EcalUtils::Filesystem::Current);
    return (status.IsOk() && (status.GetType() == EcalUtils::Filesystem::Type::Dir));
  }

  bool createdir(const std::string& path_)
  {
    return EcalUtils::Filesystem::MkDir(path_, EcalUtils::Filesystem::Current);
  }

  bool isValidConfigFilePath(const std::string& path_, const std::string& file_name_)
  {
    const std::string file_path = EcalUtils::String::Join(path_separator, std::vector<std::string>{path_, file_name_});
    const EcalUtils::Filesystem::FileStatus ecal_ini_status(file_path, EcalUtils::Filesystem::Current);
    return ecal_ini_status.IsOk() && (ecal_ini_status.GetType() == EcalUtils::Filesystem::Type::RegularFile);
  }

  // Returns path, where the specified ini file is found, or empty string if not found
  std::string findValidConfigPath(std::vector<std::string> paths_, const std::string& file_name_)
  {
    auto it = std::find_if(paths_.begin(), paths_.end(), [&file_name_](const std::string& path_)
    {
      if (path_.empty())
        return false;
      
      return isValidConfigFilePath(path_, file_name_);
    });

    // We should have encountered a valid path
    if (it != paths_.end())
      return (*it);

    // If valid path is not encountered, defaults should be used
    return {};
  }

  // Returns the default paths for possible ecal configurations
  // Order:
  // 1. ECAL_CONFIG environment varible path if set
  // 2. local user path
  // 3. system path like etc, ProgramData
  std::vector<std::string> getEcalDefaultPaths()
  {
    std::vector<std::string> ecal_default_paths;
    // -----------------------------------------------------------
    // precedence 1: ECAL_CONFIG variable (windows and linux)
    // -----------------------------------------------------------
    ecal_default_paths.emplace_back(eCAL::Config::eCALConfigEnvPath());

    // -----------------------------------------------------------
    // precedence 2: local user path 
    // -----------------------------------------------------------
    ecal_default_paths.emplace_back(eCAL::Config::eCALLocalUserPath());

    // -----------------------------------------------------------
    // precedence 3: eCAL data system path 
    // -----------------------------------------------------------
    ecal_default_paths.emplace_back(eCAL::Config::eCALDataSystemPath());
    return ecal_default_paths;
  }

#ifdef ECAL_OS_WINDOWS
  std::string getKnownFolderPath(REFKNOWNFOLDERID id_)
  {
    std::string return_path;
    PWSTR path_tmp = nullptr;

    // Retrieve the known folder path: users local app data
    auto ret = SHGetKnownFolderPath(id_, 0, nullptr, &path_tmp);

    if (ret != S_OK)
    {
      if (path_tmp != nullptr)
        // Free the memory allocated by SHGetKnownFolderPath
        CoTaskMemFree(path_tmp);

      return {};
    }

    // Convert the wide-character string to a multi-byte string
    // For supporting full Unicode compatibility
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, path_tmp, -1, nullptr, 0, nullptr, nullptr);
    if (size_needed > 0)
    {
      // Exclude the null terminator from the size
      return_path.resize(size_needed - 1);
      WideCharToMultiByte(CP_UTF8, 0, path_tmp, -1, &return_path[0], size_needed, nullptr, nullptr);
    }
    
    // Free the memory allocated by SHGetKnownFolderPath
    CoTaskMemFree(path_tmp);

    return return_path;
  }
#endif /* ECAL_OS_WINDOWS */

  // returns local users home path
  // e.g. C:\Users\username\AppData\Local in windows
  // or /home/username in linux
  // returns empty string if not found
  std::string getLocalUserPath()
  {
    std::string userspace_path;
  #ifdef ECAL_OS_WINDOWS
    
    userspace_path = getKnownFolderPath(FOLDERID_LocalAppData);
  
  #elif defined(ECAL_OS_LINUX)
    
    std::string hdir = getEnvVar(ECAL_LINUX_HOME_VAR);

  #endif /* ECAL_OS_LINUX */
    return userspace_path;
  }

  std::string getSystemPath()
  {
    std::string system_path;
  #ifdef ECAL_OS_WINDOWS

    system_path = getKnownFolderPath(FOLDERID_ProgramData);
  
  #elif defined(ECAL_OS_LINUX)
  
    system_path = ECAL_LINUX_DATA_SYSTEM_PATH;
  
  #endif /* ECAL_OS_LINUX */
    return system_path;
  }

  std::string getTempPath()
  {
    std::string temp_path;

  #ifdef ECAL_OS_WINDOWS
      
      char temp_path_buffer[MAX_PATH];
      DWORD path_length = GetTempPathA(MAX_PATH, temp_path_buffer);
      if (path_length > 0 && path_length < MAX_PATH)
      {
        temp_path = std::string(temp_path_buffer, path_length);
      }
      
  #elif defined(ECAL_OS_LINUX)

      temp_path = getEnvVar(ECAL_LINUX_TMP_VAR);

  #endif /* ECAL_OS_LINUX */    

    return temp_path;
  }
}

namespace eCAL
{
  namespace Config
  {
    std::string eCALLocalUserPath()
    {
      std::string userspace_path = getLocalUserPath();

      if (!userspace_path.empty())
      {
    #ifdef ECAL_OS_WINDOWS
        
        return EcalUtils::String::Join(path_separator, std::vector<std::string>{userspace_path, ECAL_FOLDER_NAME_WINDOWS});
        
    #elif defined(ECAL_OS_LINUX)
        
        return EcalUtils::String::Join(path_separator, std::vector<std::string>{userspace_path, ECAL_FOLDER_NAME_HOME_LINUX});
        
    #endif
      }
      
      return {};
    }

    std::string eCALConfigEnvPath()
    {
      return getEnvVar(ECAL_CONFIG_VAR);
    }

    std::string eCALConfigLogPath()
    {
      // check first if the ECAL_LOG environment variable is set
      std::string env_path = getEnvVar(ECAL_LOG_VAR);
      if (!env_path.empty())
      {
        return env_path;
      }

      // if no environment variable is set, check the local user path
      std::string local_user_path = eCALLocalUserPath();
      if (!local_user_path.empty())
      {
        return EcalUtils::String::Join(path_separator, std::vector<std::string>{local_user_path, ECAL_FOLDER_NAME_LOG});
      }

    #ifdef ECAL_OS_WINDOWS
      // only works on windows, as ProgramData is writable for the normal user
      // check the system_data_path if available
      std::string system_data_path = getSystemPath();
      if (!system_data_path.empty())
      {
        return EcalUtils::String::Join(path_separator, std::vector<std::string>{system_data_path, ECAL_FOLDER_NAME_WINDOWS, ECAL_FOLDER_NAME_LOG});
      }
    #endif

      return {};
    }

    std::string eCALDataSystemPath()
    {
      std::string system_path = getSystemPath();
      std::string return_path;

      if (!system_path.empty())
      {
    #ifdef ECAL_OS_WINDOWS
      
        // system path = "ProgramData" if available      
        return_path = EcalUtils::String::Join(path_separator, std::vector<std::string>{system_path, ECAL_FOLDER_NAME_WINDOWS});
    
    #elif defined(ECAL_OS_LINUX)
      
        // system path = "/etc" if available
        return_path = EcalUtils::String::Join(path_separator, std::vector<std::string>{system_path, ECAL_FOLDER_NAME_HOME_LINUX});
      
    #endif 
      }
 
      return return_path;;
    }

    std::string checkForValidConfigFilePath(const std::string& config_file_)
    {
      // -----------------------------------------------------------
      // precedence 0: relative path to executable
      // -----------------------------------------------------------
      const std::string cwd_directory_path = EcalUtils::Filesystem::CurrentWorkingDir();

      std::vector<std::string> ecal_default_paths = getEcalDefaultPaths();
      
      // insert cwd on 2nd position, so that ECAL_CONFIG dir has precedence
      ecal_default_paths.insert(ecal_default_paths.begin() + 1, cwd_directory_path);
      
      const std::string found_path = findValidConfigPath(ecal_default_paths, config_file_);

      // check in case user provided whole path
      if (found_path.empty())
      {
        return isValidConfigFilePath(config_file_, "") ? config_file_ : found_path;
      }

      return found_path + config_file_;
    }
  } // namespace Config
  
  namespace Util
  {
    std::string GeteCALConfigPath()
    {
      // Return the possible default paths that could contain the yaml file
      const std::vector<std::string> search_directories = getEcalDefaultPaths();
      
      // Check for first directory which contains the ini file.
      return findValidConfigPath(search_directories, ECAL_DEFAULT_CFG);
    }

    // Returns the path to the eCAL installation directory
    std::string GeteCALHomePath()
    {
      std::string home_path;
    #ifdef ECAL_OS_WINDOWS
      
      // check ECAL_HOME
      home_path = getEnvVar(ECAL_HOME_VAR);
      
      // TODO PG: Check, why there is a ECAL_FOLDER_NAME_WINDOWS - What is the expected behaviour?
      if (!std::string(ECAL_FOLDER_NAME_WINDOWS).empty()) //-V815
      {
        home_path = EcalUtils::String::Join(path_separator, std::vector<std::string>{home_path, ECAL_FOLDER_NAME_WINDOWS});
      }

    #elif defined(ECAL_OS_LINUX)
      
      const char *hdir = nullptr;
      home_path = getEnvVar(ECAL_LINUX_HOME_VAR);
      if (hdir == nullptr) {
        hdir = getpwuid(getuid())->pw_dir;
      }
      home_path += hdir;
      if (!std::string(ECAL_FOLDER_NAME_HOME_LINUX).empty())
      {
        home_path = EcalUtils::String::Join(path_separator, std::vector<std::string>{home_path, ECAL_FOLDER_NAME_WINDOWS});
      }

    #endif /* ECAL_OS_LINUX */

      // create if not exists
      if (!direxists(home_path))
      {
        return {};
      }

      return(home_path);
    }

    std::string GeteCALUserSettingsPath()
    {
      std::string config_path;
    #ifdef ECAL_OS_WINDOWS

      config_path = GeteCALConfigPath();

    #elif defined(ECAL_OS_LINUX)
      
      config_path = GeteCALHomePath();

    #endif /* ECAL_OS_LINUX */
      
      if (config_path.empty())
      {
        return {};
      }

      std::string settings_path = EcalUtils::String::Join(path_separator, std::vector<std::string>{config_path, ECAL_FOLDER_NAME_CFG});

      if (!direxists(settings_path))
      {
        if (!createdir(settings_path))
        {
          return {};
        }
      }

      return(settings_path);
    }

    std::string GeteCALLogPath()
    {
      const auto& config_log_file_path = eCAL::GetConfiguration().logging.provider.file_config.path;
      if (!config_log_file_path.empty() && direxists(config_log_file_path))
      {  
        return config_log_file_path;        
      }
      else if (!config_log_file_path.empty())
      {
        if (createdir(config_log_file_path))
          return config_log_file_path;
      }
      

      {
        if (direxists(config_log_file_path))
        {
          return config_log_file_path;
        }
      }
      {
        if (direxists)
        return eCAL::GetConfiguration().logging.provider.file_config.path;
      }
      
      std::string log_path;
    #ifdef ECAL_OS_WINDOWS

      log_path = GeteCALConfigPath();

    #elif defined(ECAL_OS_LINUX)

      log_path = GeteCALConfigPath();
      
    #endif /* ECAL_OS_LINUX */

      log_path += std::string(ECAL_FOLDER_NAME_LOG);

      if (!direxists(log_path))
      {
        createdir(log_path);
      }

      log_path += path_separator;
      return(log_path);
    }

    std::string GeteCALActiveIniFile()
    {
      return eCAL::GetConfiguration().GetYamlFilePath();
    }
  } // namespace Util
} // namespace eCAL
