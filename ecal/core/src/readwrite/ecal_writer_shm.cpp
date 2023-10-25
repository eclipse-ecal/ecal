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
 * @brief  memory file data writer
**/

#include <ecal/ecal.h>
#include <ecal/ecal_config.h>
#include <ecal/ecal_log.h>

#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#include <ecal/core/pb/layer.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "ecal_def.h"
#include "ecal_writer.h"
#include "ecal_writer_shm.h"

namespace eCAL
{
  const std::string CDataWriterSHM::m_memfile_base_name = "ecal_";

  CDataWriterSHM::~CDataWriterSHM()
  {
    Destroy();
  }

  SWriterInfo CDataWriterSHM::GetInfo()
  {
    SWriterInfo info_;

    info_.name                 = "shm";
    info_.description          = "Local shared memory data writer";

    info_.has_mode_local       = true;
    info_.has_mode_cloud       = false;

    info_.has_qos_history_kind = false;
    info_.has_qos_reliability  = true;

    info_.send_size_max        = -1;

    return info_;
  }
  
  bool CDataWriterSHM::Create(const std::string& /*host_name_*/, const std::string& topic_name_, const std::string & /*topic_id_*/)
  {
    if (m_created) return false;
    m_topic_name = topic_name_;

    // init write index and create memory files
    m_write_idx = 0;

    // set attributes
    m_memory_file_attr.min_size        = Config::GetMemfileMinsizeBytes();
    m_memory_file_attr.reserve         = Config::GetMemfileOverprovisioningPercentage();
    m_memory_file_attr.timeout_open_ms = PUB_MEMFILE_OPEN_TO;
    m_memory_file_attr.timeout_ack_ms  = Config::GetMemfileAckTimeoutMs();

    // initialize memory file buffer
    m_created = SetBufferCount(m_buffer_count);;
    return m_created;
  }

  bool CDataWriterSHM::Destroy()
  {
    if (!m_created) return false;
    m_created = false;

    m_memory_file_vec.clear();

    return true;
  }

  bool CDataWriterSHM::SetQOS(const QOS::SWriterQOS& qos_)
  {
    m_qos = qos_;
    return true;
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

    // retrieve the memory file size of existing files
    size_t memory_file_size(0);
    if (!m_memory_file_vec.empty())
    {
      memory_file_size = m_memory_file_vec[0]->GetSize();
    }
    else
    {
      memory_file_size = m_memory_file_attr.min_size;
    }

    // create memory file vector
    m_memory_file_vec.clear();
    while (m_memory_file_vec.size() < buffer_count_)
    {
      auto sync_memfile = std::make_shared<CSyncMemoryFile>(m_memfile_base_name, memory_file_size, m_memory_file_attr);
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

  bool CDataWriterSHM::PrepareWrite(const SWriterAttr& attr_)
  {
    if (!m_created) return false;

    // false signals no rematching / exchanging of
    // connection parameters needed
    bool ret_state(false);

    // adapt number of used memory files if needed
    if (attr_.buffering != m_buffer_count)
    {
      SetBufferCount(attr_.buffering);

      // store new buffer count and flag change
      m_buffer_count = attr_.buffering;
      ret_state |= true;
    }

    // adapt write index if needed
    m_write_idx %= m_memory_file_vec.size();
      
    // check size and reserve new if needed
    ret_state |= m_memory_file_vec[m_write_idx]->CheckSize(attr_.len);

    return ret_state;
  }

  bool CDataWriterSHM::Write(CPayloadWriter& payload_, const SWriterAttr& attr_)
  {
    if (!m_created) return false;

    // write content
    const bool force_full_write(m_memory_file_vec.size() > 1);
    const bool sent = m_memory_file_vec[m_write_idx]->Write(payload_, attr_, force_full_write);

    // and increment file index
    m_write_idx++;
    m_write_idx %= m_memory_file_vec.size();

    return sent;
  }

  void CDataWriterSHM::AddLocConnection(const std::string& process_id_, const std::string& /*topic_id_*/, const std::string& /*conn_par_*/)
  {
    if (!m_created) return;

    for (auto& memory_file : m_memory_file_vec)
    {
      memory_file->Connect(process_id_);
#ifndef NDEBUG
      Logging::Log(log_level_debug1, std::string("CDataWriterSHM::AddLocConnection - Memory FileName: ") + memory_file->GetName() + " to ProcessId " + process_id_);
#endif
    }
  }

  std::string CDataWriterSHM::GetConnectionParameter()
  {
    // starting from eCAL version > 5.8.13/5.9.0 the ConnectionParameter is defined as google protobuf
    eCAL::pb::ConnnectionPar connection_par;
    for (auto& memory_file : m_memory_file_vec)
    {
      connection_par.mutable_layer_par_shm()->add_memory_file_list(memory_file->GetName());
    }
    return connection_par.SerializeAsString();
  }
}
