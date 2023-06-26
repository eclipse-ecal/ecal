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
 * @brief  inproc data writer
**/

#include <ecal/ecal.h>
#include <ecal/ecal_log.h>

#include "ecal_global_accessors.h"
#include "pubsub/ecal_subgate.h"
#include "readwrite/ecal_writer_inproc.h"

namespace eCAL
{
  CDataWriterInProc::~CDataWriterInProc()
  {
    Destroy();
  }

  SWriterInfo CDataWriterInProc::GetInfo()
  {
    SWriterInfo info_;

    info_.name                 = "inproc";
    info_.description          = "InProc data writer";

    info_.has_mode_local       = true;
    info_.has_mode_cloud       = false;

    info_.has_qos_history_kind = false;
    info_.has_qos_reliability  = true;

    info_.send_size_max        = -1;

    return info_;
  }
  
  bool CDataWriterInProc::Create(const std::string & host_name_, const std::string & topic_name_, const std::string & topic_id_)
  {
    if (m_created) return false;

    m_host_name  = host_name_;
    m_topic_name = topic_name_;
    m_topic_id   = topic_id_;

    m_created = true;
    return true;
  }

  bool CDataWriterInProc::Destroy()
  {
    if (!m_created) return false;
    m_created = false;

    return true;
  }

  /////////////////////////////////////////////////////////////////
  // apply the data straight to the subscriber gate
  /////////////////////////////////////////////////////////////////
  bool CDataWriterInProc::Write(const void* const buf_, const SWriterAttr& attr_)
  {
    if (!m_created)             return(false);
    if (g_subgate() == nullptr) return(false);

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug4, m_topic_name + "::CDataWriterInProc::Send");
#endif

    // send it
    return g_subgate()->ApplySample(m_topic_name, m_topic_id, static_cast<const char*>(buf_), attr_.len, attr_.id, attr_.clock, attr_.time, attr_.hash, eCAL::pb::tl_inproc);
  }
}
