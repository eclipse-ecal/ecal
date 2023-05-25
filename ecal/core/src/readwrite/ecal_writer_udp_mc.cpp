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

#include <ecal/ecal_log.h>
#include <ecal/ecal_config.h>

#include "ecal_def.h"
#include "ecal_writer_udp_mc.h"

#include "io/udp_configurations.h"

namespace eCAL
{
  CDataWriterUdpMC::~CDataWriterUdpMC()
  {
    Destroy();
  }

  SWriterInfo CDataWriterUdpMC::GetInfo()
  {
    SWriterInfo info_;

    info_.name                 = "udp";
    info_.description          = "UDP multicast data writer";

    info_.has_mode_local       = true;
    info_.has_mode_cloud       = true;

    info_.has_qos_history_kind = false;
    info_.has_qos_reliability  = false;

    info_.send_size_max        = -1;

    return info_;
  }

  bool CDataWriterUdpMC::Create(const std::string & host_name_, const std::string & topic_name_, const std::string & topic_id_)
  {
    if (m_created) return false;

    m_host_name   = host_name_;
    m_topic_name  = topic_name_;
    m_topic_id    = topic_id_;

    m_udp_ipaddr = UDP::GetTopicMulticastAddress(topic_name_);

    // set network attributes
    SSenderAttr attr;
    attr.ipaddr     = m_udp_ipaddr;
    attr.port       = Config::GetUdpMulticastPort() + NET_UDP_MULTICAST_PORT_SAMPLE_OFF;
    attr.ttl        = Config::GetUdpMulticastTtl();
    attr.sndbuf     = Config::GetUdpMulticastSndBufSizeBytes();

    // create udp/sample sender with activated loop-back
    attr.loopback   = true;
    m_sample_sender_loopback = std::make_shared<CSampleSender>(attr);

    // create udp/sample sender without activated loop-back
    attr.loopback   = false;
    m_sample_sender_no_loopback = std::make_shared<CSampleSender>(attr);

    m_created = true;
    return true;
  }

  bool CDataWriterUdpMC::Destroy()
  {
    if (!m_created) return false;

    m_sample_sender_loopback.reset();
    m_sample_sender_no_loopback.reset();

    m_created = false;
    return true;
  }

  bool CDataWriterUdpMC::Write(const void* const buf_, const SWriterAttr& attr_)
  {
    if (!m_created) return false;

    // create new sample
    m_ecal_sample.Clear();
    m_ecal_sample.set_cmd_type(eCAL::pb::bct_set_sample);
    auto *ecal_sample_mutable_topic = m_ecal_sample.mutable_topic();
    ecal_sample_mutable_topic->set_hname(m_host_name);
    ecal_sample_mutable_topic->set_tname(m_topic_name);
    ecal_sample_mutable_topic->set_tid(m_topic_id);

    // set layer
    auto *layer = ecal_sample_mutable_topic->add_tlayer();
    layer->set_type(eCAL::pb::eTLayerType::tl_ecal_udp_mc);
    layer->set_confirmed(true);

    // append content
    auto *ecal_sample_mutable_content = m_ecal_sample.mutable_content();
    ecal_sample_mutable_content->set_id(attr_.id);
    ecal_sample_mutable_content->set_clock(attr_.clock);
    ecal_sample_mutable_content->set_time(attr_.time);
    ecal_sample_mutable_content->set_hash(attr_.hash);
    ecal_sample_mutable_content->set_size((google::protobuf::int32)attr_.len);
    ecal_sample_mutable_content->set_payload(buf_, attr_.len);

    // send it
    size_t sent = 0;
    if (attr_.loopback)
    {
      if (m_sample_sender_loopback)
      {
        sent = m_sample_sender_loopback->SendSample(m_ecal_sample.topic().tname(), m_ecal_sample, attr_.bandwidth);
      }
    }
    else
    {
      if (m_sample_sender_no_loopback)
      {
        sent = m_sample_sender_no_loopback->SendSample(m_ecal_sample.topic().tname(), m_ecal_sample, attr_.bandwidth);
      }
    }

    // log it
    if (sent == 0)
    {
      Logging::Log(log_level_fatal, "CDataWriterUDP::Send failed to send message !");
    }

    return(sent > 0);
  }
}
