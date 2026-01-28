/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 * Copyright 2026 AUMOVIO and subsidiaries. All rights reserved.
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
 * @brief  eCAL memory file map (database) handling
**/

#pragma once

#include <cstddef>
#include <mutex>
#include <string>
#include <unordered_map>

#include "ecal_memfile_info.h"

namespace eCAL
{
  class CMemFileMap
  {
  public:
    CMemFileMap() = default;
    ~CMemFileMap();

    void Stop();

    bool AddFile(const std::string& name_, bool create_, size_t len_, std::shared_ptr<SMemFileInfo>& memfile_info_);
    bool RemoveFile(const std::string& name_, bool remove_);
    bool CheckFileSize(size_t len_, std::shared_ptr<SMemFileInfo>& memfile_info_);

  protected:
    using MemFileMapT = std::unordered_map<std::string, std::shared_ptr<SMemFileInfo>>;
    std::mutex  m_memfile_map_mtx;
    MemFileMapT m_memfile_map;
  };
}
