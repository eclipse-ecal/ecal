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

#include <iostream>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

namespace eCAL
{
  bool AllocMemFile(const std::string& name_, const bool create_, SMemFileInfo& mem_file_info_)
  {
    int oflag = 0;
    if(create_)
    {
      oflag = O_CREAT | O_RDWR;
    }
    else
    {
      oflag = O_RDONLY;
    }
    int previous_umask = umask(000);  // set umask to nothing, so we can create files with all possible permission bits
    mem_file_info_.name = name_.size() ? ( (name_[0] != '/') ? "/" + name_ : name_) : name_; // make memory file path compatible for all posix systems
    mem_file_info_.memfile = ::shm_open(mem_file_info_.name.c_str(), oflag, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    umask(previous_umask);            // reset umask to previous permissions
    if(mem_file_info_.memfile == -1)
    {
      std::cout << "shm_open failed : " << mem_file_info_.name << " errno: " << strerror(errno) << std::endl;
      mem_file_info_.memfile = 0;
      mem_file_info_.name = "";
      return(false);
    }

    mem_file_info_.size = 0;

    return(true);
  }

  bool DeAllocMemFile(SMemFileInfo& mem_file_info_)
  {
    if(mem_file_info_.memfile)
    {
      ::close(mem_file_info_.memfile);
      mem_file_info_.memfile = 0;
    }

    mem_file_info_.name = "";
    mem_file_info_.size = 0;

    return(true);
  }

  bool MapMemFile(const bool create_, SMemFileInfo& mem_file_info_)
  {
    if(mem_file_info_.mem_address == nullptr)
    {
      if(create_)
      {
        // truncate file
        if(::ftruncate(mem_file_info_.memfile, mem_file_info_.size) != 0)
        {
          std::cout << "ftruncate failed : " << mem_file_info_.name  << " errno: " << strerror(errno) << std::endl;
        }
      }

      // get address
      int         prot  = PROT_READ;
      if(create_) prot |= PROT_WRITE;

      mem_file_info_.mem_address = ::mmap(nullptr, mem_file_info_.size, prot, MAP_SHARED, mem_file_info_.memfile, 0);
      if(mem_file_info_.mem_address == MAP_FAILED)
      {
        mem_file_info_.mem_address = nullptr;
        std::cout << "mmap failed : " << mem_file_info_.name  << " errno: " << strerror(errno) << std::endl;
        return(false);
      }
    }

    return(true);
  }

  bool UnMapMemFile(SMemFileInfo& mem_file_info_)
  {
    if(mem_file_info_.mem_address)
    {
      ::munmap(mem_file_info_.mem_address, mem_file_info_.size);
      mem_file_info_.mem_address = nullptr;
      return(true);
    }

    return(false);
  }

  bool CheckMemFile(const size_t len_, const bool create_, SMemFileInfo& mem_file_info_)
  {
    if(mem_file_info_.memfile == 0) return(false);

    size_t len = len_;
    if(len < (size_t) sysconf(_SC_PAGE_SIZE))
    {
      len = sysconf(_SC_PAGE_SIZE);
    }

    if(mem_file_info_.mem_address == nullptr)
    {
      // set file size
      mem_file_info_.size = len;

      // map memory file
      MapMemFile(create_, mem_file_info_);
    }
    else
    {
      // length changed ..
      if(len > mem_file_info_.size)
      {
        // unmap memory file
        UnMapMemFile(mem_file_info_);

        // set new file size
        mem_file_info_.size = len;

        // and map memory file again
        MapMemFile(create_, mem_file_info_);

        // reset content
        if(create_ && mem_file_info_.mem_address)
        {
          memset(mem_file_info_.mem_address, 0, len);
        }
      }
    }

    return(true);
  }

  bool RemoveMemFile(const SMemFileInfo& mem_file_info_)
  {
    ::shm_unlink(mem_file_info_.name.c_str());
    return(true);
  }

}
