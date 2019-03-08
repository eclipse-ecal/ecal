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
 * @brief  lcm data writer
**/

#include <ecal/ecal.h>

#include "ecal_def.h"
#include "ecal_config_hlp.h"
#include "ecal_writer_lcm.h"

#include "ecal_def.h"
#include "ecal_global_accessors.h"

#include "lcm/lcmlite.h"

static void transmit_packet(const void* buf_, int len_, void* user)
{
  static_cast<eCAL::CUDPSender*>(user)->Send(buf_, static_cast<size_t>(len_));
}

namespace eCAL
{
  CDataWriterLCM::CDataWriterLCM() : m_initialized(false)
  {
  }

  CDataWriterLCM::~CDataWriterLCM()
  {
    Destroy();
  }

  void CDataWriterLCM::GetInfo(SWriterInfo info_)
  {
    info_.name                 = "lcm";
    info_.description          = "Google LCM data writer";

    info_.has_mode_local       = true;
    info_.has_mode_cloud       = true;

    info_.has_qos_history_kind = false;
    info_.has_qos_reliability  = false;

    info_.send_size_max        = LCM3_MAX_PACKET_SIZE;
  }

  bool CDataWriterLCM::Create(const std::string& /*host_name_*/, const std::string& topic_name_, const std::string& /*topic_id_*/)
  {
    if (m_created) return false;

    m_topic_name = topic_name_;

    m_created = true;
    return true;
  }

  bool CDataWriterLCM::Destroy()
  {
    if (!m_created) return false;

    if (m_initialized)
    {
      m_lcm = nullptr;
      m_udp_sender.Destroy();
      m_initialized = false;
    }

    m_created = false;
    return true;
  }

  size_t CDataWriterLCM::Send(const SWriterData& data_)
  {
    if (!m_created) return 0;

    // let's initialize it on first send, to not waste memory per default
    if (!m_initialized)
    {
      int ttl(0);
      if (eCALPAR(NET, ENABLED)) ttl = eCALPAR(NET, UDP_MULTICAST_TTL);

      SSenderAttr attr;
      attr.ipaddr     = NET_UDP_MULTICAST_GROUP_LCM;
      attr.port       = NET_UDP_MULTICAST_PORT_LCM;
      attr.loopback   = true;
      attr.ttl        = ttl;
      attr.sndbuf     = eCALPAR(NET, UDP_MULTICAST_SNDBUF);
      attr.local_only = false;
      m_udp_sender.Create(attr);

      m_lcm = std::make_shared<lcmlite_pub_t>();
      lcmlite_pub_init(m_lcm.get(), transmit_packet, &m_udp_sender);

      m_initialized = true;
    }

    int res = lcmlite_publish(m_lcm.get(), m_topic_name.c_str(), data_.buf, static_cast<int>(data_.len));
    if (res == 0) return(data_.len);
    else          return(0);
  }
}
