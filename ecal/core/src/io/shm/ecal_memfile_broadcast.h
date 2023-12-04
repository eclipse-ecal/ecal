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

#include "io/shm/relocatable_circular_queue.h"

#include <ecal/ecal.h>

namespace eCAL 
{
  class CMemoryFile;

  static inline std::int64_t CreateTimestamp()
  {
    const auto time_point = std::chrono::steady_clock::now();
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(time_point.time_since_epoch()).count());
  }

  static inline std::uint64_t CreateEventId()
  {
    return static_cast<std::uint64_t>(CreateTimestamp());
  }

  static inline std::string BuildPayloadMemfileName(const std::string& name, std::uint64_t memfile_id)
  {
    return name + "_" + std::to_string(memfile_id);
  }

  enum class eMemfileBroadcastEventType : std::int32_t
  {
    INVALID,
    EVENT_CREATED,
    EVENT_REMOVED,
    EVENT_UPDATED,
    //EVENT_HEARTBEAT
  };

#pragma pack(push, 1)
  struct SMemfileBroadcastEvent
  {
    std::int32_t process_id;
    std::int64_t timestamp;
    std::uint64_t event_id;
    eMemfileBroadcastEventType type;
  };
#pragma pack(pop)

  typedef std::vector<const SMemfileBroadcastEvent*> MemfileBroadcastEventListT;

  class CMemoryFileBroadcast
  {
  public:
    CMemoryFileBroadcast();

    bool Create(const std::string& name, std::size_t max_queue_size);
    bool Destroy();

    std::string GetName() const;

    bool FlushLocalEventQueue();
    bool FlushGlobalEventQueue();

    bool SendEvent(std::uint64_t event_id, eMemfileBroadcastEventType type);
    bool ReceiveEvents(MemfileBroadcastEventListT& event_list, std::int64_t timeout, bool enable_loopback = false);

  private:
    bool IsMemfileVersionCompatible(const void * memfile_address) const;
    void ResetMemfile(void * memfile_address);

    bool m_created;
    std::string m_name;
    std::size_t m_max_queue_size;
    std::unique_ptr<CMemoryFile> m_broadcast_memfile;
    std::vector<char> m_broadcast_memfile_local_buffer;

    RelocatableCircularQueue<SMemfileBroadcastEvent> m_event_queue;

    std::int64_t m_last_timestamp;
  };
}