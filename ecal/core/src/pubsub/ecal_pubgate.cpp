/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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
  CPubGate::CPubGate() = default;

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
    const std::unique_lock<std::shared_timed_mutex> lock(m_topic_name_publisher_mutex);
    m_topic_name_publisher_map.clear();

    m_created = false;
  }

  bool CPubGate::Register(const std::string& topic_name_, const std::shared_ptr<CPublisherImpl>& publisher_)
  {
    if(!m_created) return(false);

    // register writer and multicast group
    const std::unique_lock<std::shared_timed_mutex> lock(m_topic_name_publisher_mutex);
    m_topic_name_publisher_map.emplace(std::pair<std::string, std::shared_ptr<CPublisherImpl>>(topic_name_, publisher_));

    return(true);
  }

  bool CPubGate::Unregister(const std::string& topic_name_, const std::shared_ptr<CPublisherImpl>& publisher_)
  {
    if(!m_created) return(false);
    bool ret_state = false;

    const std::unique_lock<std::shared_timed_mutex> lock(m_topic_name_publisher_mutex);
    auto res = m_topic_name_publisher_map.equal_range(topic_name_);
    for(auto iter = res.first; iter != res.second; ++iter)
    {
      if(iter->second == publisher_)
      {
        m_topic_name_publisher_map.erase(iter);
        ret_state = true;
        break;
      }
    }

    return(ret_state);
  }

  void CPubGate::ApplySubscriberRegistration(const Registration::Sample& ecal_sample_)
  {
    if(!m_created) return;

    const auto&        ecal_topic = ecal_sample_.topic;
    const std::string& topic_name = ecal_topic.topic_name;

    // check topic name
    if (topic_name.empty()) return;

    // TODO: Substitute ProducerInfo type
    const auto& subscription_info = ecal_sample_.identifier;
    const SDataTypeInformation& topic_information = ecal_topic.datatype_information;

    CPublisherImpl::SLayerStates layer_states;
    for (const auto& layer : ecal_topic.transport_layer)
    {
      // transport layer versions 0 and 1 did not support dynamic layer enable feature
      // so we set assume layer is enabled if we receive a registration in this case
      if (layer.enabled || (layer.version < 2))
      {
        switch (layer.type)
        {
        case tl_ecal_udp:
          layer_states.udp.read_enabled = true;
          break;
        case tl_ecal_shm:
          layer_states.shm.read_enabled = true;
          break;
        case tl_ecal_tcp:
          layer_states.tcp.read_enabled = true;
          break;
        default:
          break;
        }
      }
    }

    std::string reader_par;
#if 0
    for (const auto& layer : ecal_sample.transport_layer())
    {
      // layer parameter as protobuf message
      // this parameter is not used at all currently
      // for subscriber registrations
      reader_par = layer.par_layer().SerializeAsString();
    }
#endif

    // register subscriber
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_publisher_mutex);
    auto res = m_topic_name_publisher_map.equal_range(topic_name);
    for(TopicNamePublisherMapT::const_iterator iter = res.first; iter != res.second; ++iter)
    {
      iter->second->ApplySubscriberRegistration(subscription_info, topic_information, layer_states, reader_par);
    }
  }

  void CPubGate::ApplySubscriberUnregistration(const Registration::Sample& ecal_sample_)
  {
    if (!m_created) return;

    const auto& ecal_topic = ecal_sample_.topic;
    const std::string& topic_name = ecal_topic.topic_name;

    // check topic name
    if (topic_name.empty()) return;

    const auto& subscription_info = ecal_sample_.identifier;
    const SDataTypeInformation& topic_information = ecal_topic.datatype_information;

    // unregister subscriber
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_publisher_mutex);
    auto res = m_topic_name_publisher_map.equal_range(topic_name);
    for (TopicNamePublisherMapT::const_iterator iter = res.first; iter != res.second; ++iter)
    {
      iter->second->ApplySubscriberUnregistration(subscription_info, topic_information);
    }
  }

  void CPubGate::GetRegistrations(Registration::SampleList& reg_sample_list_)
  {
    if (!m_created) return;

    // read reader registrations
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_publisher_mutex);
    for (const auto& iter : m_topic_name_publisher_map)
    {
      iter.second->GetRegistration(reg_sample_list_.push_back());
    }
  }
}
