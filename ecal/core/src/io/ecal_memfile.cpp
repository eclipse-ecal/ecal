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
 * @brief  memory file interface
**/

#include <ecal/ecal_os.h>

#include "ecal_def.h"
#include "ecal_memfile.h"

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <assert.h>

#ifdef ECAL_OS_LINUX

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#endif /* ECAL_OS_LINUX */

namespace eCAL
{
  /////////////////////////////////////////////////////////////////////////////////
  // Memory file handling local help functions
  /////////////////////////////////////////////////////////////////////////////////

  bool CreateMemFile(const std::string& name_, const bool create_, const size_t len_, SMemFileInfo& mem_file_info_);
  bool AllocMemFile(const std::string& name_, const bool create_, SMemFileInfo& mem_file_info_);
  bool MapMemFile(const bool create_, SMemFileInfo& mem_file_info_);
  bool CheckMemFile(const size_t len_, const bool create_, SMemFileInfo& mem_file_info_);
  bool UnMapMemFile(SMemFileInfo& mem_file_info_);
  bool DeAllocMemFile(SMemFileInfo& mem_file_info_);
  bool DestroyMemFile(const std::string& name_, const bool remove_);
  bool RemoveMemFile(const SMemFileInfo& mem_file_info_);


  /////////////////////////////////////////////////////////////////////////////////
  // Memory file handling class
  /////////////////////////////////////////////////////////////////////////////////

  CMemoryFile::CMemoryFile() :
    m_created(false),
    m_opened(false),
    m_name(""),
    m_memfile_info(new SMemFileInfo)
  {
  }

  CMemoryFile::~CMemoryFile()
  {
    Close();
    Destroy(false);

    delete m_memfile_info;
  }

  bool CMemoryFile::Create(const char* name_, const bool create_, const size_t len_)
  {
    assert((create_ && len_ > 0) || (!create_ && len_ == 0));

    // do we have to recreate the file ?
    if(   (m_name != name_)
      || (
      create_
      && (len_ > 0)
      && (m_header.max_data_size != (unsigned long)len_)
      )
      )
    {
      // destroy existing connection
      Destroy(create_);

      // reset states
      m_created              = false;
      m_opened               = false;
      m_name.clear();
      m_memfile_info->mutex  = nullptr;
      m_header.max_data_size = 0;
      m_header.cur_data_size = 0;

      // create memory file
      if(!CreateMemFile(name_, create_, len_ + sizeof(SMemFileHeader), *m_memfile_info))
      {
#ifndef NDEBUG
        printf("Could not create memory file: %s.\n\n", name_);
#endif
        return(false);
      }
    }

    // create mutex
    if(!CreateMtx(name_, m_memfile_info->mutex))
    {
#ifndef NDEBUG
      printf("Could not create memory file mutex: %s.\n\n", name_);
#endif
      return(false);
    }

    if(create_)
    {
      // create header
      m_header.max_data_size = (unsigned long)len_;

      // lock mutex
      if(LockMtx(&m_memfile_info->mutex, PUB_MEMFILE_CREATE_TO))
      {
        if(m_memfile_info->mem_address)
        {
          // write header
          SMemFileHeader* pHeader = new (m_memfile_info->mem_address) SMemFileHeader;
          if(pHeader) *pHeader = m_header;
        }

        // unlock mutex
        UnlockMtx(&m_memfile_info->mutex);
      }
    }
    else
    {
      // lock mutex
      if(LockMtx(&m_memfile_info->mutex, PUB_MEMFILE_CREATE_TO))
      {
        // read header
        SMemFileHeader* pHeader = static_cast<SMemFileHeader*>(m_memfile_info->mem_address);
        m_header = *pHeader;

        // unlock mutex
        UnlockMtx(&m_memfile_info->mutex);
      }
    }

    // set states
    m_created = true;
    m_name    = name_;

    return(m_created);
  }

  bool CMemoryFile::Destroy(const bool remove_)
  {
    if(!m_created)      return(false);
    if(!m_memfile_info) return(false);

    // return state
    bool ret_state = true;

    // destroy memory file
    ret_state &= DestroyMemFile(m_name, remove_);

    // unlock mutex
    ret_state &= DestroyMtx(&m_memfile_info->mutex);

    // reset states
    m_created                   = false;
    m_opened                    = false;
    m_name.clear();
    m_header.max_data_size      = 0;
    m_header.cur_data_size      = 0;

    m_memfile_info->mutex       = 0;
    m_memfile_info->memfile     = 0;
    m_memfile_info->map_region  = 0;
    m_memfile_info->mem_address = 0;

    return(ret_state);
  }

  bool CMemoryFile::Open(int timeout_)
  {
    if(m_opened)                     return(true);
    if(!m_created)                   return(false);
    if(!m_memfile_info->mem_address) return(false);
    if(!g_memfile_map())             return(false);

    // lock mutex
    if(!LockMtx(&m_memfile_info->mutex, timeout_))
    {
#ifndef NDEBUG
      printf("Could not lock memory file mutex: %s.\n\n", m_name.c_str());
#endif
      return(false);
    }

    // update header
    SMemFileHeader* pHeader = static_cast<SMemFileHeader*>(m_memfile_info->mem_address);
    m_header = *pHeader;

    // check size again
    size_t len = static_cast<size_t>(m_header.hdr_size) + static_cast<size_t>(m_header.max_data_size);
    if(len > m_memfile_info->size)
    {
      // check memory file size
      CheckMemFile(len, false, *m_memfile_info);

      // update memory file map
      {
        std::lock_guard<std::mutex> lock(g_memfile_map()->sync);
        g_memfile_map()->map[m_name] = *m_memfile_info;
      }

      // check size again and give up if it is still to small
      if(len > m_memfile_info->size)
      {
        // unlock mutex
        UnlockMtx(&m_memfile_info->mutex);
        return(false);
      }
    }

    // mark as opened
    m_opened = true;

    return(true);
  }

  bool CMemoryFile::Close()
  {
    if(!m_opened)  return(true);
    if(!m_created) return(false);

    // unlock mutex
    UnlockMtx(&m_memfile_info->mutex);

    // reset states
    m_opened = false;

    return(true);
  }

  size_t CMemoryFile::Read(void* buf_, const size_t len_, const size_t offset_)
  {
    if(!m_opened)                                                         return(0);
    if(!buf_)                                                             return(0);
    if(len_ == 0)                                                         return(0);
    if(!m_memfile_info->mem_address)                                      return(0);
    if((len_ + offset_ + sizeof(SMemFileHeader)) > m_memfile_info->size)  return(0);

    // read content
    memcpy(buf_, static_cast<char*>(m_memfile_info->mem_address) + offset_ + sizeof(SMemFileHeader), len_);

    return(len_);
  }

  size_t CMemoryFile::Read(CMemConsumer& cons_, const size_t offset_)
  {
    if(!m_opened)                     return(0);
    if(!m_memfile_info->mem_address)  return(0);

    cons_.ReadBuffer(static_cast<char*>(m_memfile_info->mem_address) + offset_ + sizeof(SMemFileHeader), static_cast<size_t>(m_header.cur_data_size));
    return static_cast<size_t>(m_header.cur_data_size);
  }

  size_t CMemoryFile::Write(const void* buf_, const size_t len_, const size_t offset_)
  {
    if(!m_opened)                                                         return(0);
    if(!buf_)                                                             return(0);
    if((len_ + offset_) > static_cast<size_t>(m_header.max_data_size))    return(0);
    if(!m_memfile_info->mem_address)                                      return(0);
    if((len_ + offset_ + sizeof(SMemFileHeader)) > m_memfile_info->size)  return(0);

    // update header
    m_header.cur_data_size = (unsigned long)(len_ + offset_);
    SMemFileHeader* pHeader = static_cast<SMemFileHeader*>(m_memfile_info->mem_address);
    pHeader->cur_data_size = m_header.cur_data_size;

    // write content
    memcpy(static_cast<char*>(m_memfile_info->mem_address) + offset_ + sizeof(SMemFileHeader), buf_, len_);

    return(len_);
  }

  size_t CMemoryFile::Write(CMemProducer& prod_, const size_t len_, const size_t offset_)
  {
    if(!m_opened) return(0);

    size_t len = len_;
    if(len == 0) len = prod_.GetSize();
    if(len == 0)                                                         return(0);

    if(!m_memfile_info->mem_address)                                     return(0);
    if((len + offset_ + sizeof(SMemFileHeader)) > m_memfile_info->size)  return(0);

    // update header
    m_header.cur_data_size = (unsigned long)(len_ + offset_);
    SMemFileHeader* pHeader = static_cast<SMemFileHeader*>(m_memfile_info->mem_address);
    pHeader->cur_data_size = m_header.cur_data_size;

    // write content
    prod_.WriteBuffer(static_cast<char*>(m_memfile_info->mem_address) + offset_ + sizeof(SMemFileHeader));

    return(len);
  }

  void CleanupMemoryFileMap()
  {
    if (!g_memfile_map()) return;

    // lock memory map access
    std::lock_guard<std::mutex> lock(g_memfile_map()->sync);

    // erase memory files from memory map
    auto& memfile_map = g_memfile_map()->map;
    for(MemFileMapT::iterator iter = memfile_map.begin(); iter != memfile_map.end(); ++iter)
    {
      auto& memfile_info = iter->second;

      // unmap memory file
      UnMapMemFile(memfile_info);

      // remove memory file from system
      if(memfile_info.remove) RemoveMemFile(memfile_info);

      // deallocate memory file
      DeAllocMemFile(memfile_info);
    }

    // clear map
    memfile_map.clear();
  }

  /////////////////////////////////////////////////////////////////////////////////
  // Memory file handling help functions
  /////////////////////////////////////////////////////////////////////////////////

  bool CreateMemFile(const std::string& name_, const bool create_, const size_t len_, SMemFileInfo& mem_file_info_)
  {
    if (!g_memfile_map()) return(false);

    // we need a length != 0
    assert(len_ > 0);

    // lock memory map access
    std::lock_guard<std::mutex> lock(g_memfile_map()->sync);

    // check for existing memory file
    MemFileMapT::iterator iter = g_memfile_map()->map.find(name_);
    if(iter == g_memfile_map()->map.end())
    {
      // create memory file
      if(!AllocMemFile(name_, create_, mem_file_info_))
      {
#ifndef NDEBUG
        printf("Could create memory file: %s.\n\n", name_.c_str());
#endif
        return(false);
      }

      // check memory file size
      CheckMemFile(len_, create_, mem_file_info_);

      // and add to memory file map
      mem_file_info_.refcnt++;
      g_memfile_map()->map[name_] = mem_file_info_;
    }
    else
    {
      // increase reference counter
      iter->second.refcnt++;

      // check memory file size
      CheckMemFile(len_, false, iter->second);

      // copy info from memory file map
      mem_file_info_ = iter->second;
    }

    // return success
    return(true);
  }

  bool DestroyMemFile(const std::string& name_, const bool remove_)
  {
    if (!g_memfile_map()) return(false);

    // lock memory map access
    std::lock_guard<std::mutex> lock(g_memfile_map()->sync);

    // erase memory file from memory map
    auto& memfile_map = g_memfile_map()->map;
    MemFileMapT::iterator iter = memfile_map.find(name_);
    if(iter != memfile_map.end())
    {
      auto& memfile_info = iter->second;

      // decrease reference counter
      memfile_info.refcnt--;
      // mark for remove
      memfile_info.remove |= remove_;
      if(memfile_info.refcnt < 1)
      {
        bool remove_from_system = memfile_info.remove;

        // unmap memory file
        UnMapMemFile(memfile_info);

        // remove memory file from system
        if(remove_from_system) RemoveMemFile(memfile_info);

        // dealloc memory file
        DeAllocMemFile(memfile_info);

        memfile_map.erase(iter);

        // we removed the file
        return(true);
      }
    }

    return(false);
  }

#ifdef ECAL_OS_WINDOWS

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

  bool MapMemFile(const bool create_, SMemFileInfo& mem_file_info_)
  {
    if(mem_file_info_.map_region == nullptr)
    {
      DWORD flProtect = 0;
      if(create_)
      {
        flProtect = PAGE_READWRITE;
      }
      else
      {
        flProtect = PAGE_READONLY;
      }
      mem_file_info_.map_region = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, flProtect, 0 ,(DWORD)mem_file_info_.size, mem_file_info_.name.c_str());
      if(mem_file_info_.map_region == NULL) return(false);
    }

    if(mem_file_info_.mem_address == nullptr)
    {
      DWORD dwDesiredAccess = 0;
      if(create_)
      {
        dwDesiredAccess = FILE_MAP_ALL_ACCESS;
      }
      else
      {
        dwDesiredAccess = FILE_MAP_READ;
      }
      mem_file_info_.mem_address = (LPTSTR) MapViewOfFile(mem_file_info_.map_region,   // handle to map object
        dwDesiredAccess,                                                               // read/write permission
        0,
        0,
        mem_file_info_.size);
    }

    return(mem_file_info_.mem_address != nullptr);
  }

  bool UnMapMemFile(SMemFileInfo& mem_file_info_)
  {
    if(mem_file_info_.mem_address)
    {
      UnmapViewOfFile(mem_file_info_.mem_address);
      mem_file_info_.mem_address = nullptr;
    }

    if(mem_file_info_.map_region)
    {
      CloseHandle(mem_file_info_.map_region);
      mem_file_info_.map_region = nullptr;
    }

    return(true);
  }

  bool RemoveMemFile(const SMemFileInfo& /*mem_file_info_*/)
  {
    return(true);
  }

#endif /* ECAL_OS_WINDOWS */

#ifdef ECAL_OS_LINUX

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
    mem_file_info_.memfile = ::shm_open(name_.c_str(), oflag, 0666);
    if(mem_file_info_.memfile == -1)
    {
      mem_file_info_.memfile = 0;
      std::cout << "shm_open failed : " << name_ << " errno: " << strerror(errno) << std::endl;
      return(false);
    }

    mem_file_info_.name = name_;
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

  bool RemoveMemFile(const SMemFileInfo& mem_file_info_)
  {
    ::shm_unlink(mem_file_info_.name.c_str());
    return(true);
  }

#endif /* ECAL_OS_LINUX */
}
