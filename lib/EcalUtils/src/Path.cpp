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
#endif  // _WIN32

#include <sys/stat.h> // stat
#include <errno.h>    // errno, ENOENT, EEXIST

#include <list>
#include <regex>

#include "EcalUtils/EcalUtils.h"

namespace EcalUtils
{
  Path::Type Path::GetType(const std::string& path, OsStyle input_path_style)
  {
    std::string native_path = ToNativeSeperators(path, input_path_style);
    
#ifdef _WIN32
    DWORD attribs = ::GetFileAttributesA(path.c_str());
    if (attribs == INVALID_FILE_ATTRIBUTES)
      return Path::Type::Unknown;
    else if (attribs & FILE_ATTRIBUTE_DIRECTORY)
      return Path::Type::Dir;
    else
      return Path::Type::File;

#elif __linux__
    struct stat s;
    if (stat(path.c_str(), &s) == 0)
    {
      if (s.st_mode & S_IFDIR)
        return Path::Type::Dir;
      else if (s.st_mode & S_IFREG)
        return Path::Type::File;
    }
    return Path::Type::Unknown;
#endif  //  __linux__
  }

  bool Path::IsDir(const std::string& path, OsStyle input_path_style)
  {
    return GetType(path, input_path_style) == Type::Dir;
  }

  bool Path::IsFile(const std::string& path, OsStyle input_path_style)
  {
    return GetType(path, input_path_style) == Type::File;
  }

  bool Path::MkDir(const std::string& path, OsStyle input_path_style)
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

  bool Path::MkPath(const std::string& path, OsStyle input_path_style)
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

  std::string Path::GetAbsoluteRoot(const std::string& path, OsStyle input_path_style)
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

  bool Path::IsAbsolute(const std::string& path, OsStyle input_path_style)
  {
    return GetAbsoluteRoot(path, input_path_style) != "";
  }

  bool Path::IsRelative(const std::string& path, OsStyle input_path_style)
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

  std::list<std::string> Path::CleanPathComponentList(const std::string& path, OsStyle input_path_style)
  {
    if (path.empty())
    {
      return {};
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


  std::string Path::CleanPath(const std::string& path, OsStyle input_path_style)
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

  std::string Path::AbsolutePath(const std::string& base_path, const std::string& relative_path, OsStyle input_path_style)
  {
    return CleanPath(CleanPath(base_path) + "/" + relative_path, input_path_style);
  }

  std::string Path::AbsolutePath(const std::string& relative_path, OsStyle input_path_style)
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

  std::string Path::CurrentWorkingDir()
  {
#ifdef _WIN32
    char working_dir[MAX_PATH];
    return (_getcwd(working_dir, MAX_PATH) ? working_dir : std::string(""));
#else
    char working_dir[PATH_MAX];
    return (getcwd(working_dir, PATH_MAX) ? working_dir : std::string(""));
#endif
  }

  std::string Path::ChangeSeperators(const std::string& path, OsStyle output_path_style, OsStyle input_path_style)
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

  std::string Path::ToUnixSeperators(const std::string& path, OsStyle input_path_style)
  {
    return ChangeSeperators(path, OsStyle::Unix, input_path_style);
  }

  std::string Path::ToNativeSeperators(const std::string& path, OsStyle input_path_style)
  {
    return ChangeSeperators(path, OsStyle::Current, input_path_style);
  }

  char Path::NativeSeparator(OsStyle seperator_style)
  {
    switch (seperator_style)
    {
    case OsStyle::Windows:
      return '\\';
    default:
      return '/';
    }
  }

  bool Path::IsEqual(const std::string& path1, const std::string& path2, OsStyle compare_for)
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
