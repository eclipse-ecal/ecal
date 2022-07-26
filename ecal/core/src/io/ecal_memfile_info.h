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
 * @brief  eCAL memory file info structure
**/

#pragma once

#include <string>

#include <ecal/ecal_os.h>
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
}
