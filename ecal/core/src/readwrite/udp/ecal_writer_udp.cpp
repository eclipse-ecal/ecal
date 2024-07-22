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

#include "ecal_writer_udp.h"
#include "io/udp/ecal_udp_configurations.h"
#include "serialization/ecal_serialize_sample_payload.h"
#include "ecal/ecal_config.h"

#include <cstddef>

namespace eCAL
{
  CDataWriterUdpMC::CDataWriterUdpMC(const std::string& host_name_, const std::string& topic_name_, const std::string& topic_id_, const Publisher::UDP::Configuration& udp_config_) :
    m_config(udp_config_)
  {
    m_host_name   = host_name_;
    m_topic_name  = topic_name_;
    m_topic_id    = topic_id_;

    // set network attributes
    eCAL::UDP::SSenderAttr attr;
    attr.address   = UDP::GetTopicPayloadAddress(topic_name_);
    attr.port      = UDP::GetPayloadPort();
    attr.ttl       = UDP::GetMulticastTtl();
    attr.broadcast = UDP::IsBroadcast();
    attr.sndbuf    = eCAL::GetConfiguration().transport_layer.udp.send_buffer;

    // create udp/sample sender with activated loop-back
    attr.loopback = true;
    m_sample_sender_loopback = std::make_shared<UDP::CSampleSender>(attr);

    // create udp/sample sender without activated loop-back
    attr.loopback = false;
    m_sample_sender_no_loopback = std::make_shared<UDP::CSampleSender>(attr);
  }

  SWriterInfo CDataWriterUdpMC::GetInfo()
  {
    SWriterInfo info_;

    info_.name                 = "udp";
    info_.description          = "udp multicast data writer";

    info_.has_mode_local       = true;
    info_.has_mode_cloud       = true;

    info_.send_size_max        = -1;

    return info_;
  }

  bool CDataWriterUdpMC::Write(const void* const buf_, const SWriterAttr& attr_)
  {
    // create new sample
    Payload::Sample ecal_sample;
    ecal_sample.cmd_type = eCmdType::bct_set_sample;

    auto& ecal_sample_topic = ecal_sample.topic;
    ecal_sample_topic.hname = m_host_name;
    ecal_sample_topic.tname = m_topic_name;
    ecal_sample_topic.tid   = m_topic_id;

    // append content
    auto& ecal_sample_content = ecal_sample.content;
    ecal_sample_content.id               = attr_.id;
    ecal_sample_content.clock            = attr_.clock;
    ecal_sample_content.time             = attr_.time;
    ecal_sample_content.hash             = attr_.hash;
    ecal_sample_content.payload.type     = Payload::pl_raw;
    ecal_sample_content.payload.raw_addr = static_cast<const char*>(buf_);
    ecal_sample_content.payload.raw_size = attr_.len;

    // send it
    size_t sent = 0;
    if (SerializeToBuffer(ecal_sample, m_sample_buffer))
    {
      if (attr_.loopback)
      {
        if (m_sample_sender_loopback)
        {
          sent = m_sample_sender_loopback->Send(ecal_sample.topic.tname, m_sample_buffer);
        }
      }
      else
      {
        if (m_sample_sender_no_loopback)
        {
          sent = m_sample_sender_no_loopback->Send(ecal_sample.topic.tname, m_sample_buffer);
        }
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
