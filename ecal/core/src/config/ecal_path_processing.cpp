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
  #include <dlfcn.h>
#endif

namespace
{
  // Converts a UTF-8 encoded std::string to a std::filesystem::path.
  // std::filesystem::u8path() is the standard C++17 way to do this correctly
  // on all platforms, including Windows where path(std::string) uses the ANSI
  // code page rather than UTF-8.
  std::filesystem::path utf8ToPath(const std::string& utf8_str_)
  {
    return std::filesystem::u8path(utf8_str_);
  }

  // Converts a std::filesystem::path to a UTF-8 encoded std::string.
  std::string pathToUtf8(const std::filesystem::path& p_)
  {
    return p_.u8string();
  }

  // returns empty if str1_ is empty. otherwise returns str1_ / str2_ (native path separator)
  std::string buildPath(const std::string& str1_, const std::string& str2_)
  {
    if (str1_.empty()) return {};
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
    PWSTR path_tmp = nullptr;

    // Retrieve the known folder path: users local app data
    auto ret = SHGetKnownFolderPath(id_, 0, nullptr, &path_tmp);

    if (ret != S_OK)
    {
      CoTaskMemFree(path_tmp); // safe to call with nullptr
      return {};
    }

    // Construct a path from the wide string and convert to UTF-8.
    std::string return_path = std::filesystem::path(path_tmp).u8string();

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
  std::string getTempDir()
  {
    std::error_code ec;
    const std::filesystem::path tmp = std::filesystem::temp_directory_path(ec);
    if (!ec && !tmp.empty())
      return pathToUtf8(tmp);
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

  std::string getLibraryPath(const eCAL::Util::IDirManager& dir_manager_) 
  {
    std::string return_path = {};
  #ifdef ECAL_OS_WINDOWS
    
    HMODULE hModule = NULL;
    char path[MAX_PATH];
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)getLibraryPath, &hModule);
    GetModuleFileName(hModule, path, sizeof(path));
    return_path = dir_manager_.getDirectoryPath(path);
  
  #elif defined(ECAL_OS_LINUX)
    
    Dl_info dl_info;
    if (dladdr((void *)getLibraryPath, &dl_info)) 
    {
      return_path = dir_manager_.getDirectoryPath(dl_info.dli_fname);
    }

  #endif

    return return_path;
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
      const std::string tmp_dir = getTempDir();

      // Ensure the base tmp directory exists (e.g. when falling back to /ecal_tmp)
      if (!dir_manager_.dirExistsOrCreate(tmp_dir))
        return {};

    #ifdef ECAL_OS_WINDOWS
      
      std::wstring wide_tmp_dir = utf8ToPath(tmp_dir).wstring();
      wchar_t unique_path_buf[MAX_PATH];
      if (GetTempFileNameW(wide_tmp_dir.c_str(), L"ecal", 0, unique_path_buf) == 0)
      {
        // failed to generate the path
        return {};
      }

      // GetTempFileNameW creates a file to reserve the name; remove it so we
      // can create a directory with the same name instead.
      const std::filesystem::path unique_path(unique_path_buf);
      std::error_code ec;
      std::filesystem::remove(unique_path, ec);
      if (ec) return {};

      std::filesystem::create_directory(unique_path, ec);
      if (ec) return {};

      return pathToUtf8(unique_path);
    
    #elif defined(ECAL_OS_LINUX)

      std::string path_template = buildPath(tmp_dir, "ecal-XXXXXX"); // 'X's will be replaced
      char* dir = mkdtemp(path_template.data());

      if (dir == nullptr)
        return {};

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

    std::string DirProvider::eCALLibraryDir(const Util::IDirManager& dir_manager_) const
    {
      return getLibraryPath(dir_manager_);
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
          return path;
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

      // -----------------------------------------------------------
      // precedence 4: library path
      // -----------------------------------------------------------
      const std::string library_dir = dir_provider_.eCALLibraryDir(dir_manager_);
      if (!library_dir.empty())
      {
        ecal_default_paths.emplace_back(buildPath(library_dir, ECAL_FOLDER_RELATIVE_ETC));
      }

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
