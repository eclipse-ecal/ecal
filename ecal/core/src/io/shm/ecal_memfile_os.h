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
 * @brief  memory file utility functions os specific
**/

#pragma once

#include <string>

#include "ecal_memfile.h"

namespace eCAL
{
  namespace memfile
  {
    namespace os
    {
      bool AllocFile(const std::string& name_, const bool create_, SMemFileInfo& mem_file_info_);
      bool DeAllocFile(SMemFileInfo& mem_file_info_);
      bool RemoveFile(const SMemFileInfo& mem_file_info_);

      bool MapFile(const bool create_, SMemFileInfo& mem_file_info_);
      bool UnMapFile(SMemFileInfo& mem_file_info_);

      bool CheckFileSize(const size_t len_, const bool create_, SMemFileInfo& mem_file_info_);
    }
  }
}
