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
#include "ecal/config.h"

#include "ecal/util.h"
#include "util/getenvvar.h"

#include <ecal_utils/str_convert.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <system_error>
#include <thread>
#include <vector>

// OS-specific includes for path/temp/registry APIs
#ifdef ECAL_OS_WINDOWS
  #include <windows.h>
  #include <ShlObj.h>
#endif
#ifdef ECAL_OS_LINUX
  #include <unistd.h>
#endif

namespace
{
#ifdef ECAL_OS_WINDOWS
  // Converts a UTF-8 encoded std::string to a std::filesystem::path.
  // Necessary on Windows where std::filesystem::path(std::string) uses
  // the current ANSI code page, not UTF-8.
  std::filesystem::path utf8ToPath(const std::string& utf8_str_)
  {
    return std::filesystem::path(EcalUtils::StrConvert::Utf8ToWide(utf8_str_));
  }

  // Converts a std::filesystem::path to a UTF-8 encoded std::string.
  // Uses the wide (UTF-16) representation to avoid code-page issues.
  std::string pathToUtf8(const std::filesystem::path& p_)
  {
    return EcalUtils::StrConvert::WideToUtf8(p_.wstring());
  }
#else
  std::filesystem::path utf8ToPath(const std::string& utf8_str_)
  {
    return std::filesystem::path(utf8_str_);
  }

  std::string pathToUtf8(const std::filesystem::path& p_)
  {
    return p_.string();
  }
#endif /* ECAL_OS_WINDOWS */

  // returns empty if str1_ is empty. otherwise returns str1_ / str2_ (native path separator)
  std::string buildPath(const std::string& str1_, const std::string& str2_)
  {
    if (str1_.empty()) return "";
    return pathToUtf8(utf8ToPath(str1_) / utf8ToPath(str2_));
  }

  // checks if the specified file is a proper file
  bool isValidFile(const std::string& full_file_path_)
  {
    return std::filesystem::is_regular_file(utf8ToPath(full_file_path_));
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
      WideCharToMultiByte(CP_UTF8, 0, path_tmp, -1, return_path.data(), size_needed, nullptr, nullptr);
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

  #else

    return {};

  #endif
  }

  // returns temp dir, e.g. /tmp in linux or C:\Users\username\AppData\Local\Temp in windows
  // never returns an empty string, if there is no valid temp dir found, fallback /ecal_tmp is returned
  std::string getTempDir(const eCAL::Util::IDirManager& dir_manager_)
  {
  #ifdef ECAL_OS_WINDOWS
      
      wchar_t temp_path_buffer[MAX_PATH];
      DWORD path_length = GetTempPathW(MAX_PATH, temp_path_buffer);
      if (path_length > 0 && path_length < MAX_PATH)
      {
        return EcalUtils::StrConvert::WideToUtf8(std::wstring(temp_path_buffer, path_length));
      }
      else
      {
        std::string appdata_path = getKnownFolderPath(FOLDERID_LocalAppData);
        if (!appdata_path.empty())
        {
          std::string apdata_tmp_path = buildPath(appdata_path, ECAL_FOLDER_NAME_TMP_WINDOWS);
          if (dir_manager_.dirExists(apdata_tmp_path))
          {
            return apdata_tmp_path;
          }
        }
      }
      
  #elif defined(ECAL_OS_LINUX)

      std::string env_tmp_dir = getEnvVar(ECAL_LINUX_TMP_VAR);
      if (!env_tmp_dir.empty() && dir_manager_.dirExists(env_tmp_dir))
      {
        return env_tmp_dir;
      }

  #endif /* ECAL_OS_LINUX */

    return ECAL_FALLBACK_TMP_DIR;
  }

  std::string eCALPlatformSpecificFolder(const std::string& path_, [[maybe_unused]] const std::string& linux_folder_name_ = ECAL_FOLDER_NAME_HOME_LINUX, [[maybe_unused]] const std::string& win_folder_name_ = ECAL_FOLDER_NAME_WINDOWS)
  {
    if (path_.empty()) return {};

  #ifdef ECAL_OS_WINDOWS
        
    return buildPath(path_, win_folder_name_);
        
  #elif defined(ECAL_OS_LINUX)
        
    return buildPath(path_, linux_folder_name_);
        
  #else

    return {};
    
  #endif
  }
}

namespace eCAL
{
  namespace Util
  {
    bool DirManager::dirExists(const std::string& path_) const
    {
      return std::filesystem::is_directory(utf8ToPath(path_));
    }

    bool DirManager::createDir(const std::string& path_) const
    {
      std::error_code ec;
      return std::filesystem::create_directory(utf8ToPath(path_), ec);
    }

    bool DirManager::dirExistsOrCreate(const std::string& path_) const
    {
      if (path_.empty())    return false;
      if (dirExists(path_)) return true;
      if (createDir(path_)) return true;
      return false;
    }

    bool DirManager::createEcalDirStructure(const std::string& path_) const
    {
      if (!dirExistsOrCreate(path_)) return false;

      // create also logs directory
      const std::string log_path = buildPath(path_, ECAL_FOLDER_NAME_LOG);
      return dirExistsOrCreate(log_path);
    }

    std::string DirManager::findFileInPaths(const std::vector<std::string>& paths_, const std::string& file_name_) const
    {
      auto it = std::find_if(paths_.begin(), paths_.end(), [&file_name_](const std::string& path_)
      {
        if (path_.empty())
          return false;
        
        const std::string file_path = buildPath(path_, file_name_);
        
        return isValidFile(file_path);
      });

      // We should have encountered a valid path
      if (it != paths_.end())
        return *it;

      // If valid path is not encountered, defaults should be used
      return {};
    }

    bool DirManager::canWriteToDirectory(const std::string& path_) const 
    {
      // Attempt-the-write is the most reliable cross-platform check:
      //   - std::filesystem::perms doesn't model Windows ACLs
      //   - access(W_OK) is POSIX-only
      // Use a thread-id-qualified name to avoid collisions under concurrent calls.
      const std::string test_file_name = "ecal_write_test_" + std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id())) + ".tmp";
      const std::filesystem::path test_file_path = utf8ToPath(path_) / utf8ToPath(test_file_name);
      std::ofstream test_file(test_file_path);

      if (test_file)
      {
        test_file.close();
        std::error_code ec;
        std::filesystem::remove(test_file_path, ec);
        return true;
      }

      return false;
    }

    // returns the directory path of the specified file
    std::string DirManager::getDirectoryPath(const std::string& file_path_) const
    {
      return pathToUtf8(utf8ToPath(file_path_).parent_path());
    }

    // return a unique temporary folder name
    // the folder is created and the name is returned
    // returns an empty string if the folder could not be created
    std::string DirProvider::uniqueTmpDir(const eCAL::Util::IDirManager& dir_manager_) const
    {
      const std::string tmp_dir = getTempDir(dir_manager_);
    #ifdef ECAL_OS_WINDOWS
      
      std::wstring wide_tmp_dir = EcalUtils::StrConvert::Utf8ToWide(tmp_dir);
      wchar_t unique_path[MAX_PATH];
      if (GetTempFileNameW(wide_tmp_dir.c_str(), L"ecal", 0, unique_path) == 0)
      {
        // failed to generate the path
        return {};
      }

      // delete the temporary file and use the name as a directory
      DeleteFileW(unique_path);
      if (!CreateDirectoryW(unique_path, nullptr)) {
        return {};
      }

      return EcalUtils::StrConvert::WideToUtf8(std::wstring(unique_path));
    
    #elif defined(ECAL_OS_LINUX)

      std::string path_template = buildPath(tmp_dir, "ecal-XXXXXX"); // 'X's will be replaced
      char* dir = mkdtemp(path_template.data());

      if (dir == nullptr) {
        return {};
      }

      return std::string(dir);

    #else

      return {};

    #endif
    }

    std::string DirProvider::eCALEnvVar(const std::string& var_) const
    {
      return getEnvVar(var_);
    }

    std::string DirProvider::eCALLocalUserDir() const
    {
      return eCALPlatformSpecificFolder(getLocalUserPath());
    }

    std::string DirProvider::eCALDataSystemDir([[maybe_unused]] const Util::IDirManager& dir_manager_) const
    {
      std::string system_dir;
    #ifdef ECAL_OS_WINDOWS

      system_dir = getKnownFolderPath(FOLDERID_ProgramData);
    
    #elif defined(ECAL_OS_LINUX)
    
      if (dir_manager_.dirExists(ECAL_LINUX_SYSTEM_PATH))
        system_dir = ECAL_LINUX_SYSTEM_PATH;
    
    #endif /* ECAL_OS_LINUX */

      if (system_dir.empty()) return {};
        
      return eCALPlatformSpecificFolder(system_dir, ECAL_FOLDER_NAME_LINUX);
    }
  } // namespace Util

  namespace Config
  {
    std::string GeteCALLogDirImpl(const Util::IDirProvider& dir_provider_ /* = Util::DirProvider() */, const Util::IDirManager& dir_manager_ /* = Util::DirManager() */, const eCAL::Configuration& config_ /* = eCAL::GetConfiguration() */)
    {
      const std::string config_file_dir = dir_manager_.getDirectoryPath(config_.GetConfigurationFilePath());
      const std::string ecal_data_env_dir = dir_provider_.eCALEnvVar(ECAL_DATA_VAR);
      
      const std::vector<std::string> log_paths = {
        dir_provider_.eCALEnvVar(ECAL_LOG_VAR),
        buildPath(ecal_data_env_dir, ECAL_FOLDER_NAME_LOG),
        ecal_data_env_dir,
        config_.logging.provider.file_config.path,
        buildPath(config_file_dir, ECAL_FOLDER_NAME_LOG),
        config_file_dir
      };

      for (const auto& path : log_paths)
      {
        if (!path.empty() && dir_manager_.dirExists(path) && dir_manager_.canWriteToDirectory(path))
        {
          return path;
        }
      }
      
      // if no path is available, we create temp directories for logging
      // check now for a tmp directory and return
      return dir_provider_.uniqueTmpDir(dir_manager_);
    }

    std::string checkForValidConfigFilePath(const std::string& config_file_, const Util::DirProvider& dir_provider_ /* = Util::DirProvider() */, const Util::DirManager& dir_manager_ /* = Util::DirManager() */)
    {
      std::error_code ec;
      const std::string cwd_directory_path = pathToUtf8(std::filesystem::current_path(ec));

      std::vector<std::string> ecal_default_paths = getEcalDefaultPaths(dir_provider_, dir_manager_);
      
      // insert cwd on 2nd position, so that ECAL_DATA dir has precedence
      // skip if CWD could not be determined (e.g. directory was deleted)
      if (!ec)
        ecal_default_paths.insert(ecal_default_paths.begin() + 1, cwd_directory_path);
      
      const std::string found_path = dir_manager_.findFileInPaths(ecal_default_paths, config_file_);

      // check in case user provided whole path
      if (found_path.empty())
      {
        return isValidFile(config_file_) ? config_file_ : found_path;
      }

      return buildPath(found_path, config_file_);
    }

    std::vector<std::string> getEcalDefaultPaths(const Util::IDirProvider& dir_provider_ /* = DirProvider() */, const Util::IDirManager& dir_manager_  /* = Util::DirManager() */)
    {
      std::vector<std::string> ecal_default_paths;
      // -----------------------------------------------------------
      // precedence 1: ECAL_DATA variable (windows and linux)
      // -----------------------------------------------------------
      ecal_default_paths.emplace_back(dir_provider_.eCALEnvVar(ECAL_DATA_VAR));

      // -----------------------------------------------------------
      // precedence 2: local user path 
      // -----------------------------------------------------------
      ecal_default_paths.emplace_back(dir_provider_.eCALLocalUserDir());

      // -----------------------------------------------------------
      // precedence 3: eCAL data system path 
      // -----------------------------------------------------------
      ecal_default_paths.emplace_back(dir_provider_.eCALDataSystemDir(dir_manager_));

      return ecal_default_paths;
    }

    std::string GeteCALDataDirImpl(const Util::IDirProvider& dir_provider /* = DirProvider() */, const Util::IDirManager& dir_manager_ /* = DirManager() */)
    {
      // Return the possible default paths that could contain the yaml file
      const std::vector<std::string> search_directories = getEcalDefaultPaths(dir_provider, dir_manager_);
      
      // return the first non empty path that also exists
      for (const auto& path : search_directories)
      {
        if (!path.empty() && dir_manager_.dirExists(path)) return path;
      }

      return {};
    }
  } // namespace Config
  
} // namespace eCAL
