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
 * @brief  memory file util functions
**/

#include "../ecal_memfile.h"

namespace eCAL
{
  bool AllocMemFile(const std::string& name_, const bool /*create_*/, SMemFileInfo& mem_file_info_)
  {
    mem_file_info_.name = name_;
    mem_file_info_.size = 0;
    return(true);
  }

  bool DeAllocMemFile(SMemFileInfo& mem_file_info_)
  {
    mem_file_info_.name.clear();
    mem_file_info_.size = 0;
    return(true);
  }

  bool MapMemFile(const bool create_, SMemFileInfo& mem_file_info_)
  {
    if (mem_file_info_.map_region == nullptr)
    {
      DWORD flProtect = 0;
      if (create_)
      {
        flProtect = PAGE_READWRITE;
      }
      else
      {
        flProtect = PAGE_READONLY;
      }
      mem_file_info_.map_region = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, flProtect, 0, (DWORD)mem_file_info_.size, mem_file_info_.name.c_str());
      if (mem_file_info_.map_region == NULL) return(false);
    }

    if (mem_file_info_.mem_address == nullptr)
    {
      DWORD dwDesiredAccess = 0;
      if (create_)
      {
        dwDesiredAccess = FILE_MAP_ALL_ACCESS;
      }
      else
      {
        dwDesiredAccess = FILE_MAP_READ;
      }
      mem_file_info_.mem_address = (LPTSTR)MapViewOfFile(mem_file_info_.map_region,   // handle to map object
        dwDesiredAccess,                                                               // read/write permission
        0,
        0,
        mem_file_info_.size);
    }

    return(mem_file_info_.mem_address != nullptr);
  }

  bool UnMapMemFile(SMemFileInfo& mem_file_info_)
  {
    if (mem_file_info_.mem_address)
    {
      UnmapViewOfFile(mem_file_info_.mem_address);
      mem_file_info_.mem_address = nullptr;
    }

    if (mem_file_info_.map_region)
    {
      CloseHandle(mem_file_info_.map_region);
      mem_file_info_.map_region = nullptr;
    }

    return(true);
  }

  bool CheckMemFile(const size_t len_, const bool create_, SMemFileInfo& mem_file_info_)
  {
    if(len_ > mem_file_info_.size)
    {
      // set new file size
      mem_file_info_.size = len_;

      // unmap memory file
      UnMapMemFile(mem_file_info_);
    }

    if(mem_file_info_.mem_address == nullptr)
    {
      // map memory file
      MapMemFile(create_, mem_file_info_);

      // reset content
      if(create_ && mem_file_info_.mem_address)
      {
        memset(mem_file_info_.mem_address, 0, len_);
      }
    }

    return(mem_file_info_.mem_address != nullptr);
  }

  bool RemoveMemFile(const SMemFileInfo& /*mem_file_info_*/)
  {
    return(true);
  }

}
