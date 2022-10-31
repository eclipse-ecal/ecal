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
 * @brief  eCAL common memory file broadcast
**/

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstring>
#include <cstdint>

#include "relocatable_circular_queue.h"

#include <ecal/ecal.h>

namespace eCAL 
{

  class CMemoryFile;

  typedef std::int64_t TimestampT;
  typedef std::uint64_t UniqueIdT;
  typedef UniqueIdT GroupIdT;

  static inline TimestampT CreateTimestamp()
  {
    return static_cast<uint64_t>(eCAL::Time::GetMicroSeconds());
  }

  static inline UniqueIdT CreateUniqueId()
  {
    return static_cast<UniqueIdT>(CreateTimestamp());
  }

  static inline std::string BuildPayloadMemfileName(const std::string& name, UniqueIdT memfile_id)
  {
    return name + "_" + std::to_string(memfile_id);
  }

  enum class eMemfileBroadcastMessageType : std::int32_t
  {
    INVALID,
    PAYLOAD_MEMFILE_CREATED,
    PAYLOAD_MEMFILE_REMOVED,
    PAYLOAD_MEMFILE_UPDATED,
    //PAYLOAD_MEMFILE_HEARTBEAT
  };

#pragma pack(push, 1)
  struct SMemfileBroadcastMessage
  {
    std::int32_t process_id;
    TimestampT timestamp;
    UniqueIdT payload_memfile_id;
    eMemfileBroadcastMessageType type;
  };
#pragma pack(pop)

  typedef std::vector<const SMemfileBroadcastMessage*> MemfileBroadcastMessageListT;

  class CMemoryFileBroadcast
  {
  public:
    CMemoryFileBroadcast();

    bool Create(const std::string& name, std::size_t max_queue_size);
    bool Destroy();

    std::string GetName() const;

    bool FlushLocalBroadcastQueue();
    bool FlushGlobalBroadcastQueue();

    bool Broadcast(UniqueIdT payload_memfile_id, eMemfileBroadcastMessageType type);
    bool ReceiveBroadcast(MemfileBroadcastMessageListT& message_list, TimestampT timeout, bool enable_loopback = false);

  private:
    bool IsMemfileVersionCompatible(const void * memfile_address) const;
    void ResetMemfile(void * memfile_address);

    bool m_created = false;
    std::string m_name;
    std::size_t m_max_queue_size;
    std::unique_ptr<CMemoryFile> m_broadcast_memfile;
    std::vector<char> m_broadcast_memfile_local_buffer;

    RelocatableCircularQueue<SMemfileBroadcastMessage> m_message_queue;

    TimestampT m_last_timestamp;
  };
}