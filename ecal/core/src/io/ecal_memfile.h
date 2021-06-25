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
 * @brief  eCAL memory file interface
**/

#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <mutex>

#include <ecal/ecal_os.h>
#include <ecal_global_accessors.h>
#include "ecal_memfile_mtx.h"

#ifdef ECAL_OS_WINDOWS

#include "ecal_win_main.h"

typedef HANDLE  MemFileT;
typedef HANDLE  MapRegionT;

#endif /* ECAL_OS_WINDOWS */

#ifdef ECAL_OS_LINUX

typedef int  MemFileT;
typedef int  MapRegionT;

#endif /* ECAL_OS_LINUX */

namespace eCAL
{
  struct SMemFileInfo
  {
    SMemFileInfo()
    {
      refcnt      = 0;
      remove      = false;
      mutex       = 0;
      memfile     = 0;
      map_region  = 0;
      mem_address = 0;
      size        = 0;
    }
    int          refcnt;
    bool         remove;
    MutexT       mutex;
    MemFileT     memfile;
    MapRegionT   map_region;
    void*        mem_address;
    std::string  name;
    size_t       size;
  };

  typedef std::unordered_map<std::string, SMemFileInfo> MemFileMapT;
  struct SMemFileMap
  {
    std::mutex  sync;
    MemFileMapT map;
  };

  /**
   * @brief Shared memory file handler class. 
  **/
  class CMemoryFile
  {
  public:
    /**
     * @brief Constructor. 
    **/
    CMemoryFile();

    /**
     * @brief Destructor. 
    **/
    ~CMemoryFile();

    /**
     * @brief Create a new memory file. 
     *
     * @param name_    Unique file name. 
     * @param create_  Add file to system if not exists.
     * @param len_     Number of bytes to allocate (only if add_ == true). 
     *
     * @return  true if it succeeds, false if it fails. 
    **/
    bool Create(const char* name_, const bool create_, const size_t len_ = 0);

    /**
     * @brief Delete the associated memory file from system. 
     *
     * @param remove_  Remove file from system.
     *
     * @return  true if it succeeds, false if it fails. 
    **/
    bool Destroy(const bool remove_);

    /**
     * @brief Open the memory file. 
     *
     * @param timeout_  The timeout in ms for access via mutex.
     *
     * @return  true if file exists and could be opened. 
    **/
    bool Open(const int timeout_);

    /**
     * @brief Closes the current opened memory file. 
     *
     * @return  true if it succeeds, false if it fails. 
    **/
    bool Close();

    /**
     * @brief Read bytes from an opened memory file. 
     *
     * @param buf_     The destination address. 
     * @param len_     The length of the allocated memory (has to be allocated by caller). 
     * @param offset_  The offset where to start reading. 
     *
     * @return         Number of copied bytes (or zero if it fails). 
    **/
    size_t Read(void* buf_, const size_t len_, const size_t offset_);

    /**
     * @brief Write bytes to the memory file. 
     *
     * @param buf_     The source address. 
     * @param len_     The number of bytes to write. 
     * @param offset_  The offset for writing the data. 
     *
     * @return         Number of bytes copied to the memory file. 
    **/
    size_t Write(const void* buf_, const size_t len_, const size_t offset_);
    
    /**
     * @brief Get payload buffer pointer from an opened memory file.
     *
     * @param buf_     The destination address.
     * @param len_     Expected length of the available payload.
     * @param offset_  The offset where to start reading.
     *
     * @return         Number of copied bytes (or zero if it fails).
    **/
    size_t GetBuffer(const void*& buf_, const size_t len_, const size_t offset_);

    /**
     * @brief Maximum data size of the whole memory file.
     *
     * @return  The size of the data object. 
    **/
    size_t FileSize() const {return static_cast<size_t>(m_header.max_data_size);};

    /**
     * @brief Size of the stored data object (can be smaller than the size
     *        of the whole memory file.
     *
     * @return  The size of the data object. 
    **/
    size_t DataSize()  const {return static_cast<size_t>(m_header.cur_data_size);};

    bool IsCreated()   const {return(m_created);};
    bool IsOpened()    const {return(m_opened);};
    std::string Name() const {return(m_name);};

    struct SMemFileHeader
    {
      SMemFileHeader()
      {
        hdr_size      = sizeof(SMemFileHeader);
        cur_data_size = 0;
        max_data_size = 0;
      };
      unsigned short hdr_size;
      unsigned long  cur_data_size;
      unsigned long  max_data_size;
    };

  protected:
    bool            m_created;
    bool            m_opened;
    std::string     m_name;
    SMemFileHeader  m_header;
    SMemFileInfo*   m_memfile_info;

  private:
    CMemoryFile(const CMemoryFile&);                 // prevent copy-construction
    CMemoryFile& operator=(const CMemoryFile&);      // prevent assignment
  };
};
