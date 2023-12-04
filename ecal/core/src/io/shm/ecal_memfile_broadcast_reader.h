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
#include <vector>
#include <unordered_map>
#include <memory>

namespace eCAL
{
  struct SMemfileBroadcastMessage
    {
    const void *data;
    std::size_t size;
    std::int64_t timestamp;
  };

  using MemfileBroadcastMessageListT = std::vector<SMemfileBroadcastMessage>;

  class CMemoryFileBroadcastReader
  {
    struct SPayloadMemfile
    {
      std::shared_ptr <CMemoryFile> payload_memfile;
      std::vector<char> payload_memfile_buffer;
      std::int64_t timestamp;
    };
  public:
    bool Bind(CMemoryFileBroadcast *memfile_broadcast);
    bool Unbind();

    bool Read(MemfileBroadcastMessageListT &memfile_broadcast_message_list, std::int64_t timeout);
  private:
    CMemoryFileBroadcast *m_memfile_broadcast = nullptr;
    std::unordered_map<std::uint64_t, SPayloadMemfile> m_payload_memfiles;
    MemfileBroadcastEventListT m_broadcast_event_list;
    bool m_bound = false;
  };
}