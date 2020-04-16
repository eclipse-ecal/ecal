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

/**
 * eCALHDF5 utilities
**/

#pragma once

#include <string>
#include <regex>
#include <list>

#ifdef WIN32
#if defined(_MSC_VER) && defined(__clang__) && !defined(CINTERFACE)
#define CINTERFACE
#endif
#include <windows.h>
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif  // WIN32

namespace Utility
{
  namespace Path
  {
    enum class eType
    {
      unknown,
      directory,
      file
    };

    inline eType GetType(const std::string& path)
    {
      eType type = eType::unknown;
#ifdef WIN32
      DWORD attribs = ::GetFileAttributesA(path.c_str());
      if (attribs == INVALID_FILE_ATTRIBUTES)
        type = eType::unknown;
      else if ((attribs & FILE_ATTRIBUTE_DIRECTORY) != 0)
        type = eType::directory;
      else
        type = eType::file;
#else
      struct stat s;
      if (stat(path.c_str(), &s) == 0)
      {
        if (s.st_mode & S_IFDIR)
          type = eType::directory;
        else if (s.st_mode & S_IFREG)
          type = eType::file;
      }
#endif  // WIN32

      return type;
    }
  }

  namespace Directory
  {
    /**
     * @brief Checks if directory exists
     *
     * @param path   directory path.
     *
     * @return  true if exists, false if it does not.
    **/
    inline bool Exists(const std::string& path)
    {
#ifdef ECAL_OS_WINDOWS
      DWORD attribs = ::GetFileAttributesA(path.c_str());
      if (attribs == INVALID_FILE_ATTRIBUTES)
        return false;

      return (attribs & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
      struct stat st;
      if (stat(path.c_str(), &st) == 0)
        if ((st.st_mode & S_IFDIR) != 0)
          return true;
      return false;
#endif  //  ECAL_OS_WINDOWS
    }

    /**
     * @brief Creates directory structure
     *
     * @param path   directory path.
     *
     * @return      true if succeeds, false if it fails.
    **/
    inline bool CreateDirectories(std::string path)
    {
      size_t pre = 0, position;
      std::string directory;

      path += '/';

      std::replace(path.begin(), path.end(), '\\', '/');

      std::regex reg("\\/+");
      path = std::regex_replace(path, reg, std::string("/"));

      while ((position = path.find_first_of('/', pre)) != std::string::npos)
      {
        directory = path.substr(0, position++);
        pre = position;
        if (directory.size() == 0)
          continue;
#ifdef WIN32
        _mkdir(directory.c_str());
#else
        mkdir(directory.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif  //  WIN32
      }
      return Exists(path);
    }

  }  // namespace Directory

  namespace String
  {
    inline bool icharcompare(char a, char b)
    {
      return(toupper(a) == toupper(b));
    }

    inline bool icompare(const std::string& s1, const std::string& s2)
    {
      return((s1.size() == s2.size()) &&
        equal(s1.begin(), s1.end(), s2.begin(), icharcompare));
    }

    inline void split(const std::string& str, const std::string& delim, std::list<std::string>& parts)
    {
      size_t end = 0;
      while (end < str.size())
      {
        size_t start = end;
        while (start < str.size() && (delim.find(str[start]) != std::string::npos))
        {
          start++;
        }
        end = start;
        while (end < str.size() && (delim.find(str[end]) == std::string::npos))
        {
          end++;
        }
        if (end - start != 0)
        {
          parts.push_back(std::string(str, start, end - start));
        }
      }
    }

  }  // namespace String
}   // namespace Utility
