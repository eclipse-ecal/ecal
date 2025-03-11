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

#include "../include/windows/memory.h"

Memory::Memory()
{
  RefreshData();
}


Memory::~Memory()
{
}

void Memory::RefreshData()
{
  memory_info_.dwLength = sizeof(MEMORYSTATUSEX);
  GlobalMemoryStatusEx(&memory_info_);

  total_memory_ = static_cast<uint64_t>(memory_info_.ullTotalPhys);
  available_memory_ = static_cast<uint64_t>(memory_info_.ullAvailPhys);
}
