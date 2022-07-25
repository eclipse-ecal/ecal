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

#include "ecal_memfile_broadcast_writer.h"
#include "ecal_memfile_broadcast.h"

namespace eCAL
{
  bool CMemoryFileBroadcastWriter::Bind(CMemoryFileBroadcast *memfile_broadcast)
  {
    if (m_created) return false;
    m_memfile_broadcast = memfile_broadcast;
    m_payload_memfile_id = CreateUniqueId();
    m_payload_memfile = std::make_unique<CMemoryFile>();
    if (!m_payload_memfile->Create(
      BuildPayloadMemfileName(m_memfile_broadcast->GetName(), m_payload_memfile_id).c_str(), true, 1024)) {
      std::cerr << "Unable to create payload memfile" << std::endl;
      return false;
    }

    m_memfile_broadcast->Broadcast(m_payload_memfile_id, eMemfileBroadcastMessageType::PAYLOAD_MEMFILE_CREATED);

    m_created = true;
    return true;
  }

  bool CMemoryFileBroadcastWriter::Write(const void *data, std::size_t size)
  {
    if (!m_created) return false;

    if (m_payload_memfile->MaxDataSize() < size) {
      auto payload_memfile = std::make_unique<CMemoryFile>();
      const auto payload_memfile_id = CreateUniqueId();
      if (!payload_memfile->Create(
        BuildPayloadMemfileName(m_memfile_broadcast->GetName(), payload_memfile_id).c_str(), true, size * 2)) {
        std::cerr << "Unable to create new payload memory file" << std::endl;
        return false;
      }
      m_memfile_broadcast->Broadcast(m_payload_memfile_id, eMemfileBroadcastMessageType::PAYLOAD_MEMFILE_REMOVED);
      m_payload_memfile = std::move(payload_memfile);
      m_payload_memfile_id = payload_memfile_id;
      m_memfile_broadcast->Broadcast(m_payload_memfile_id, eMemfileBroadcastMessageType::PAYLOAD_MEMFILE_CREATED);
    }

    if (m_payload_memfile->GetWriteAccess(100)) {
      m_payload_memfile->Write(data, size, 0);
      m_payload_memfile->ReleaseWriteAccess();
      m_memfile_broadcast->Broadcast(m_payload_memfile_id, eMemfileBroadcastMessageType::PAYLOAD_MEMFILE_UPDATED);

      return true;
    }
    else
      std::cerr << "Error acquiring write access on payload file" << std::endl;

    return false;
  }

  void CMemoryFileBroadcastWriter::Unbind()
  {
    if (!m_created) return;

    m_memfile_broadcast->Broadcast(m_payload_memfile_id, eMemfileBroadcastMessageType::PAYLOAD_MEMFILE_REMOVED);
    m_memfile_broadcast = nullptr;
    m_payload_memfile.reset();
    m_created = false;
  }
}