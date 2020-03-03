/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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

#include "EcalUtils/Path.h"

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX
  #include <windows.h>
  #include <direct.h>
#else // _WIN32
  #include <dirent.h>
#endif  // _WIN32

#include <sys/stat.h> // stat
#include <errno.h>    // errno, ENOENT, EEXIST

#include <list>
#include <regex>
#include <iostream>

#include "EcalUtils/EcalUtils.h"

namespace EcalUtils
{
  namespace Path
  {
    ////////////////////////////////////////
    // FileStatus Class
    ////////////////////////////////////////

    FileStatus::FileStatus(const std::string& path, OsStyle input_path_style)
    {
      path_ = CleanPath(path, input_path_style);
#ifdef WIN32
      const int error_code = _stat64(ToNativeSeperators(path, input_path_style).c_str(), &file_status_);
#else // WIN32
      const int error_code = stat(ToNativeSeperators(path, input_path_style).c_str(), &file_status_);
#endif // WIN32
      is_ok_ = (error_code == 0);
    }

    FileStatus::~FileStatus() {}

    bool FileStatus::IsOk() const
    {
      return is_ok_;
    }

    Type FileStatus::GetType() const
    {
      if (!is_ok_)
        return Type::Unknown;

      switch (file_status_.st_mode & S_IFMT) {
      case S_IFREG:  return Type::RegularFile;
      case S_IFDIR:  return Type::Dir;
      case S_IFCHR:  return Type::CharacterDevice;
#ifndef WIN32
      case S_IFBLK:  return Type::BlockDevice;
      case S_IFIFO:  return Type::Fifo;
      case S_IFLNK:  return Type::SymbolicLink;
      case S_IFSOCK: return Type::Socket;
#endif // !WIN32
      default:       return Type::Unknown;
      }

    }

    int64_t FileStatus::FileSize() const
    {
      if (!is_ok_)
        return 0;

      return file_status_.st_size;
    }

#ifdef WIN32
    bool FileStatus::PermissionRootRead()     const { return 0 != (file_status_.st_mode & S_IREAD); }
    bool FileStatus::PermissionRootWrite()    const { return 0 != (file_status_.st_mode & S_IWRITE); }
    bool FileStatus::PermissionRootExecute()  const { return 0 != (file_status_.st_mode & S_IEXEC); }
    bool FileStatus::PermissionGroupRead()    const { return 0 != (file_status_.st_mode & S_IREAD); }
    bool FileStatus::PermissionGroupWrite()   const { return 0 != (file_status_.st_mode & S_IWRITE); }
    bool FileStatus::PermissionGroupExecute() const { return 0 != (file_status_.st_mode & S_IEXEC); }
    bool FileStatus::PermissionOwnerRead()    const { return 0 != (file_status_.st_mode & S_IREAD); }
    bool FileStatus::PermissionOwnerWrite()   const { return 0 != (file_status_.st_mode & S_IWRITE); }
    bool FileStatus::PermissionOwnerExecute() const { return 0 != (file_status_.st_mode & S_IEXEC); }
#else // WIN32
    bool FileStatus::PermissionRootRead()     const { return 0 != (file_status_.st_mode & S_IRUSR); }
    bool FileStatus::PermissionRootWrite()    const { return 0 != (file_status_.st_mode & S_IWUSR); }
    bool FileStatus::PermissionRootExecute()  const { return 0 != (file_status_.st_mode & S_IXUSR); }
    bool FileStatus::PermissionGroupRead()    const { return 0 != (file_status_.st_mode & S_IRGRP); }
    bool FileStatus::PermissionGroupWrite()   const { return 0 != (file_status_.st_mode & S_IWGRP); }
    bool FileStatus::PermissionGroupExecute() const { return 0 != (file_status_.st_mode & S_IXGRP); }
    bool FileStatus::PermissionOwnerRead()    const { return 0 != (file_status_.st_mode & S_IROTH); }
    bool FileStatus::PermissionOwnerWrite()   const { return 0 != (file_status_.st_mode & S_IWOTH); }
    bool FileStatus::PermissionOwnerExecute() const { return 0 != (file_status_.st_mode & S_IXOTH); }
#endif // WIN32

    bool FileStatus::CanOpenDir() const
    {
      if (!is_ok_)
        return false;

      if (GetType() != Type::Dir)
        return false;

      bool can_open_dir(false);
#ifdef WIN32
      std::string find_file_path = path_ + "\\*";
      std::replace(find_file_path.begin(), find_file_path.end(), '/', '\\');

      HANDLE hFind;
      WIN32_FIND_DATAA ffd;
      hFind = FindFirstFileA(find_file_path.c_str(), &ffd);
      if (hFind != INVALID_HANDLE_VALUE)
      {
        can_open_dir = true;
      }
      FindClose(hFind);
#else // WIN32
      DIR *dp;
      if ((dp = opendir(path_.c_str())) != NULL)
      {
        can_open_dir = true;
      }
      closedir(dp);
#endif // WIN32

      return can_open_dir;
    }

    ////////////////////////////////////////
    // Path Functions
    ////////////////////////////////////////

    Type GetType(const std::string& path, OsStyle input_path_style)
    {
      FileStatus file_status(path, input_path_style);
      return file_status.GetType();
    }

    bool IsDir(const std::string& path, OsStyle input_path_style)
    {
      return GetType(path, input_path_style) == Type::Dir;
    }

    bool IsFile(const std::string& path, OsStyle input_path_style)
    {
      return GetType(path, input_path_style) == Type::RegularFile;
    }

    std::map<std::string, FileStatus> DirContent(const std::string& path, OsStyle input_path_style)
    {
      std::string clean_path = ToNativeSeperators(CleanPath(path, input_path_style), input_path_style);

      std::map<std::string, FileStatus> content;
#ifdef WIN32
      std::string find_file_path = clean_path + "\\*";
      std::replace(find_file_path.begin(), find_file_path.end(), '/', '\\');

      HANDLE hFind;
      WIN32_FIND_DATAA ffd;
      hFind = FindFirstFileA(find_file_path.c_str(), &ffd);
      if (hFind == INVALID_HANDLE_VALUE)
      {
        std::cerr << "FindFirstFile Error" << std::endl;
        return content;
      }

      do
      {
        std::string file_name(ffd.cFileName);
        content.emplace(std::string(ffd.cFileName), FileStatus(clean_path + "\\" + std::string(ffd.cFileName)));
      } while (FindNextFileA(hFind, &ffd) != 0);
      FindClose(hFind);
#else // WIN32
      DIR *dp;
      struct dirent *dirp;
      if ((dp = opendir(clean_path.c_str())) == NULL)
      {
        std::cerr << "Error opening directory: " << strerror(errno) << std::endl;
        return content;
      }

      while ((dirp = readdir(dp)) != NULL)
      {
        content.emplace(std::string(dirp->d_name), FileStatus(clean_path + "/" + std::string(dirp->d_name)));
      }
      closedir(dp);

#endif // WIN32
      return content;
    }

    bool MkDir(const std::string& path, OsStyle input_path_style)
    {
      std::string native_path = ChangeSeperators(path, OsStyle::Current, input_path_style);

#if defined(_WIN32)
      int ret = _mkdir(native_path.c_str());
#else
      mode_t mode = 0755;
      int ret = mkdir(native_path.c_str(), mode);
#endif
      return ret == 0;
    }

    bool MkPath(const std::string& path, OsStyle input_path_style)
    {
      std::string native_path = ChangeSeperators(CleanPath(path, input_path_style), OsStyle::Current, input_path_style);

      if (native_path.empty())
      {
        return false;
      }
      else if (native_path.back() == NativeSeparator(OsStyle::Current))
      {
        // Remove tailing separator
        native_path.pop_back();
      }

      // Create last directory
      if (MkDir(native_path, OsStyle::Current))
      {
        return true;
      }

      // If creating last directory did not work, we may need to create the parent
      switch (errno)
      {
      case ENOENT:
        // parent didn't exist, try to create it
      {
        size_t pos = native_path.find_last_of(NativeSeparator(OsStyle::Current));
        if (pos == std::string::npos)
          return false;
        if (!MkPath(native_path.substr(0, pos), OsStyle::Current))
          return false;
      }

      // now, try to create again
      return MkDir(native_path, OsStyle::Current);

      case EEXIST:
        // done!
        return IsDir(native_path, OsStyle::Current);

      default:
        return false;
      }
    }

    std::string GetAbsoluteRoot(const std::string& path, OsStyle input_path_style)
    {
      if ((input_path_style == OsStyle::Windows) || (input_path_style == OsStyle::Combined))
      {
        std::regex win_drive_with_slash("^[a-zA-Z]\\:[/\\\\]");    // Drive with tailing separator and optional directory
        std::regex win_drive_only("^[a-zA-Z]\\: *$");              // Drive only, after wich must not come anything
        std::regex win_network_drive("^[/\\\\]{2}[^/\\\\]+");      // Network path starting with two slashes or backslashes

        if ((std::regex_search(path, win_drive_with_slash)
          || std::regex_search(path, win_drive_only)))
        {
          // Windows local drive, consisting of drive-letter and colon
          return path.substr(0, 2);
        }
        else if (std::regex_search(path, win_network_drive))
        {
          // Window network drive, consisting of \\ and hostname
          size_t sep_pos = path.find_first_of("/\\", 2);
          return path.substr(0, sep_pos); // If no seperator was found, this will return the entire string
        }
      }

      if (input_path_style == OsStyle::Unix)
      {
        std::regex unix_slash("^/");                               // Normal Unix root
        if (std::regex_search(path, unix_slash))
        {
          return path.substr(0, 1);
        }
      }
      else if (input_path_style == OsStyle::Combined)
      {
        std::regex unix_slash("^[/\\\\]");                         // Normal Unix root (with optional windows seperator indicating the root)
        if (std::regex_search(path, unix_slash))
        {
          return path.substr(0, 1);
        }
      }

      return "";
    }

    bool IsAbsolute(const std::string& path, OsStyle input_path_style)
    {
      return GetAbsoluteRoot(path, input_path_style) != "";
    }

    bool IsRelative(const std::string& path, OsStyle input_path_style)
    {
      if (path.empty())
      {
        return false;
      }
      else
      {
        return !IsAbsolute(path, input_path_style);
      }
    }

    std::list<std::string> CleanPathComponentList(const std::string& path, OsStyle input_path_style)
    {
      if (path.empty())
      {
        return{};
      }

      std::list<std::string> components;

      std::string unix_style_path = ToUnixSeperators(path, input_path_style);

      // Get information about the root
      std::string root = GetAbsoluteRoot(path, OsStyle::Combined);
      bool is_absolute = (root != "");

      // Remove the root from the path (we will add it later)
      unix_style_path = unix_style_path.substr(root.size());

      // Split the relative path into its components
      std::list<std::string> splitted_path;
      EcalUtils::String::Split(unix_style_path, "/", splitted_path);

      // The components-stack that will increase and shrink depending on the folders and .. elements in the splitted path
      for (const std::string& part : splitted_path)
      {
        if (part.empty() || part == ".")
        {
          continue;
        }
        else if (part == "..")
        {
          if (is_absolute)
          {
            if (!components.empty())
            {
              // Move one folder up if we are not already at the root
              components.pop_back();
            }
          }
          else
          {
            if (!components.empty() && (components.back() != ".."))
            {
              // Move one folder up by removing it. We must not remove ".." elements that we were not able to resolve previously.
              components.pop_back();
            }
            else
            {
              components.push_back("..");
            }
          }
        }
        else
        {
          components.push_back(part);
        }
      }

      return components;
    }


    std::string CleanPath(const std::string& path, OsStyle input_path_style)
    {
      if (path.empty())
        return ".";

      // Split the path into its cleaned components
      std::list<std::string> cleaned_path_components = CleanPathComponentList(path, input_path_style);

      // Check whether the path ended with a slash
      bool tailing_separator = ((path.back() == '/') || (path.back() == '\\'));

      // Gather information about the root (it will not be in the components list)
      std::string root = GetAbsoluteRoot(path, input_path_style);
      root = ToUnixSeperators(root, input_path_style);
      bool is_absolute = (root != "");

      std::string cleaned_path;
      cleaned_path.reserve(path.size() + root.size() + 2);

      if (is_absolute)
      {
        cleaned_path += root;
        if (cleaned_path.back() != '/')
        {
          cleaned_path += '/';
        }
      }
      else
      {
        cleaned_path += '.';
        if (!cleaned_path_components.empty())
        {
          cleaned_path += '/';
        }
      }

      cleaned_path += EcalUtils::String::Join("/", cleaned_path_components);

      if (tailing_separator && (cleaned_path.back() != '/'))
      {
        cleaned_path += '/';
      }

      return cleaned_path;
    }

    std::string AbsolutePath(const std::string& base_path, const std::string& relative_path, OsStyle input_path_style)
    {
      return CleanPath(CleanPath(base_path) + "/" + relative_path, input_path_style);
    }

    std::string AbsolutePath(const std::string& relative_path, OsStyle input_path_style)
    {
      if (IsAbsolute(relative_path, input_path_style))
      {
        return CleanPath(relative_path, input_path_style);
      }
      else
      {
        return CleanPath(CurrentWorkingDir() + "/" + relative_path);
      }
    }

    std::string CurrentWorkingDir()
    {
#ifdef _WIN32
      char working_dir[MAX_PATH];
      return (_getcwd(working_dir, MAX_PATH) ? working_dir : std::string(""));
#else
      char working_dir[PATH_MAX];
      return (getcwd(working_dir, PATH_MAX) ? working_dir : std::string(""));
#endif
    }

    std::string ChangeSeperators(const std::string& path, OsStyle output_path_style, OsStyle input_path_style)
    {
      std::string output;
      output.reserve(path.size());

      for (size_t i = 0; i < path.size(); i++)
      {
        if ((((input_path_style == OsStyle::Windows) || (input_path_style == OsStyle::Combined)) && (path[i] == '\\'))
          || (path[i] == '/'))
        {
          output += NativeSeparator(output_path_style);
        }
        else
        {
          output += path[i];
        }
      }

      return output;
    }

    std::string ToUnixSeperators(const std::string& path, OsStyle input_path_style)
    {
      return ChangeSeperators(path, OsStyle::Unix, input_path_style);
    }

    std::string ToNativeSeperators(const std::string& path, OsStyle input_path_style)
    {
      return ChangeSeperators(path, OsStyle::Current, input_path_style);
    }

    bool IsEqual(const std::string& path1, const std::string& path2, OsStyle compare_for)
    {
      std::string clean_path1 = ToNativeSeperators(AbsolutePath(path1, compare_for), compare_for);
      std::string clean_path2 = ToNativeSeperators(AbsolutePath(path2, compare_for), compare_for);

      // Compare the root
      std::string path1_root = GetAbsoluteRoot(clean_path1, compare_for);
      std::string path2_root = GetAbsoluteRoot(clean_path2, compare_for);

      if (compare_for == OsStyle::Windows)
      {
        // Windows is case-insensitive
        // cause warning C4244 with VS2017, VS2019
        //std::transform(path1_root.begin(), path1_root.end(), path1_root.begin(), ::tolower);
        std::transform(path1_root.begin(), path1_root.end(), path1_root.begin(),
          [](char c) {return static_cast<char>(::tolower(c)); });
        // cause warning C4244 with VS2017, VS2019
        //std::transform(path2_root.begin(), path2_root.end(), path2_root.begin(), ::tolower);
        std::transform(path2_root.begin(), path2_root.end(), path2_root.begin(),
          [](char c) {return static_cast<char>(::tolower(c)); });
      }

      if (path1_root != path2_root)
      {
        return false;
      }

      // Compare the rest
      auto path1_components = CleanPathComponentList(clean_path1, compare_for);
      auto path2_components = CleanPathComponentList(clean_path2, compare_for);

      if (path1_components.size() != path2_components.size())
      {
        return false;
      }

      auto path1_component_it = path1_components.begin();
      auto path2_component_it = path2_components.begin();

      while (path1_component_it != path1_components.end())
      {
        if (compare_for == OsStyle::Windows)
        {
          // Windows is case-insensitive
          // cause warning C4244 with VS2017, VS2019
          //std::transform(path1_component_it->begin(), path1_component_it->end(), path1_component_it->begin(), ::tolower);
          std::transform(path1_component_it->begin(), path1_component_it->end(), path1_component_it->begin(),
            [](char c) {return static_cast<char>(::tolower(c)); });
          // cause warning C4244 with VS2017, VS2019
          //std::transform(path2_component_it->begin(), path2_component_it->end(), path2_component_it->begin(), ::tolower);
          std::transform(path2_component_it->begin(), path2_component_it->end(), path2_component_it->begin(),
            [](char c) {return static_cast<char>(::tolower(c)); });
        }

        if (*path1_component_it != *path2_component_it)
        {
          return false;
        }

        path1_component_it++;
        path2_component_it++;
      }

      return true;
    }
  }
}
