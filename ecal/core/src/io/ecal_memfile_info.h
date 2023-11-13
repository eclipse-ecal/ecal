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
#include <memory>

#include <ecal/ecal_os.h>

#ifdef ECAL_OS_WINDOWS

#include "ecal_win_main.h"

using MemFileT = HANDLE;
using MapRegionT = HANDLE;

#endif /* ECAL_OS_WINDOWS */

#ifdef ECAL_OS_LINUX

typedef int  MemFileT;
typedef int  MapRegionT;

#endif /* ECAL_OS_LINUX */

namespace eCAL
{
  struct SMemFileInfo
  {
    int          refcnt      = 0;
    bool         remove      = false;
    MemFileT     memfile     = 0;
    MapRegionT   map_region  = 0;
    void*        mem_address = 0;
    std::string  name;
    size_t       size        = 0;
    bool         exists      = false;
  };
}
