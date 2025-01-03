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
#endif

namespace
{
  // get the path separator from the current OS (win: "\\", unix: "/")
  const std::string path_separator(1, EcalUtils::Filesystem::NativeSeparator());

  std::string buildPath(const std::string& str1_, const std::string& str2_)
  {
    return EcalUtils::String::Join(path_separator, std::vector<std::string>{str1_, str2_});
  }

  // checks if the specified file is a proper file
  bool isValidFile(const std::string& full_file_path_)
  {
    const EcalUtils::Filesystem::FileStatus file_status(full_file_path_, EcalUtils::Filesystem::Current);
    return file_status.IsOk() && (file_status.GetType() == EcalUtils::Filesystem::Type::RegularFile);
  }

  // combines path and file and checks with isValidFile
  bool isValidFilePath(const std::string& path_, const std::string& file_name_)
  {
    const std::string file_path = buildPath(path_, file_name_);
    return isValidFile(file_path);
  }

  // Returns path, where the specified ini file is found, or empty string if not found
  std::string findValidFilePath(std::vector<std::string> paths_, const std::string& file_name_)
  {
    auto it = std::find_if(paths_.begin(), paths_.end(), [&file_name_](const std::string& path_)
    {
      if (path_.empty())
        return false;
      
      return isValidFilePath(path_, file_name_);
    });

    // We should have encountered a valid path
    if (it != paths_.end())
      return (*it);

    // If valid path is not encountered, defaults should be used
    return {};
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
  #ifdef ECAL_OS_WINDOWS
    
    return getKnownFolderPath(FOLDERID_LocalAppData);
  
  #elif defined(ECAL_OS_LINUX)
    
    return getEnvVar(ECAL_LINUX_HOME_VAR);

  #endif /* ECAL_OS_LINUX */

    return {};
  }

  std::string getSystemDir()
  {
  #ifdef ECAL_OS_WINDOWS

    return getKnownFolderPath(FOLDERID_ProgramData);
  
  #elif defined(ECAL_OS_LINUX)
  
    // TODO PG: Check if we really want to give that back here
    if (dirExists(ECAL_LINUX_SYSTEM_PATH)) 
      return ECAL_LINUX_SYSTEM_PATH;
  
  #endif /* ECAL_OS_LINUX */
    return {};
  }

  // returns temp dir, e.g. /tmp in linux or C:\Users\username\AppData\Local\Temp in windows
  // never returns an empty string, if there is no valid temp dir found, fallback /ecal_tmp is returned
  std::string getTempDir()
  {
  #ifdef ECAL_OS_WINDOWS
      
      char temp_path_buffer[MAX_PATH];
      DWORD path_length = GetTempPathA(MAX_PATH, temp_path_buffer);
      if (path_length > 0 && path_length < MAX_PATH)
      {
        return std::string(temp_path_buffer, path_length);
      }
      else
      {
        std::string appdata_path = getKnownFolderPath(FOLDERID_LocalAppData);
        if (!appdata_path.empty())
        {
          std::string apdata_tmp_path = buildPath(appdata_path, ECAL_FOLDER_NAME_TMP_WINDOWS);
          if (eCAL::Util::dirExists(apdata_tmp_path))
          {
            return apdata_tmp_path;
          }
        }
      }
      
  #elif defined(ECAL_OS_LINUX)

      std::string env_tmp_dir = getEnvVar(ECAL_LINUX_TMP_VAR);
      if (!env_tmp_dir.empty() && eCAL::Util::dirExists(env_tmp_dir))
      {
        return env_tmp_dir;
      }

  #endif /* ECAL_OS_LINUX */

    return ECAL_FALLBACK_TMP_DIR;
  }

  // return a unique temporary folder name
  // the folder is created and the name is returned
  // returns an empty string if the folder could not be created
  std::string createUniqueTmpFolderName()
  {
    const std::string tmp_dir = getTempDir();
  #ifdef ECAL_OS_WINDOWS
    
    char unique_path[MAX_PATH];
    if (!GetTempFileNameA(tmp_dir.c_str(), "ecal", 0, unique_path) != 0)
    {
      // failed to generate the path
      return {};
    }

    // delete the temporary file and use the name as a directory
    DeleteFileA(unique_path);
    if (!CreateDirectoryA(unique_path, nullptr)) {
       return {};
    }

    return std::string(unique_path);
  
  #elif defined(ECAL_OS_LINUX)

    std::string path_template = buildPath(tmp_dir, "ecal-XXXXXX"); // 'X's will be replaced
    char* dir = mkdtemp(&path_template[0]);

    if (dir == nullptr) {
      return {};
    }

    return std::string(dir);

  #endif
  }

  std::string eCALPlatformSpecificFolder(const std::string& path_, const std::string& linux_folder_name_ = ECAL_FOLDER_NAME_HOME_LINUX, const std::string& win_folder_name_ = ECAL_FOLDER_NAME_WINDOWS)
  {
    if (path_.empty()) return {};

  #ifdef ECAL_OS_WINDOWS
        
    return buildPath(path_, win_folder_name_);
        
  #elif defined(ECAL_OS_LINUX)
        
    return buildPath(path_, linux_folder_name_);
        
  #endif
  }
}

namespace eCAL
{
  namespace Util
  {
    bool dirExists(const std::string& path_)
    {
      const EcalUtils::Filesystem::FileStatus status(path_, EcalUtils::Filesystem::Current);
      return (status.IsOk() && (status.GetType() == EcalUtils::Filesystem::Type::Dir));
    }

    bool createDir(const std::string& path_)
    {
      return EcalUtils::Filesystem::MkDir(path_, EcalUtils::Filesystem::Current);
    }

    bool dirExistsOrCreate(const std::string& path_)
    {
      if (path_.empty())    return false;
      if (dirExists(path_)) return true;
      if (createDir(path_)) return true;
      return false;
    }

    // Returns the default paths for possible ecal configurations
    // Order:
    // 1. ECAL_DATA environment varible path if set
    // 2. local user path
    // 3. system path like etc, ProgramData
    std::vector<std::string> getEcalDefaultPaths()
    {
      std::vector<std::string> ecal_default_paths;
      // -----------------------------------------------------------
      // precedence 1: ECAL_DATA variable (windows and linux)
      // -----------------------------------------------------------
      ecal_default_paths.emplace_back(eCAL::Config::eCALDataEnvPath());

      // -----------------------------------------------------------
      // precedence 2: local user path 
      // -----------------------------------------------------------
      ecal_default_paths.emplace_back(eCAL::Config::eCALLocalUserDir());

      // -----------------------------------------------------------
      // precedence 3: eCAL data system path 
      // -----------------------------------------------------------
      ecal_default_paths.emplace_back(eCAL::Config::eCALDataSystemDir());
      return ecal_default_paths;
    }
  } // namespace Util

  namespace Config
  {
    std::string eCALLocalUserDir()
    {
      const std::string userspace_path = getLocalUserPath();

      if (!userspace_path.empty())
      {
        return eCALPlatformSpecificFolder(userspace_path);
      }
      
      return {};
    }

    std::string eCALDataEnvPath()
    {
      return getEnvVar(ECAL_DATA_VAR);
    }

    std::string eCALLogDir()
    {
      const std::vector<const std::string> log_paths = {
        getEnvVar(ECAL_LOG_VAR),
        eCALDataEnvPath(),
        eCAL::GetConfiguration().logging.provider.file_config.path,
        eCALLocalUserDir(),
      
      // only system dir on windows is writable for standard user (ProgramData)
      #ifdef ECAL_OS_WINDOWS
        EcalUtils::String::Join(path_separator, std::vector<std::string>{getSystemDir(), ECAL_FOLDER_NAME_WINDOWS, ECAL_FOLDER_NAME_LOG})
      #endif
      };

      for (const auto& path : log_paths)
      {
        if (!path.empty())
        {
          if (Util::dirExists(path))
          {
            return path;
          }
          else
          {
            std::cout << "[eCAL] Log path does not exist: " << path << std::endl;
          }
        }
      }
      
      // if no path is available, we create temp directories for logging
      // check now for a tmp directory and return
      return createUniqueTmpFolderName();
    }

    std::string eCALDataSystemDir()
    {
      const std::string system_path = getSystemDir();

      if (!system_path.empty())
      {
        return eCALPlatformSpecificFolder(system_path, ECAL_FOLDER_NAME_LINUX);
      }
 
      return {};
    }

    std::string checkForValidConfigFilePath(const std::string& config_file_)
    {
      const std::string cwd_directory_path = EcalUtils::Filesystem::CurrentWorkingDir();

      std::vector<std::string> ecal_default_paths = Util::getEcalDefaultPaths();
      
      // insert cwd on 2nd position, so that ECAL_CONFIG_DIR dir has precedence
      ecal_default_paths.insert(ecal_default_paths.begin() + 1, cwd_directory_path);
      
      const std::string found_path = findValidFilePath(ecal_default_paths, config_file_);

      // check in case user provided whole path
      if (found_path.empty())
      {
        return isValidFile(config_file_) ? config_file_ : found_path;
      }

      return buildPath(found_path, config_file_);
    }

    bool createEcalDirStructure(const std::string& path_)
    {
        if (!Util::dirExistsOrCreate(path_)) return false;

        // create also logs directory
        const std::string log_path = buildPath(path_, ECAL_FOLDER_NAME_LOG);
        return Util::dirExistsOrCreate(log_path);
    }
  } // namespace Config
  
} // namespace eCAL
