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
#include "ecal_memfile.h"
#include "ecal_def.h"

namespace eCAL
{
  bool CMemoryFileBroadcastWriter::Bind(CMemoryFileBroadcast *memfile_broadcast)
  {
    if (m_bound) return false;
    m_memfile_broadcast = memfile_broadcast;
    m_event_id = CreateEventId();
    m_payload_memfile = std::make_unique<CMemoryFile>();
    if (!m_payload_memfile->Create(BuildPayloadMemfileName(m_memfile_broadcast->GetName(), m_event_id).c_str(), true, 1024))
    {
#ifndef NDEBUG
      std::cerr << "Unable to create payload memfile" << std::endl;
#endif
      return false;
    }

    m_memfile_broadcast->SendEvent(m_event_id, eMemfileBroadcastEventType::EVENT_CREATED);

    m_bound = true;
    return true;
  }

  bool CMemoryFileBroadcastWriter::Write(const void *data, std::size_t size)
  {
    if (!m_bound) return false;

    if ((m_payload_memfile->MaxDataSize() < size) || m_reset)
    {
      auto payload_memfile = std::make_unique<CMemoryFile>();
      const auto event_id = CreateEventId();
      if (!payload_memfile->Create(BuildPayloadMemfileName(m_memfile_broadcast->GetName(), event_id).c_str(), true, size * 2))
      {
#ifndef NDEBUG
        std::cerr << "Unable to create new payload memory file" << std::endl;
#endif
        return false;
      }
      m_memfile_broadcast->SendEvent(m_event_id, eMemfileBroadcastEventType::EVENT_REMOVED);
      m_payload_memfile->Destroy(true);
      m_payload_memfile = std::move(payload_memfile);
      m_event_id = event_id;
      m_memfile_broadcast->SendEvent(m_event_id, eMemfileBroadcastEventType::EVENT_CREATED);
      m_reset = false;
    }

    if (m_payload_memfile->GetWriteAccess(EXP_MEMFILE_ACCESS_TIMEOUT))
    {
      m_payload_memfile->Write(data, size, 0);
      m_payload_memfile->ReleaseWriteAccess();
      m_memfile_broadcast->SendEvent(m_event_id, eMemfileBroadcastEventType::EVENT_UPDATED);

      return true;
    }
    else
    {
#ifndef NDEBUG
      std::cerr << "Error acquiring write access on payload file" << std::endl;
#endif
      m_reset = true;
    }

    return false;
  }

  void CMemoryFileBroadcastWriter::Unbind()
  {
    if (!m_bound) return;

    m_memfile_broadcast->SendEvent(m_event_id, eMemfileBroadcastEventType::EVENT_REMOVED);
    m_memfile_broadcast = nullptr;
    m_payload_memfile->Destroy(true);
    m_payload_memfile.reset();
    m_bound = false;
    m_reset = false;
  }
}