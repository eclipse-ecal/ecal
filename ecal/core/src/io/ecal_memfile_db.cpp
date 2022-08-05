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
 * @brief  eCAL global memory file map (database) handling
**/

#include "ecal_global_accessors.h"
#include "ecal_memfile_os.h"
#include "ecal_memfile_db.h"

#include <cassert>

namespace eCAL
{
  CMemFileMap::~CMemFileMap()
  {
    Destroy();
  }

  void CMemFileMap::Destroy()
  {
    // lock memory map access
    std::lock_guard<std::mutex> lock(m_memfile_map_mtx);

    // erase memory files from memory map
    for (MemFileMapT::iterator iter = m_memfile_map.begin(); iter != m_memfile_map.end(); ++iter)
    {
      auto& memfile_info = iter->second;

      // unmap memory file
      memfile::os::UnMapFile(memfile_info);

      // remove memory file from system
      if (memfile_info.remove) memfile::os::RemoveFile(memfile_info);

      // deallocate memory file
      memfile::os::DeAllocFile(memfile_info);
    }

    // clear map
    m_memfile_map.clear();
  }

  bool CMemFileMap::AddFile(const std::string& name_, const bool create_, const size_t len_, SMemFileInfo& mem_file_info_)
  {
    // we need a length != 0
    assert(len_ > 0);

    // lock memory map access
    std::lock_guard<std::mutex> lock(m_memfile_map_mtx);

    // check for existing memory file
    MemFileMapT::iterator iter = m_memfile_map.find(name_);
    if (iter == m_memfile_map.end())
    {
      // create memory file
      if (!memfile::os::AllocFile(name_, create_, mem_file_info_))
      {
#ifndef NDEBUG
        printf("Could create memory file: %s.\n\n", name_.c_str());
#endif
        return(false);
      }

      // check memory file size
      memfile::os::CheckFileSize(len_, create_, mem_file_info_);

      // and add to memory file map
      mem_file_info_.refcnt++;
      m_memfile_map[name_] = mem_file_info_;
    }
    else
    {
      // increase reference counter
      iter->second.refcnt++;

      // check memory file size
      memfile::os::CheckFileSize(len_, false, iter->second);

      // copy info from memory file map
      mem_file_info_ = iter->second;
    }

    // return success
    return(true);
  }

  bool CMemFileMap::RemoveFile(const std::string& name_, const bool remove_)
  {
    // lock memory map access
    std::lock_guard<std::mutex> lock(m_memfile_map_mtx);

    // erase memory file from memory map
    auto& memfile_map = m_memfile_map;
    MemFileMapT::iterator iter = memfile_map.find(name_);
    if (iter != memfile_map.end())
    {
      auto& memfile_info = iter->second;

      // decrease reference counter
      memfile_info.refcnt--;
      // mark for remove
      memfile_info.remove |= remove_;
      if (memfile_info.refcnt < 1)
      {
        bool remove_from_system = memfile_info.remove;

        // unmap memory file
        memfile::os::UnMapFile(memfile_info);

        // remove memory file from system
        if (remove_from_system) memfile::os::RemoveFile(memfile_info);

        // dealloc memory file
        memfile::os::DeAllocFile(memfile_info);

        memfile_map.erase(iter);
      }

      // we removed the file (or marked it for later removal)
      return(true);
    }

    return(false);
  }

  bool CMemFileMap::CheckFileSize(const std::string& name_, const size_t len_, SMemFileInfo& mem_file_info_)
  {
    // check and correct file size
    memfile::os::CheckFileSize(len_, false, mem_file_info_);

    // lock memory map access
    std::lock_guard<std::mutex> lock(m_memfile_map_mtx);

    // update/set info
    m_memfile_map[name_] = mem_file_info_;

    return(true);
  }

  namespace memfile
  {
    namespace db
    {
      bool AddFile(const std::string& name_, const bool create_, const size_t len_, SMemFileInfo& mem_file_info_)
      {
        if (!g_memfile_map()) return false;
        return g_memfile_map()->AddFile(name_, create_, len_, mem_file_info_);
      }

      bool RemoveFile(const std::string& name_, const bool remove_)
      {
        if (!g_memfile_map()) return false;
        return g_memfile_map()->RemoveFile(name_, remove_);
      }

      bool CheckFileSize(const std::string& name_, const size_t len_, SMemFileInfo& mem_file_info_)
      {
        if (!g_memfile_map()) return false;
        return g_memfile_map()->CheckFileSize(name_, len_, mem_file_info_);
      }
    }
  }
}
