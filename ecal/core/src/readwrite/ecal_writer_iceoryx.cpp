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
 * @brief  iceoryx data writer
**/

#include "ecal_def.h"
#include "ecal_config_hlp.h"
#include "ecal/ecal_process.h"
#include "readwrite/ecal_writer_iceoryx.h"

#include <iceoryx_posh/runtime/posh_runtime.hpp>

namespace eCAL
{
  CDataWriterIceoryx::CDataWriterIceoryx()
  {
  }

  CDataWriterIceoryx::~CDataWriterIceoryx()
  {
    Destroy();
  }

  void CDataWriterIceoryx::GetInfo(SWriterInfo info_)
  {
    info_.name                 = "iceoryx";
    info_.description          = "Iceoryx data writer";

    info_.has_mode_local       = true;
    info_.has_mode_cloud       = false;

    info_.has_qos_history_kind = false;
    info_.has_qos_reliability  = false;

    info_.send_size_max        = -1;
  }

  bool CDataWriterIceoryx::Create(const std::string& /*host_name_*/, const std::string& topic_name_, const std::string& /*topic_id_*/)
  {
    // create the runtime for registering with the RouDi daemon
    iox::runtime::PoshRuntime::getInstance(std::string("/") + eCAL::Process::GetUnitName() + std::string("_") + std::to_string(eCAL::Process::GetProcessID()));

    // create publisher
    m_publisher = std::shared_ptr<iox::popo::Publisher>(new iox::popo::Publisher({eCALPAR(ICEORYX, SERVICE), eCALPAR(ICEORYX, INSTANCE), topic_name_}));
    m_publisher->offer();

    return true;
  }

  bool CDataWriterIceoryx::Destroy()
  {
    if(!m_publisher) return false;

    m_publisher->stopOffer();
    m_publisher = nullptr;

    return true;
  }

  size_t CDataWriterIceoryx::Send(const SWriterData& data_)
  {
    if (!m_publisher) return 0;

    // allocate and fill chunk payload
    auto header_data_len = sizeof(SWriterData) + data_.len;
    auto ch = m_publisher->allocateChunkWithHeader(header_data_len, true);
    if(!ch)
    {
      // no more memory from iceoryx :-(
      return 0;
    }
        
    // copy payload header
    std::memcpy(ch->payload(), &data_, sizeof(SWriterData));
    // copy payload data
    std::memcpy(static_cast<char*>(ch->payload()) + sizeof(SWriterData), data_.buf, data_.len);

    // send the chunk
    m_publisher->sendChunk(ch);

    return data_.len;
  }
}
