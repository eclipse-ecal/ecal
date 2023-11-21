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
 * @brief  eCAL memory file broadcast interface
**/

#pragma once

#include "ecal_memfile_broadcast.h"

#include <cstdint>
#include <memory>

namespace eCAL
{
  class CMemoryFileBroadcastWriter
  {
  public:
    bool Bind(CMemoryFileBroadcast *memfile_broadcast);
    void Unbind();

    bool Write(const void *data, std::size_t size);
  private:
    CMemoryFileBroadcast *m_memfile_broadcast = nullptr;
    std::unique_ptr<CMemoryFile> m_payload_memfile;
    std::uint64_t m_event_id = 0;
    bool m_bound = false;
    bool m_reset = false;
  };
}