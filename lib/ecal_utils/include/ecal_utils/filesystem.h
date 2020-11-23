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

#pragma once

#include <string>
#include <vector>
#include <map>

#include <sys/stat.h>

namespace EcalUtils
{
  namespace Filesystem
  {
    enum OsStyle : int
    {
      Combined = 0,
      Windows  = 1,
      Unix     = 2,

#if defined _WIN32
      Current  = Windows,
#else
      Current  = Unix,
#endif
    };

    enum Type : int
    {
      Unknown,
      RegularFile,
      Dir,
      CharacterDevice,
      BlockDevice,
      Fifo,
      SymbolicLink,
      Socket
    };

    class FileStatus
    {
    public:
      FileStatus(const std::string& path, OsStyle input_path_style = OsStyle::Combined);

      ~FileStatus();

      bool IsOk() const;
      Type GetType() const;

      int64_t FileSize() const;

      bool PermissionRootRead()     const;
      bool PermissionRootWrite()    const;
      bool PermissionRootExecute()  const;
      bool PermissionGroupRead()    const;
      bool PermissionGroupWrite()   const;
      bool PermissionGroupExecute() const;
      bool PermissionOwnerRead()    const;
      bool PermissionOwnerWrite()   const;
      bool PermissionOwnerExecute() const;

      bool CanOpenDir() const;

    private:
      std::string path_;
      bool is_ok_;
#ifdef WIN32
      struct __stat64 file_status_;
#else // WIN32
      struct stat file_status_;
#endif 
    };

    Type GetType(const std::string& path, OsStyle input_path_style = OsStyle::Combined);

    bool IsDir(const std::string& path, OsStyle input_path_style = OsStyle::Combined);
    bool IsFile(const std::string& path, OsStyle input_path_style = OsStyle::Combined);
    std::map<std::string, FileStatus> DirContent(const std::string& path, OsStyle input_path_style = OsStyle::Combined);

    bool MkDir(const std::string& path, OsStyle input_path_style = OsStyle::Combined);
    bool MkPath(const std::string& path, OsStyle input_path_style = OsStyle::Combined);

    bool CopyFile(const std::string& source, const std::string& destination, OsStyle input_path_style = OsStyle::Combined);

    bool DeleteDir(const std::string& source, OsStyle input_path_style = OsStyle::Combined);

    std::string GetAbsoluteRoot(const std::string& path, OsStyle input_path_style = OsStyle::Combined);

    bool IsAbsolute(const std::string& path, OsStyle input_path_style = OsStyle::Combined);
    bool IsRelative(const std::string& path, OsStyle input_path_style = OsStyle::Combined);

    std::string CleanPath(const std::string& path, OsStyle input_path_style = OsStyle::Combined);
    std::vector<std::string> CleanPathComponentList(const std::string& path, OsStyle input_path_style = OsStyle::Combined);

    std::string AbsolutePath(const std::string& base_path, const std::string& relative_path, OsStyle input_path_style = OsStyle::Combined);
    std::string AbsolutePath(const std::string& relative_path, OsStyle input_path_style = OsStyle::Combined);

    std::string RelativePath(const std::string& base_path, const std::string& path, OsStyle input_path_style = OsStyle::Combined);

    std::string CurrentWorkingDir();
    std::string ApplicationDir();

    std::string ChangeSeperators(const std::string& path, OsStyle output_path_style, OsStyle input_path_style = OsStyle::Combined);

    std::string ToUnixSeperators(const std::string& path, OsStyle input_path_style = OsStyle::Combined);
    std::string ToNativeSeperators(const std::string& path, OsStyle input_path_style = OsStyle::Combined);

    inline char NativeSeparator(OsStyle seperator_style = OsStyle::Current)
    {
      switch (seperator_style)
      {
      case OsStyle::Windows:
        return '\\';
      default:
        return '/';
      }
    }

    bool IsEqual(const std::string& path1, const std::string& path2, OsStyle compare_for = OsStyle::Current);

    std::string FileName(const std::string& path, OsStyle input_path_style = OsStyle::Combined);

    std::string BaseName(const std::string& path, OsStyle input_path_style = OsStyle::Combined);
  };
}
