/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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
 * @brief  memory file data writer
**/

#include <ecal/log.h>

#include "ecal_def.h"
#include "ecal_writer_shm.h"

#include <string>

namespace eCAL
{
  const std::string CDataWriterSHM::m_memfile_base_name = "ecal_";

  CDataWriterSHM::CDataWriterSHM(const eCALWriter::SHM::SAttributes& attr_) :
    m_attributes(attr_)
  {
    // initialize memory file buffer
    if (m_attributes.memfile_buffer_count < 1) m_attributes.memfile_buffer_count = 1;
    SetBufferCount(m_attributes.memfile_buffer_count);
  }

  SWriterInfo CDataWriterSHM::GetInfo()
  {
    SWriterInfo info_;

    info_.name           = "shm";
    info_.description    = "Local shared memory data writer";

    info_.has_mode_local = true;
    info_.has_mode_cloud = false;

    info_.send_size_max  = -1;

    return info_;
  }

  bool CDataWriterSHM::PrepareWrite(const SWriterAttr& attr_)
  {
    // false signals no rematching / exchanging of
    // connection parameters needed
    bool ret_state(false);

    // adapt write index if needed
    m_write_idx %= m_memory_file_vec.size();
      
    // check size and reserve new if needed
    ret_state |= m_memory_file_vec[m_write_idx]->CheckSize(attr_.len);

    return ret_state;
  }

  bool CDataWriterSHM::Write(CPayloadWriter& payload_, const SWriterAttr& attr_)
  {
    // write content
    const bool force_full_write(m_memory_file_vec.size() > 1);
    const bool sent = m_memory_file_vec[m_write_idx]->Write(payload_, attr_, force_full_write);

    // and increment file index
    m_write_idx++;
    m_write_idx %= m_memory_file_vec.size();

    return sent;
  }

  void CDataWriterSHM::ApplySubscription(const std::string& host_name_, const int32_t process_id_, const EntityIdT& topic_id_, const std::string& /*conn_par_*/)
  {
    // we accept local connections only
    if (host_name_ != m_attributes.host_name) return;

    // add or update the map with process id's and sets of topic ids
    {
      const std::lock_guard<std::mutex> lock(m_process_id_topic_id_set_map_sync);
      auto& topic_set = m_process_id_topic_id_set_map[process_id_];
      topic_set.insert(topic_id_);
    }

    for (auto& memory_file : m_memory_file_vec)
    {
      memory_file->Connect(std::to_string(process_id_));
#ifndef NDEBUG
      Logging::Log(Logging::log_level_debug1, std::string("CDataWriterSHM::ApplySubscription - Memory FileName: ") + memory_file->GetName() + " to ProcessId " + std::to_string(process_id_));
#endif
    }
  }

  void CDataWriterSHM::RemoveSubscription(const std::string& host_name_, const int32_t process_id_, const EntityIdT& topic_id_)
  {
    // we accept local disconnections only
    if (host_name_ != m_attributes.host_name) return;

    // remove topic id from the id set for the given process id
    // bool memfile_has_subscriptions(true);
    {
      const std::lock_guard<std::mutex> lock(m_process_id_topic_id_set_map_sync);
      auto process_it = m_process_id_topic_id_set_map.find(process_id_);

      // this process id is connected to the memory file
      if (process_it != m_process_id_topic_id_set_map.end())
      {
        // remove it from the id set
        process_it->second.erase(topic_id_);

        // this process id has no more connection to this memory file
        if (process_it->second.empty())
        {
          // we can remove the empty topic id set
          m_process_id_topic_id_set_map.erase(process_it);
          // and set the subscription state to false for later processing
          //memfile_has_subscriptions = false;
        }
      }
    }
// TODO: Disconnect events immediately if there is no longer an existing subscription.
//       Unfortunately the below Disconnect()-method leads to incongruous event handles on
//       unix operating systems in case of a subscriber tries to reconnected to same publisher
//       again.
//
//    // memory file is still connected to at least one topic id of this process id
//    // no need to Disconnect process id
//    if (memfile_has_subscriptions) return;
//
//    for (auto& memory_file : m_memory_file_vec)
//    {
//      memory_file->Disconnect(std::to_string(process_id_));
//#ifndef NDEBUG
//      Logging::Log(Logging::log_level_debug1, std::string("CDataWriterSHM::RemoveSubscription - Memory FileName: ") + memory_file->GetName() + " to ProcessId " + std::to_string(process_id_));
//#endif
//    }
  }

  Registration::ConnectionPar CDataWriterSHM::GetConnectionParameter()
  {
    Registration::ConnectionPar connection_par;
    for (auto& memory_file : m_memory_file_vec)
    {
      connection_par.layer_par_shm.memory_file_list.push_back(memory_file->GetName());
    }
    return connection_par;
  }

  bool CDataWriterSHM::SetBufferCount(size_t buffer_count_)
  {
    // no need to adapt anything
    if (m_memory_file_vec.size() == buffer_count_) return true;

    // buffer count zero not allowed
    if (buffer_count_ < 1)
    {
      Logging::Log(Logging::log_level_error, m_attributes.topic_name + "::CDataWriterSHM::SetBufferCount minimal number of memory files is 1 !");
      return false;
    }

    // prepare memfile attributes
    SSyncMemoryFileAttr memory_file_attr = {};
    memory_file_attr.min_size        = m_attributes.memfile_min_size_bytes;
    memory_file_attr.reserve         = m_attributes.memfile_reserve_percent;
    memory_file_attr.timeout_open_ms = PUB_MEMFILE_OPEN_TO;
    memory_file_attr.timeout_ack_ms  = m_attributes.acknowledge_timeout_ms;

    // retrieve the memory file size of existing files
    size_t memory_file_size(0);
    if (!m_memory_file_vec.empty())
    {
      memory_file_size = m_memory_file_vec[0]->GetSize();
    }
    else
    {
      memory_file_size = memory_file_attr.min_size;
    }

    // create memory file vector
    m_memory_file_vec.clear();
    while (m_memory_file_vec.size() < buffer_count_)
    {
      auto sync_memfile = std::make_shared<CSyncMemoryFile>(m_memfile_base_name, memory_file_size, memory_file_attr);
      if (sync_memfile->IsCreated())
      {
        m_memory_file_vec.push_back(sync_memfile);
      }
      else
      {
        m_memory_file_vec.clear();
        Logging::Log(Logging::log_level_error, "CDataWriterSHM::SetBufferCount - FAILED");
        return false;
      }
    }

    return true;
  }
}
