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
 * @brief  shared memory (iceoryx) writer
**/

#include <sstream>

#include "ecal_def.h"
#include "ecal_config_hlp.h"
#include <ecal/ecal_log.h>
#include "ecal/ecal_process.h"
#include "readwrite/ecal_writer_iceoryx.h"

#include <iceoryx_posh/runtime/posh_runtime.hpp>

namespace eCAL
{
  CDataWriterSHM::CDataWriterSHM()
  {
  }

  CDataWriterSHM::~CDataWriterSHM()
  {
    Destroy();
  }

  void CDataWriterSHM::GetInfo(SWriterInfo info_)
  {
    info_.name                 = "iceoryx";
    info_.description          = "Iceoryx data writer";

    info_.has_mode_local       = true;
    info_.has_mode_cloud       = false;

    info_.has_qos_history_kind = false;
    info_.has_qos_reliability  = false;

    info_.send_size_max        = -1;
  }

  bool CDataWriterSHM::Create(const std::string& /*host_name_*/, const std::string& topic_name_, const std::string& /*topic_id_*/)
  {
    // create the runtime for registering with the RouDi daemon
    const iox::capro::IdString_t runtime (iox::cxx::TruncateToCapacity, std::string("/") + eCAL::Process::GetUnitName() + std::string("_") + std::to_string(eCAL::Process::GetProcessID()));
    iox::runtime::PoshRuntime::initRuntime(runtime);

    // publisher description
    const iox::capro::IdString_t service  (iox::cxx::TruncateToCapacity, eCALPAR(ICEORYX, SERVICE));
    const iox::capro::IdString_t instance (iox::cxx::TruncateToCapacity, eCALPAR(ICEORYX, INSTANCE));
    const iox::capro::IdString_t event    (iox::cxx::TruncateToCapacity, topic_name_);
    const iox::capro::ServiceDescription servicedesc(service, instance, event);

    // create publisher
    m_publisher = std::shared_ptr<iox::popo::UntypedPublisher>(new iox::popo::UntypedPublisher({servicedesc}));

    return true;
  }

  bool CDataWriterSHM::Destroy()
  {
    if(!m_publisher) return false;
    m_publisher = nullptr;
    return true;
  }

  size_t CDataWriterSHM::Send(const SWriterData& data_)
  {
    if (!m_publisher) return 0;
    size_t ret(0);

    m_publisher->loan(data_.len, alignof(data_.buf), sizeof(data_), alignof(data_))
      .and_then([&](auto& userPayload) {
        // loan successful
        // copy payload header
        std::memcpy(iox::mepoo::ChunkHeader::fromUserPayload(userPayload)->userHeader(), &data_, sizeof(SWriterData));
        // copy payload data
        std::memcpy(static_cast<char*>(userPayload), data_.buf, data_.len);
        // publish all
        m_publisher->publish(userPayload);
        ret = data_.len;
      }).or_else([&](auto& error) {
        // loan failed
        std::stringstream ss;
        ss << "CDataWriterSHM::Send(): Loan of iceoryx chunk failed ! Error code: " << static_cast<uint64_t>(error);
        Logging::Log(log_level_fatal, ss.str());
      });

    return ret;
  }
}
