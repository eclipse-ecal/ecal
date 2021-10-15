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
 * @brief  tcp writer
**/

#include <sstream>
#include <regex>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include "ecal/pb/layer.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "ecal_def.h"
#include "ecal_config_hlp.h"
#include <ecal/ecal_log.h>
#include "ecal/ecal_process.h"
#include "readwrite/ecal_writer_tcp.h"


namespace eCAL
{
  CDataWriterTCP::CDataWriterTCP()
  {
  }

  CDataWriterTCP::~CDataWriterTCP()
  {
    Destroy();
  }

  void CDataWriterTCP::GetInfo(SWriterInfo info_)
  {
    info_.name                 = "tcp";
    info_.description          = "tcp data writer";

    info_.has_mode_local       = true;
    info_.has_mode_cloud       = true;

    info_.has_qos_history_kind = false;
    info_.has_qos_reliability  = false;

    info_.send_size_max        = -1;
  }

  bool CDataWriterTCP::Create(const std::string& /*host_name_*/, const std::string& topic_name_, const std::string& /*topic_id_*/)
  {
    // create publisher
    //m_publisher = ??;

    return true;
  }

  bool CDataWriterTCP::Destroy()
  {
    //if(!m_publisher) return false;

    // destroy publisher
    //m_publisher = nullptr;

    return true;
  }

  size_t CDataWriterTCP::Write(const SWriterData& data_)
  {
    //if (!m_publisher) return 0;
    size_t ret(0);

    return ret;
  }

  std::string CDataWriterTCP::GetConnectionParameter()
  {
    eCAL::pb::ConnnectionPar connection_par;

    // set tcp port
    connection_par.mutable_layer_par_tcp()->set_port(1234);

    return connection_par.SerializeAsString();
  }
}
