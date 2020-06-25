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
 * @brief  udp data writer
**/

#include <ecal/ecal.h>
#include <ecal/ecal_log.h>

#include "ecal_def.h"
#include "ecal_config_hlp.h"
#include "ecal_writer_udp_mc.h"

#include "topic2mcast.h"
#include "io/snd_sample.h"

namespace eCAL
{
  CDataWriterUdpMC::~CDataWriterUdpMC()
  {
    Destroy();
  }

  void CDataWriterUdpMC::GetInfo(SWriterInfo info_)
  {
    info_.name                 = "udp";
    info_.description          = "UDP multicast data writer";

    info_.has_mode_local       = true;
    info_.has_mode_cloud       = true;

    info_.has_qos_history_kind = false;
    info_.has_qos_reliability  = false;

    info_.send_size_max        = -1;
  }

  bool CDataWriterUdpMC::Create(const std::string & host_name_, const std::string & topic_name_, const std::string & topic_id_)
  {
    if (m_created) return false;

    m_host_name   = host_name_;
    m_topic_name  = topic_name_;
    m_topic_id    = topic_id_;

    m_udp_ipaddr  = topic2mcast(topic_name_, eCALPAR(NET, UDP_MULTICAST_GROUP), eCALPAR(NET, UDP_MULTICAST_MASK));

    SSenderAttr attr;
    attr.ipaddr     = m_udp_ipaddr;
    attr.port       = eCALPAR(NET, UDP_MULTICAST_PORT) + NET_UDP_MULTICAST_PORT_SAMPLE_OFF;
    attr.unicast    = false;
    attr.sndbuf     = eCALPAR(NET, UDP_MULTICAST_SNDBUF);

    // create sample sender without activated loopback
    attr.loopback   = false;
    attr.ttl        = eCALPAR(NET, UDP_MULTICAST_TTL);
    m_sample_snd_no_loopback.Create(attr);

    // create sample sender with activated loopback
    int ttl(0);
    if (eCALPAR(NET, ENABLED)) ttl = eCALPAR(NET, UDP_MULTICAST_TTL);
    attr.loopback   = true;
    attr.ttl        = ttl;
    m_sample_snd_loopback.Create(attr);

    m_created = true;
    return true;
  }

  bool CDataWriterUdpMC::Destroy()
  {
    if (!m_created) return false;

    m_created = false;
    return true;
  }

  size_t CDataWriterUdpMC::Send(const SWriterData& data_)
  {
    if (!m_created) return 0;

    // create new sample
    m_ecal_sample.Clear();
    m_ecal_sample.set_cmd_type(eCAL::pb::bct_set_sample);
    auto ecal_sample_mutable_topic = m_ecal_sample.mutable_topic();
    ecal_sample_mutable_topic->set_hname(m_host_name);
    ecal_sample_mutable_topic->set_tname(m_topic_name);
    ecal_sample_mutable_topic->set_tid(m_topic_id);

    // set layer
    auto layer = ecal_sample_mutable_topic->add_tlayer();
    layer->set_type(eCAL::pb::eTLayerType::tl_ecal_udp_mc);
    layer->set_confirmed(true);

    // append content
    auto ecal_sample_mutable_content = m_ecal_sample.mutable_content();
    ecal_sample_mutable_content->set_id(data_.id);
    ecal_sample_mutable_content->set_clock(data_.clock);
    ecal_sample_mutable_content->set_time(data_.time);
    ecal_sample_mutable_content->set_hash(data_.hash);
    ecal_sample_mutable_content->set_size((google::protobuf::int32)data_.len);
    ecal_sample_mutable_content->set_payload(data_.buf, data_.len);

    // send it
    size_t sent = 0;
    if (data_.loopback)
    {
      sent = eCAL::SendSample(&m_sample_snd_loopback, m_ecal_sample.topic().tname(), m_ecal_sample, m_udp_ipaddr, data_.bandwidth);
    }
    else
    {
      sent = eCAL::SendSample(&m_sample_snd_no_loopback, m_ecal_sample.topic().tname(), m_ecal_sample, m_udp_ipaddr, data_.bandwidth);
    }

    // log it
    if (sent == 0)
    {
      Logging::Log(log_level_fatal, "CDataWriterUDP::Send failed to send message !");
    }

    return(sent);
  }
}
