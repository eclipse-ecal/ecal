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
 * @brief  rtps data writer
**/

#include <ecal/ecal_log.h>

#include "ecal_def.h"
#include "ecal_config_hlp.h"
#include "ecal_writer_rtps.h"

#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/publisher/PublisherHistory.h>

eprosima::fastrtps::Participant* GetRTPSParticipant()
{
  static eprosima::fastrtps::Participant* pparticipant(nullptr);
  static string_message_PubSubType          stype;
  if (pparticipant == nullptr)
  {
    eprosima::fastrtps::ParticipantAttributes PParam;
    PParam.rtps.defaultSendPort                                                = 11511;
    PParam.rtps.use_IP6_to_send                                                = true;
    PParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol            = true;
    PParam.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol                   = true;
    PParam.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    PParam.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    PParam.rtps.builtin.domainId                                               = 0;
    PParam.rtps.builtin.leaseDuration                                          = eprosima::fastrtps::rtps::c_TimeInfinite;
    PParam.rtps.setName(eCALPAR(RTPS, DEFAULT_DOMAIN).c_str());
    pparticipant = eprosima::fastrtps::Domain::createParticipant(PParam);
    eprosima::fastrtps::Domain::registerType(pparticipant, &stype);
  }
  return pparticipant;
}

namespace eCAL
{
  CDataWriterRTPS::CDataWriterRTPS() : 
    m_publisher(nullptr)
  {
    // rtps writer default attributes
    m_attr.topic.topicKind                           = eprosima::fastrtps::rtps::NO_KEY;
    m_attr.topic.topicDataType                       = m_string_type.getName();
    m_attr.topic.topicName.clear();
    m_attr.topic.historyQos.kind                     = eprosima::fastrtps::KEEP_LAST_HISTORY_QOS;
    m_attr.topic.historyQos.depth                    = 8;
    m_attr.topic.resourceLimitsQos.max_samples       = 32; //-V112
    m_attr.topic.resourceLimitsQos.allocated_samples = 16;
    m_attr.times.heartbeatPeriod.seconds             = 2;
    m_attr.times.heartbeatPeriod.fraction            = 200 * 1000 * 1000;
    m_attr.qos.m_reliability.kind                    = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
    m_attr.historyMemoryPolicy                       = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    m_attr.qos.m_publishMode.kind                    = eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE;
    m_attr.qos.m_partition.push_back("rt");
  }

  CDataWriterRTPS::~CDataWriterRTPS()
  {
    Destroy();
  }

  void CDataWriterRTPS::GetInfo(SWriterInfo info_)
  {
    info_.name                 = "rtps";
    info_.description          = "fastRTPS data writer";

    info_.has_mode_local       = true;
    info_.has_mode_cloud       = true;

    info_.has_qos_history_kind = true;
    info_.has_qos_reliability  = true;

    info_.send_size_max        = -1;
  }

  bool CDataWriterRTPS::Create(const std::string & host_name_, const std::string & topic_name_, const std::string & topic_id_)
  {
    if (m_created) return false;

    m_host_name  = host_name_;
    m_topic_name = topic_name_;
    m_topic_id   = topic_id_;

    // create participant
    if (GetRTPSParticipant() == nullptr) return false;

    // create publisher
    if (!CreateRtpsPub(topic_name_)) return false;

    m_created = true;
    return true;
  }

  bool CDataWriterRTPS::Destroy()
  {
    if (!m_created) return false;

    // destroy publisher
    DestroyRtpsPub();

    m_created = false;
    return true;
  }

  bool CDataWriterRTPS::SetQOS(const QOS::SWriterQOS & qos_)
  {
    if (m_created) return false;
    m_qos = qos_;

    switch (qos_.history_kind)
    {
    case QOS::keep_last_history_qos:
      m_attr.topic.historyQos.kind = eprosima::fastrtps::KEEP_LAST_HISTORY_QOS;
      m_attr.topic.historyQos.depth = qos_.history_kind_depth;
      break;
    case QOS::keep_all_history_qos:
      m_attr.topic.historyQos.kind = eprosima::fastrtps::KEEP_ALL_HISTORY_QOS;
      break;
    default:
      break;
    }

    switch (qos_.reliability)
    {
    case QOS::reliable_reliability_qos:
      m_attr.qos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
      break;
    case QOS::best_effort_reliability_qos:
      m_attr.qos.m_reliability.kind = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
      break;
    default:
      break;
    }

    return true;
  }

  size_t CDataWriterRTPS::Send(const SWriterData& data_)
  {
    if (!m_created) return 0;

    // send it
    bool sent(false);
    if (m_listener.n_matched > 0)
    {
      // fill rtps message
      m_string_msg.tid(m_topic_id);
      m_string_msg.id(data_.id);
      m_string_msg.clock(data_.clock);
      m_string_msg.time(data_.time);
      m_string_msg.compressed(0);
      m_string_msg.size(data_.len);
      m_string_msg.hash(data_.hash);
      m_string_msg.payload().resize(data_.len);
      memcpy(&m_string_msg.payload()[0], data_.buf, data_.len);

      // send it
      sent = m_publisher->write((void*)&m_string_msg);

      // log it
      if (sent == false)
      {
        Logging::Log(log_level_fatal, "CDataWriterRTPS::Send failed to send message !");
        return(0);
      }
    }

    return(data_.len);
  }

  bool CDataWriterRTPS::CreateRtpsPub(const std::string& topic_name_)
  {
    if (m_publisher) return false;

    // create publisher
    m_attr.topic.topicName = topic_name_;
    m_publisher = eprosima::fastrtps::Domain::createPublisher(GetRTPSParticipant(), m_attr, (eprosima::fastrtps::PublisherListener*)&m_listener);
    if (m_publisher == nullptr) return false;

    return true;
  }

  bool CDataWriterRTPS::DestroyRtpsPub()
  {
    if (m_publisher == nullptr) return false;

    // destroy publisher
    eprosima::fastrtps::Domain::removePublisher(m_publisher);
    m_publisher = nullptr;

    return true;
  }

  void CDataWriterRTPS::PubListener::onPublicationMatched(eprosima::fastrtps::Publisher* /*pub_*/, eprosima::fastrtps::rtps::MatchingInfo& info_)
  {
    if (info_.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
    {
      n_matched++;
      Logging::Log(log_level_debug1, "CDataWriterRTPS::Publisher matched.");
    }
    else
    {
      n_matched--;
      Logging::Log(log_level_debug1, "CDataWriterRTPS::Publisher unmatched.");
    }
  }
}
