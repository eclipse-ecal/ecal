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

  bool CMemoryFileBroadcastReader::Read(MemfileBroadcastPayloadMessageListT &memfile_broadcast_payload_message_list, TimestampT timeout)
  {
    if (!m_bound) return false;

    if (timeout != 0) {
      /*const auto timeout_threshold = CreateTimestamp() - (timeout * 1000);
      std::remove_if(m_payload_memfiles.begin(), m_payload_memfiles.end(),[timeout_threshold](const auto& pair){
        return pair.second.timestamp < timeout_threshold;
      });*/
    }

    m_memfile_broadcast->ReceiveBroadcast(m_broadcast_message_list, timeout, false);
    std::set<UniqueIdT> handled_payload_memfile_ids;

    memfile_broadcast_payload_message_list.clear();
    for (const auto &broadcast_message: m_broadcast_message_list) {
      const auto payload_memfile_id = broadcast_message->payload_memfile_id;
      if (!handled_payload_memfile_ids.insert(payload_memfile_id).second) continue;

      decltype(m_payload_memfiles)::iterator iterator;
      bool is_new_payload_memfile;
      std::tie(iterator, is_new_payload_memfile) = m_payload_memfiles.insert(
        {payload_memfile_id, {std::make_shared<CMemoryFile>(), std::vector<char>(), 0}});

      auto &memfile_broadcast_payload = iterator->second;
      if (is_new_payload_memfile &&
          broadcast_message->type != eMemfileBroadcastMessageType::PAYLOAD_MEMFILE_REMOVED) {
        memfile_broadcast_payload.payload_memfile->Create(
          BuildPayloadMemfileName(m_memfile_broadcast->GetName(), payload_memfile_id).c_str(), false, 0);
        memfile_broadcast_payload.payload_memfile_buffer.reserve(
          memfile_broadcast_payload.payload_memfile->MaxDataSize());
      }

      switch (broadcast_message->type) {
        case eMemfileBroadcastMessageType::PAYLOAD_MEMFILE_UPDATED: {
          if (memfile_broadcast_payload.payload_memfile->GetReadAccess(100)) {
            memfile_broadcast_payload.payload_memfile_buffer.resize(
              memfile_broadcast_payload.payload_memfile->CurDataSize());
            memfile_broadcast_payload.payload_memfile->Read(memfile_broadcast_payload.payload_memfile_buffer.data(),
                                                            memfile_broadcast_payload.payload_memfile_buffer.size(),
                                                            0);
            memfile_broadcast_payload.payload_memfile->ReleaseReadAccess();
            memfile_broadcast_payload.timestamp = broadcast_message->timestamp;

            memfile_broadcast_payload_message_list.push_back({memfile_broadcast_payload.payload_memfile_buffer.data(),
                                                              memfile_broadcast_payload.payload_memfile_buffer.size(),
                                                              memfile_broadcast_payload.timestamp});
          }
          else
          {
            std::cerr << "Error acquiring read access on payload file" << std::endl;
            return false;
          }
          break;
        }
        case eMemfileBroadcastMessageType::PAYLOAD_MEMFILE_REMOVED:
          m_payload_memfiles.erase(payload_memfile_id);
          break;
        case eMemfileBroadcastMessageType::PAYLOAD_MEMFILE_CREATED:
          break;
        default:
          break;
      }

    }
    return true;
  }
}