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

#include "ecal_memfile_broadcast_reader.h"
#include "ecal_memfile.h"
#include "ecal_def.h"

namespace eCAL
{
  bool CMemoryFileBroadcastReader::Bind(CMemoryFileBroadcast *memfile_broadcast)
  {
    if (m_bound) return false;
    m_memfile_broadcast = memfile_broadcast;
    m_bound = true;

    return true;
  }

  bool CMemoryFileBroadcastReader::Unbind()
  {
    if (!m_bound) return false;
    m_memfile_broadcast = nullptr;
    m_bound = false;
    return true;
  }

  bool CMemoryFileBroadcastReader::Read(MemfileBroadcastMessageListT &memfile_broadcast_message_list, std::int64_t timeout)
  {
    if (!m_bound) return false;

    bool return_result {true};

    m_memfile_broadcast->ReceiveEvents(m_broadcast_event_list, timeout, true);
    std::set<std::uint64_t> handled_event_ids;

    memfile_broadcast_message_list.clear();
    for (const auto &broadcast_event: m_broadcast_event_list)
    {
      const auto event_id = broadcast_event->event_id;
      if (!handled_event_ids.insert(event_id).second) continue;

      decltype(m_payload_memfiles)::iterator iterator;
      bool is_new_payload_memfile {false};
      std::tie(iterator, is_new_payload_memfile) = m_payload_memfiles.insert(
        {event_id, {std::make_shared<CMemoryFile>(), std::vector<char>(), 0}});

      auto &memfile_broadcast_payload = iterator->second;
      if (is_new_payload_memfile && broadcast_event->type != eMemfileBroadcastEventType::EVENT_REMOVED)
      {
        if(!memfile_broadcast_payload.payload_memfile->Create(
          BuildPayloadMemfileName(m_memfile_broadcast->GetName(), event_id).c_str(), false, 0))
        {
#ifndef NDEBUG
          std::cerr << "Error opening payload memory file" << std::endl;
#endif
          return_result = false;
          continue;
        }
        memfile_broadcast_payload.payload_memfile_buffer.reserve(
          memfile_broadcast_payload.payload_memfile->MaxDataSize());
      }

      switch (broadcast_event->type)
      {
        case eMemfileBroadcastEventType::EVENT_UPDATED:
        {
          if (memfile_broadcast_payload.payload_memfile->GetReadAccess(EXP_MEMFILE_ACCESS_TIMEOUT)) {
            memfile_broadcast_payload.payload_memfile_buffer.resize(
              memfile_broadcast_payload.payload_memfile->CurDataSize());
            memfile_broadcast_payload.payload_memfile->Read(memfile_broadcast_payload.payload_memfile_buffer.data(),
                                                            memfile_broadcast_payload.payload_memfile_buffer.size(),
                                                            0);
            memfile_broadcast_payload.payload_memfile->ReleaseReadAccess();
            memfile_broadcast_payload.timestamp = broadcast_event->timestamp;

            memfile_broadcast_message_list.push_back({memfile_broadcast_payload.payload_memfile_buffer.data(),
                                                              memfile_broadcast_payload.payload_memfile_buffer.size(),
                                                              memfile_broadcast_payload.timestamp});
          }
          else
          {
#ifndef NDEBUG
            std::cerr << "Error acquiring read access on payload file" << std::endl;
#endif
            return_result = false;
            continue;
          }
          break;
        }
        case eMemfileBroadcastEventType::EVENT_REMOVED:
          m_payload_memfiles.erase(event_id);
          break;
        case eMemfileBroadcastEventType::EVENT_CREATED:
        default:
          break;
      }

    }
    return return_result;
  }
}