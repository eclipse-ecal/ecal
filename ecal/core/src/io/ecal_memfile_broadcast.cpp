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

#include "ecal_def.h"

#include "ecal_memfile_broadcast.h"
#include "ecal_memfile.h"
#include "ecal_global_accessors.h"

#include <iostream>
#include <array>

namespace eCAL
{
#pragma pack(push, 1)
  struct SMemfileBroadcastHeader
  {
    std::uint32_t version = 1;
    std::uint64_t message_queue_offset = sizeof(SMemfileBroadcastHeader);
    std::int64_t timestamp = CreateTimestamp();
    std::array<uint8_t, 4> _reserved_0 = {};
  };
#pragma pack(pop)

  static SMemfileBroadcastHeader *GetMemfileHeader(void *address)
  {
    return reinterpret_cast<SMemfileBroadcastHeader *>(address);
  }

  static const SMemfileBroadcastHeader *GetMemfileHeader(const void *address)
  {
    return reinterpret_cast<const SMemfileBroadcastHeader *>(address);
  }

  static void *GetEventQueueAddress(void *address)
  {
    return reinterpret_cast<void *>(static_cast<char *>(address) + GetMemfileHeader(address)->message_queue_offset);
  }

  CMemoryFileBroadcast::CMemoryFileBroadcast(): m_created(false), m_max_queue_size(0), m_broadcast_memfile(std::make_unique<eCAL::CMemoryFile>()), m_event_queue(), m_last_timestamp(0)
  {
  }

  bool CMemoryFileBroadcast::Create(const std::string &name, std::size_t max_queue_size)
  {
    if (m_created) return false;

    m_max_queue_size = max_queue_size;
    m_name = name;
    const auto presumably_memfile_size =
      RelocatableCircularQueue<SMemfileBroadcastEvent>::PresumablyOccupiedMemorySize(m_max_queue_size) +
      sizeof(SMemfileBroadcastHeader);
    if (!m_broadcast_memfile->Create(name.c_str(), true, presumably_memfile_size, true))
    {
#ifndef NDEBUG
      std::cerr << "Unable to access broadcast memory file." << std::endl;
#endif
      return false;
    }

    if (m_broadcast_memfile->MaxDataSize() < presumably_memfile_size)
    {
#ifndef NDEBUG
      std::cerr << "Invalid broadcast memory file size." << std::endl;
#endif
      return false;
    }

    m_broadcast_memfile_local_buffer.resize(presumably_memfile_size);

    if (m_broadcast_memfile->GetWriteAccess(EXP_MEMFILE_ACCESS_TIMEOUT))
    {
      // Check if memfile is initialized
      bool is_memfile_initialized{ m_broadcast_memfile->CurDataSize() != 0 };

      void* memfile_address = nullptr;
      m_broadcast_memfile->GetWriteAddress(memfile_address, m_broadcast_memfile->MaxDataSize());

      if (!is_memfile_initialized)
        ResetMemfile(memfile_address);
      else
      {
        if (!IsMemfileVersionCompatible(memfile_address))
        {
#ifndef NDEBUG
          std::cerr << "Broadcast memory file is not compatible" << std::endl;
#endif
          m_broadcast_memfile->ReleaseWriteAccess();
          return false;
        }
      }

      m_broadcast_memfile->ReleaseWriteAccess();
    }
    else
    {
#ifndef NDEBUG
      std::cerr << "Unable to acquire write access on broadcast memory file" << std::endl;
#endif
      return false;
    }

    m_created = true;
    return true;
  }

  bool CMemoryFileBroadcast::Destroy()
  {
    if (!m_created) return false;
    m_broadcast_memfile->Destroy(false);
    m_created = false;
    return true;
  }

  std::string CMemoryFileBroadcast::GetName() const
  {
    return m_name;
  }

  bool CMemoryFileBroadcast::IsMemfileVersionCompatible(const void *memfile_address) const
  {
    const auto *header = GetMemfileHeader(memfile_address);
    return (header->version == SMemfileBroadcastHeader().version);
  }

  void CMemoryFileBroadcast::ResetMemfile(void *memfile_address)
  {
    auto *header = GetMemfileHeader(memfile_address);
    *header = SMemfileBroadcastHeader();
    m_event_queue.SetBaseAddress(GetEventQueueAddress(memfile_address));
    m_event_queue.Reset(m_max_queue_size);
#ifndef NDEBUG
    std::cout << "Broadcast memory file has been resetted" << std::endl;
#endif
  }

  bool CMemoryFileBroadcast::FlushLocalEventQueue()
  {
    if (!m_created) return false;

    if (m_broadcast_memfile->GetReadAccess(EXP_MEMFILE_ACCESS_TIMEOUT))
    {
      // Check if memfile is initialized
      if (m_broadcast_memfile->CurDataSize())
      {
        const void* memfile_address = nullptr;
        m_broadcast_memfile->GetReadAddress(memfile_address, m_broadcast_memfile->MaxDataSize());
        m_last_timestamp = GetMemfileHeader(memfile_address)->timestamp;
        m_broadcast_memfile->ReleaseReadAccess();
        return true;
      }
      else
      {
#ifndef NDEBUG
        std::cerr << "Broadcast memory file is not initialized" << std::endl;
#endif
        m_broadcast_memfile->ReleaseReadAccess();
        return false;
      }

    }
    else
    {
#ifndef NDEBUG
      std::cerr << "Unable to acquire read access on broadcast memory file" << std::endl;
#endif
      return false;
    }
  }

  bool CMemoryFileBroadcast::FlushGlobalEventQueue()
  {
    if (!m_created) return false;

    if (m_broadcast_memfile->GetWriteAccess(EXP_MEMFILE_ACCESS_TIMEOUT))
    {
      // Don't need to check consistency since memfile will be resetted anyways
      void *memfile_address = nullptr;
      m_broadcast_memfile->GetWriteAddress(memfile_address, m_broadcast_memfile->MaxDataSize());
      ResetMemfile(memfile_address);
      m_broadcast_memfile->ReleaseWriteAccess();
    }
    else
    {
#ifndef NDEBUG
      std::cerr << "Unable to acquire read access on broadcast memory file" << std::endl;
#endif
      return false;
    }

    return true;
  }

  bool CMemoryFileBroadcast::SendEvent(std::uint64_t event_id, eMemfileBroadcastEventType type)
  {
    if (!m_created) return false;

    if (m_broadcast_memfile->GetWriteAccess(EXP_MEMFILE_ACCESS_TIMEOUT))
    {
      // Check if memfile is initialized
      bool is_memfile_initialized{ m_broadcast_memfile->CurDataSize() != 0 };

      void* memfile_address = nullptr;
      m_broadcast_memfile->GetWriteAddress(memfile_address, m_broadcast_memfile->MaxDataSize());

      if (!is_memfile_initialized)
        ResetMemfile(memfile_address);

      m_event_queue.SetBaseAddress(GetEventQueueAddress(memfile_address));
      const auto timestamp = CreateTimestamp();

      m_event_queue.Push({g_process_id, timestamp, event_id, type});
      GetMemfileHeader(memfile_address)->timestamp = timestamp;

      m_broadcast_memfile->ReleaseWriteAccess();
      return true;
    }
    else
    {
#ifndef NDEBUG
      std::cerr << "Unable to acquire write access on broadcast memory file" << std::endl;
#endif
      return false;
    }
  }


  bool CMemoryFileBroadcast::ReceiveEvents(MemfileBroadcastEventListT &event_list, std::int64_t timeout, bool enable_loopback)
  {
    if (m_broadcast_memfile->GetReadAccess(EXP_MEMFILE_ACCESS_TIMEOUT))
    {
      // Check if memfile is initialized
      if (m_broadcast_memfile->CurDataSize())
      {
        m_broadcast_memfile->Read(m_broadcast_memfile_local_buffer.data(), m_broadcast_memfile_local_buffer.size(), 0);
        m_broadcast_memfile->ReleaseReadAccess();
      }
      else
      {
#ifndef NDEBUG
        std::cerr << "Broadcast memory file is not initialized" << std::endl;
#endif
        m_broadcast_memfile->ReleaseReadAccess();
        return false;
      }
      
    }
    else
    {
#ifndef NDEBUG
      std::cerr << "Unable to acquire read access on broadcast memory file" << std::endl;
#endif
      return false;
    }

    event_list.clear();

    const auto *header = GetMemfileHeader(m_broadcast_memfile_local_buffer.data());
    m_event_queue.SetBaseAddress(GetEventQueueAddress(m_broadcast_memfile_local_buffer.data()));

    const auto timeout_threshold = CreateTimestamp() - (timeout * 1000);
    for (const auto &broadcast_message: m_event_queue)
    {
      const auto timestamp = broadcast_message.timestamp;
      if ((timeout && (timestamp <= timeout_threshold)) || (timestamp <= m_last_timestamp))
        break;
      if ((broadcast_message.process_id == g_process_id) && !enable_loopback)
        continue;
      event_list.push_back(&broadcast_message);
    }

    m_last_timestamp = header->timestamp;

    return true;
  }
}
