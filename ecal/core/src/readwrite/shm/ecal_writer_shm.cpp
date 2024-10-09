/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

#include <ecal/ecal_log.h>

#include "ecal_def.h"
#include "ecal_writer_shm.h"
#include "ecal/ecal_config.h"

#include <string>

namespace eCAL
{
  const std::string CDataWriterSHM::m_memfile_base_name = "ecal_";

  CDataWriterSHM::CDataWriterSHM(const std::string& host_name_, const std::string& topic_name_, const std::string& /*topic_id_*/, const Publisher::Layer::SHM::Configuration& shm_config_) :
    m_config(shm_config_)
  {
    m_host_name  = host_name_;
    m_topic_name = topic_name_;

    // initialize memory file buffer
    if (m_config.memfile_buffer_count < 1) m_config.memfile_buffer_count = 1;
    SetBufferCount(m_config.memfile_buffer_count);
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

  void CDataWriterSHM::ApplySubscription(const std::string& host_name_, const int32_t process_id_, const std::string& /*topic_id_*/, const std::string& /*conn_par_*/)
  {
    // we accept local connections only
    if (host_name_ != m_host_name) return;

    for (auto& memory_file : m_memory_file_vec)
    {
      memory_file->Connect(std::to_string(process_id_));
#ifndef NDEBUG
      Logging::Log(log_level_debug1, std::string("CDataWriterSHM::ApplySubscription - Memory FileName: ") + memory_file->GetName() + " to ProcessId " + std::to_string(process_id_));
#endif
    }
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
      Logging::Log(log_level_error, m_topic_name + "::CDataWriterSHM::SetBufferCount minimal number of memory files is 1 !");
      return false;
    }

    // prepare memfile attributes
    SSyncMemoryFileAttr memory_file_attr = {};
    memory_file_attr.min_size        = GetConfiguration().transport_layer.shm.memfile_min_size_bytes;
    memory_file_attr.reserve         = GetConfiguration().transport_layer.shm.memfile_reserve_percent;
    memory_file_attr.timeout_open_ms = PUB_MEMFILE_OPEN_TO;
    memory_file_attr.timeout_ack_ms  = m_config.acknowledge_timeout_ms;

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
        Logging::Log(log_level_error, "CDataWriterSHM::SetBufferCount - FAILED");
        return false;
      }
    }

    return true;
  }
}
