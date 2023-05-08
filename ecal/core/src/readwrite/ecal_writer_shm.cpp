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
#include "ecal_config_reader_hlp.h"
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

    // create the files
    for (size_t num(0); num < m_buffer_count; ++num)
    {
      auto sync_memfile = std::make_shared<CSyncMemoryFile>(m_memfile_base_name, 0, m_memory_file_attr);
      m_memory_file_vec.push_back(sync_memfile);
    }

    m_created = true;
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

  bool CDataWriterSHM::PrepareWrite(const SWriterAttr& attr_)
  {
    if (!m_created)     return false;
    if (attr_.len == 0) return false;

    // false signals no rematching / exchanging of
    // connection parameters needed
    bool ret_state(false);

    // check number of requested memory file buffer
    if (attr_.buffering != m_buffer_count)
    {
      // store new size and flag change
      m_buffer_count = attr_.buffering;
      ret_state |= true;

      // ----------------------------------------------------------------------
      // REMOVE ME IN ECAL6
      // ----------------------------------------------------------------------
      // recreate memory buffer list to stay compatible to older versions
      // for the case that we have ONE existing buffer
      // and that single buffer is communicated with an older shm datareader
      // in this case we need to invalidate (destroy) the existing buffer
      // and the old datareader will get blind (fail safe)
      // otherwise it would still receive every n-th write
      // this state change will lead to some lost samples
      if ((m_memory_file_vec.size() == 1) && (m_memory_file_vec.size() < m_buffer_count))
      {
        m_memory_file_vec.clear();
      }
      // ----------------------------------------------------------------------
      // REMOVE ME IN ECAL6
      // ----------------------------------------------------------------------

      // increase buffer count
      while (m_memory_file_vec.size() < m_buffer_count)
      {
        auto sync_memfile = std::make_shared<CSyncMemoryFile>(m_memfile_base_name, attr_.len, m_memory_file_attr);
        m_memory_file_vec.push_back(sync_memfile);
      }
      // decrease buffer count
      while (m_memory_file_vec.size() > m_buffer_count)
      {
        m_memory_file_vec.pop_back();
      }
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
    const bool sent = m_memory_file_vec[m_write_idx]->Write(payload_, attr_);

    // and increment file index
    m_write_idx++;
    m_write_idx %= m_memory_file_vec.size();

    return sent;
  }

  bool CDataWriterSHM::AddLocConnection(const std::string& process_id_, const std::string& /*conn_par_*/)
  {
    if (!m_created) return false;

    for (auto& memory_file : m_memory_file_vec)
    {
      if (!memory_file->Connect(process_id_))
      {
        return false;
      }
    }

    return true;
  }

  bool CDataWriterSHM::RemLocConnection(const std::string& process_id_)
  {
    if (!m_created) return false;

    for (auto& memory_file : m_memory_file_vec)
    {
      if (!memory_file->Disconnect(process_id_))
      {
        return false;
      }
    }

    return true;
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
