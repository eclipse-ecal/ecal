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
 * @brief  eCAL rtps reader
**/

#include "ecal_global_accessors.h"

#include "pubsub/ecal_subgate.h"
#include "readwrite/ecal_reader_rtps.h"

#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/subscriber/Subscriber.h>

eprosima::fastrtps::Participant* GetRTPSParticipant();

namespace eCAL
{
  template<> std::shared_ptr<CRtpsLayer> CReaderLayer<CRtpsLayer>::layer(nullptr);

  //////////////////////////////////////////////////////////////////
  // CDataReaderRtps
  //////////////////////////////////////////////////////////////////
  CDataReaderRtps::CDataReaderRtps()
  {
  }

  bool CDataReaderRtps::CreateRtpsSub(const std::string& topic_name_, const QOS::SReaderQOS& qos_)
  {
    std::lock_guard<std::mutex> lock(m_sub_map_sync);

    auto iter = m_sub_map.find(topic_name_);
    if (iter != m_sub_map.end()) return false;

    auto sub = new CSubscriberRtps(topic_name_, qos_);
    m_sub_map[topic_name_] = sub;

    return true;
  }

  bool CDataReaderRtps::DestroyRtpsSub(const std::string& topic_name_)
  {
    std::lock_guard<std::mutex> lock(m_sub_map_sync);

    auto iter = m_sub_map.find(topic_name_);
    if (iter != m_sub_map.end()) return false;

    delete iter->second;
    m_sub_map.erase(iter);

    return true;
  }

  //////////////////////////////////////////////////////////////////
  // CSubscriberRtps
  //////////////////////////////////////////////////////////////////
  CDataReaderRtps::CSubscriberRtps::CSubscriberRtps(const std::string& topic_name_, QOS::SReaderQOS qos_)
  {
    if (GetRTPSParticipant() == nullptr) return;

    // rtps reader default attributes
    m_attr.topic.topicName                           = topic_name_;
    m_attr.topic.topicKind                           = eprosima::fastrtps::rtps::NO_KEY;
    m_attr.topic.topicDataType                       = m_string_type.getName();
    m_attr.topic.historyQos.kind                     = eprosima::fastrtps::KEEP_LAST_HISTORY_QOS;
    m_attr.topic.historyQos.depth                    = 8;
    m_attr.topic.resourceLimitsQos.max_samples       = 32; //-V112
    m_attr.topic.resourceLimitsQos.allocated_samples = 16;
    m_attr.qos.m_reliability.kind                    = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
    m_attr.historyMemoryPolicy                       = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    m_attr.qos.m_partition.push_back("rt");

    // store qos
    m_qos = qos_;

    // qos HistroryKind
    switch (m_qos.history_kind)
    {
    case QOS::keep_last_history_qos:
      m_attr.topic.historyQos.kind  = eprosima::fastrtps::KEEP_LAST_HISTORY_QOS;
      m_attr.topic.historyQos.depth = m_qos.history_kind_depth;
      break;
    case QOS::keep_all_history_qos:
      m_attr.topic.historyQos.kind  = eprosima::fastrtps::KEEP_ALL_HISTORY_QOS;
      break;
    default:
      break;
    }

    // qos Reliability
    switch (m_qos.reliability)
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

    // create subscriber
    m_subscriber = eprosima::fastrtps::Domain::createSubscriber(GetRTPSParticipant(), m_attr, (eprosima::fastrtps::SubscriberListener*)&m_listener);
  }

  CDataReaderRtps::CSubscriberRtps::~CSubscriberRtps()
  {
    if (!m_subscriber) return;

    // destroy subscriber
    eprosima::fastrtps::Domain::removeSubscriber(m_subscriber);
    m_subscriber = nullptr;
  }

  void CDataReaderRtps::CSubscriberRtps::SubListener::onSubscriptionMatched(eprosima::fastrtps::Subscriber* /*sub_*/, eprosima::fastrtps::rtps::MatchingInfo& info_)
  {
    if (info_.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
    {
      n_matched++;
      Logging::Log(log_level_debug1, "CDataReaderRTPS::Subscriber matched.");
    }
    else
    {
      n_matched--;
      Logging::Log(log_level_debug1, "CDataReaderRTPS::Subscriber unmatched.");
    }
  }

  void CDataReaderRtps::CSubscriberRtps::SubListener::onNewDataMessage(eprosima::fastrtps::Subscriber* sub_)
  {
    if (sub_->takeNextData((void*)&m_string_msg, &m_info))
    {
      if (m_info.sampleKind == eprosima::fastrtps::rtps::ALIVE)
      {
        // count received samples
        this->n_samples++;

        // apply data to subscriber gate
        if (g_subgate()) g_subgate()->ApplySample(sub_->getAttributes().topic.topicName, m_string_msg.tid(), m_string_msg.payload().data(), m_string_msg.payload().size(), m_string_msg.id(), m_string_msg.clock(), m_string_msg.time(), static_cast<size_t>(m_string_msg.hash()), eCAL::pb::eTLayerType::tl_rtps);
      }
    }
  }
};
