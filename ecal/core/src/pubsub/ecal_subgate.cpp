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
 * @brief  eCAL subscriber gateway class
**/

#include "pubsub/ecal_subgate.h"
#include "ecal_globals.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <iterator>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <utility>
#include <vector>

namespace eCAL
{
  //////////////////////////////////////////////////////////////////
  // CSubGate
  //////////////////////////////////////////////////////////////////
  std::atomic<bool> CSubGate::m_created;

  CSubGate::CSubGate() = default;

  CSubGate::~CSubGate()
  {
    Stop();
  }

  void CSubGate::Start()
  {
    if(m_created) return;
    m_created = true;
  }

  void CSubGate::Stop()
  {
    if(!m_created) return;

    // stop & destroy all remaining subscriber
    const std::unique_lock<std::shared_timed_mutex> lock(m_topic_name_subscriber_mutex);
    m_topic_name_subscriber_map.clear();

    m_created = false;
  }

  bool CSubGate::Register(const std::string& topic_name_, const std::shared_ptr<CSubscriberImpl>& datareader_)
  {
    if(!m_created) return(false);

    // register reader
    const std::unique_lock<std::shared_timed_mutex> lock(m_topic_name_subscriber_mutex);
    m_topic_name_subscriber_map.emplace(std::pair<std::string, std::shared_ptr<CSubscriberImpl>>(topic_name_, datareader_));

    return(true);
  }

  bool CSubGate::Unregister(const std::string& topic_name_, const std::shared_ptr<CSubscriberImpl>& datareader_)
  {
    if(!m_created) return(false);
    bool ret_state = false;

    const std::unique_lock<std::shared_timed_mutex> lock(m_topic_name_subscriber_mutex);
    auto res = m_topic_name_subscriber_map.equal_range(topic_name_);
    for(auto iter = res.first; iter != res.second; ++iter)
    {
      if(iter->second == datareader_)
      {
        m_topic_name_subscriber_map.erase(iter);
        ret_state = true;
        break;
      }
    }

    return(ret_state);
  }

  bool CSubGate::HasSample(const std::string& sample_name_)
  {
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_subscriber_mutex);
    return(m_topic_name_subscriber_map.find(sample_name_) != m_topic_name_subscriber_map.end());
  }

  bool CSubGate::ApplySample(const char* serialized_sample_data_, size_t serialized_sample_size_, eTLayerType layer_)
  {
    if(!m_created) return false;

    Payload::Sample ecal_sample;
    if (!DeserializeFromBuffer(serialized_sample_data_, serialized_sample_size_, ecal_sample)) return false;

    size_t applied_size(0);
    switch (ecal_sample.cmd_type)
    {
    case bct_set_sample:
    {
#ifndef NDEBUG
      // check layer
      if (layer_ == eTLayerType::tl_none)
      {
        // log it
        Logging::Log(Logging::log_level_error, ecal_sample.topic_info.topic_name + " : payload received without layer definition !");
      }
#endif

      // extract payload
      const char* payload_addr = nullptr;
      size_t      payload_size = 0;
      switch (ecal_sample.content.payload.type)
      {
      case eCAL::Payload::pl_raw:
        payload_addr = ecal_sample.content.payload.raw_addr;
        payload_size = ecal_sample.content.payload.raw_size;
        break;
      case eCAL::Payload::pl_vec:
        payload_addr = ecal_sample.content.payload.vec.data();
        payload_size = ecal_sample.content.payload.vec.size();
        break;
      default:
        break;
      }

      // apply sample to data reader
      std::vector<std::shared_ptr<CSubscriberImpl>> readers_to_apply;
        
      // Lock the sync map only while extracting the relevant shared pointers to the Datareaders.
      // Apply the samples to the readers afterward.
      {
        // apply sample to data reader
        const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_subscriber_mutex);
        auto res = m_topic_name_subscriber_map.equal_range(ecal_sample.topic_info.topic_name);
        std::transform(
          res.first, res.second, std::back_inserter(readers_to_apply), [](const auto& match) { return match.second; }
        );
      }

      const auto& ecal_sample_content = ecal_sample.content;
      for (const auto& reader : readers_to_apply)
      {
        applied_size = reader->ApplySample(
          ecal_sample.topic_info,
          payload_addr,
          payload_size,
          ecal_sample_content.id,
          ecal_sample_content.clock,
          ecal_sample_content.time,
          static_cast<size_t>(ecal_sample_content.hash),
          layer_
        );
      }
    }
    break;
    default:
      break;
    }

    return (applied_size > 0);
  }

  bool CSubGate::ApplySample(const Payload::TopicInfo& topic_info_, const char* buf_, size_t len_, long long id_, long long clock_, long long time_, size_t hash_, eTLayerType layer_)
  {
    if (!m_created) return false;

    // apply sample to data reader
    size_t applied_size(0);
    std::vector<std::shared_ptr<CSubscriberImpl>> readers_to_apply;

    // Lock the sync map only while extracting the relevant shared pointers to the Datareaders.
    // Apply the samples to the readers afterwards.
    {
      const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_subscriber_mutex);
      auto res = m_topic_name_subscriber_map.equal_range(topic_info_.topic_name);
      std::transform(
        res.first, res.second, std::back_inserter(readers_to_apply), [](const auto& match) { return match.second; }
      );
    }


    for (const auto& reader : readers_to_apply)
    {
      applied_size = reader->ApplySample(topic_info_, buf_, len_, id_, clock_, time_, hash_, layer_);
    }

    return (applied_size > 0);
  }

  void CSubGate::ApplyPublisherRegistration(const Registration::Sample& ecal_sample_)
  {
    if(!m_created) return;

    const auto&        ecal_topic = ecal_sample_.topic;
    const std::string& topic_name = ecal_topic.topic_name;

    // check topic name
    if (topic_name.empty()) return;

    const auto& publication_info = ecal_sample_.identifier;
    const SDataTypeInformation& topic_information = ecal_topic.datatype_information;

    CSubscriberImpl::SLayerStates layer_states;
    for (const auto& layer : ecal_topic.transport_layer)
    {
      // transport layer versions 0 and 1 did not support dynamic layer enable feature
      // so we set assume layer is enabled if we receive a registration in this case
      if (layer.enabled || layer.version < 2)
      {
        switch (layer.type)
        {
        case tl_ecal_udp:
          layer_states.udp.write_enabled = true;
          break;
        case tl_ecal_shm:
          layer_states.shm.write_enabled = true;
          break;
        case tl_ecal_tcp:
          layer_states.tcp.write_enabled = true;
          break;
        default:
          break;
        }
      }
    }

    // register publisher
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_subscriber_mutex);
    auto res = m_topic_name_subscriber_map.equal_range(topic_name);
    for (auto iter = res.first; iter != res.second; ++iter)
    {
      // apply layer specific parameter
      for (const auto& transport_layer : ecal_sample_.topic.transport_layer)
      {
        iter->second->ApplyLayerParameter(publication_info, transport_layer.type, transport_layer.par_layer);
      }
      iter->second->ApplyPublisherRegistration(publication_info, topic_information, layer_states);
    }
  }

  void CSubGate::ApplyPublisherUnregistration(const Registration::Sample& ecal_sample_)
  {
    if (!m_created) return;

    const auto&        ecal_topic = ecal_sample_.topic;
    const std::string& topic_name = ecal_topic.topic_name;

    // check topic name
    if (topic_name.empty()) return;

    const auto& publication_info = ecal_sample_.identifier;
    const SDataTypeInformation& topic_information = ecal_topic.datatype_information;

    // unregister publisher
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_subscriber_mutex);
    auto res = m_topic_name_subscriber_map.equal_range(topic_name);
    for (auto iter = res.first; iter != res.second; ++iter)
    {
      iter->second->ApplyPublisherUnregistration(publication_info, topic_information);
    }
  }

  void CSubGate::GetRegistrations(Registration::SampleList& reg_sample_list_)
  {
    if (!m_created) return;

    // read reader registrations
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_subscriber_mutex);
    for (const auto& iter : m_topic_name_subscriber_map)
    {
      iter.second->GetRegistration(reg_sample_list_.push_back());
    }
  }
}
