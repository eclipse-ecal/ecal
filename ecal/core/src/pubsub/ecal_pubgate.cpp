/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * @brief  eCAL publisher gateway class
**/

#include "ecal_pubgate.h"
#include "ecal_globals.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <utility>

namespace eCAL
{
  //////////////////////////////////////////////////////////////////
  // CPubGate
  //////////////////////////////////////////////////////////////////
  std::atomic<bool> CPubGate::m_created;
  CPubGate::CPubGate() :
    m_share_type(true),
    m_share_desc(true)
  {
  }

  CPubGate::~CPubGate()
  {
    Stop();
  }

  void CPubGate::Start()
  {
    if(m_created) return;
    m_created = true;
  }

  void CPubGate::Stop()
  {
    if(!m_created) return;

    // stop & destroy all remaining publisher
    const std::unique_lock<std::shared_timed_mutex> lock(m_topic_name_datawriter_sync);
    for (const auto& datawriter : m_topic_name_datawriter_map)
    {
      datawriter.second->Stop();
    }
    m_topic_name_datawriter_map.clear();

    m_created = false;
  }

  void CPubGate::ShareType(bool state_)
  {
    m_share_type = state_;
  }

  void CPubGate::ShareDescription(bool state_)
  {
    m_share_desc = state_;
  }

  bool CPubGate::Register(const std::string& topic_name_, const std::shared_ptr<CDataWriter>& datawriter_)
  {
    if(!m_created) return(false);

    // register writer and multicast group
    const std::unique_lock<std::shared_timed_mutex> lock(m_topic_name_datawriter_sync);
    m_topic_name_datawriter_map.emplace(std::pair<std::string, std::shared_ptr<CDataWriter>>(topic_name_, datawriter_));

    return(true);
  }

  bool CPubGate::Unregister(const std::string& topic_name_, const std::shared_ptr<CDataWriter>& datawriter_)
  {
    if(!m_created) return(false);
    bool ret_state = false;

    const std::unique_lock<std::shared_timed_mutex> lock(m_topic_name_datawriter_sync);
    auto res = m_topic_name_datawriter_map.equal_range(topic_name_);
    for(auto iter = res.first; iter != res.second; ++iter)
    {
      if(iter->second == datawriter_)
      {
        m_topic_name_datawriter_map.erase(iter);
        ret_state = true;
        break;
      }
    }

    return(ret_state);
  }

  void CPubGate::ApplySubRegistration(const Registration::Sample& ecal_sample_)
  {
    if(!m_created) return;

    const auto&        ecal_topic = ecal_sample_.topic;
    const std::string& topic_name = ecal_topic.tname;

    // check topic name
    if (topic_name.empty()) return;

    CDataWriter::SSubscriptionInfo subscription_info;
    subscription_info.host_name                  = ecal_topic.hname;
    subscription_info.topic_id                   = ecal_topic.tid;
    subscription_info.process_id                 = ecal_topic.pid;
    const SDataTypeInformation topic_information = ecal_topic.tdatatype;

    CDataWriter::SLayerStates layer_states;
    for (const auto& layer : ecal_topic.tlayer)
    {
      // transport layer versions 0 and 1 did not support dynamic layer enable feature
      // so we set assume layer is enabled if we receive a registration in this case
      if (layer.enabled || (layer.version < 2))
      {
        switch (layer.type)
        {
        case TLayer::tlayer_udp_mc:
          layer_states.udp.read_enabled = true;
          break;
        case TLayer::tlayer_shm:
          layer_states.shm.read_enabled = true;
          break;
        case TLayer::tlayer_tcp:
          layer_states.tcp.read_enabled = true;
          break;
        default:
          break;
        }
      }
    }

    std::string reader_par;
#if 0
    for (const auto& layer : ecal_sample.tlayer())
    {
      // layer parameter as protobuf message
      // this parameter is not used at all currently
      // for subscriber registrations
      reader_par = layer.par_layer().SerializeAsString();
    }
#endif

    // register subscriber
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datawriter_sync);
    auto res = m_topic_name_datawriter_map.equal_range(topic_name);
    for(TopicNameDataWriterMapT::const_iterator iter = res.first; iter != res.second; ++iter)
    {
      iter->second->ApplySubscription(subscription_info, topic_information, layer_states, reader_par);
    }
  }

  void CPubGate::ApplySubUnregistration(const Registration::Sample& ecal_sample_)
  {
    if (!m_created) return;

    const auto& ecal_topic = ecal_sample_.topic;
    const std::string& topic_name = ecal_topic.tname;

    // check topic name
    if (topic_name.empty()) return;

    CDataWriter::SSubscriptionInfo subscription_info;
    subscription_info.host_name  = ecal_topic.hname;
    subscription_info.topic_id   = ecal_topic.tid;
    subscription_info.process_id = ecal_topic.pid;

    // unregister subscriber
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datawriter_sync);
    auto res = m_topic_name_datawriter_map.equal_range(topic_name);
    for (TopicNameDataWriterMapT::const_iterator iter = res.first; iter != res.second; ++iter)
    {
      iter->second->RemoveSubscription(subscription_info);
    }
  }

  void CPubGate::GetRegistrations(Registration::SampleList& reg_sample_list_)
  {
    if (!m_created) return;

    // read reader registrations
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datawriter_sync);
    for (const auto& iter : m_topic_name_datawriter_map)
    {
      reg_sample_list_.samples.emplace_back(iter.second->GetRegistration());
    }
  }
}
