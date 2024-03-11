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

namespace
{
  // TODO: remove me with new CDescGate
  bool ApplyTopicDescription(const std::string& topic_name_, const eCAL::SDataTypeInformation& topic_info_)
  {
    if (eCAL::g_descgate() != nullptr)
    {
      // Calculate the quality of the current info
      eCAL::CDescGate::QualityFlags quality = eCAL::CDescGate::QualityFlags::NO_QUALITY;
      if (!topic_info_.name.empty() || !topic_info_.encoding.empty())
        quality |= eCAL::CDescGate::QualityFlags::TYPE_AVAILABLE;
      if (!topic_info_.descriptor.empty())
        quality |= eCAL::CDescGate::QualityFlags::DESCRIPTION_AVAILABLE;
      quality |= eCAL::CDescGate::QualityFlags::INFO_COMES_FROM_CORRECT_ENTITY;
      quality |= eCAL::CDescGate::QualityFlags::INFO_COMES_FROM_PRODUCER;

      return eCAL::g_descgate()->ApplyTopicDescription(topic_name_, topic_info_, quality);
    }
    return false;
  }
}

namespace eCAL
{
  //////////////////////////////////////////////////////////////////
  // CSubGate
  //////////////////////////////////////////////////////////////////
  std::atomic<bool> CSubGate::m_created;

  CSubGate::CSubGate() = default;

  CSubGate::~CSubGate()
  {
    Destroy();
  }

  void CSubGate::Create()
  {
    if(m_created) return;

    // initialize data reader layers
    CDataReader::InitializeLayers();

    m_created = true;
  }

  void CSubGate::Destroy()
  {
    if(!m_created) return;

    // destroy all remaining subscriber
    const std::unique_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    for (auto iter = m_topic_name_datareader_map.begin(); iter != m_topic_name_datareader_map.end(); ++iter)
    {
      iter->second->Destroy();
    }

    m_created = false;
  }

  bool CSubGate::Register(const std::string& topic_name_, const std::shared_ptr<CDataReader>& datareader_)
  {
    if(!m_created) return(false);

    // register reader
    const std::unique_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    m_topic_name_datareader_map.emplace(std::pair<std::string, std::shared_ptr<CDataReader>>(topic_name_, datareader_));

    return(true);
  }

  bool CSubGate::Unregister(const std::string& topic_name_, const std::shared_ptr<CDataReader>& datareader_)
  {
    if(!m_created) return(false);
    bool ret_state = false;

    const std::unique_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    auto res = m_topic_name_datareader_map.equal_range(topic_name_);
    for(auto iter = res.first; iter != res.second; ++iter)
    {
      if(iter->second == datareader_)
      {
        m_topic_name_datareader_map.erase(iter);
        ret_state = true;
        break;
      }
    }

    return(ret_state);
  }

  bool CSubGate::HasSample(const std::string& sample_name_)
  {
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    return(m_topic_name_datareader_map.find(sample_name_) != m_topic_name_datareader_map.end());
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
        Logging::Log(log_level_error, ecal_sample.topic.tname + " : payload received without layer definition !");
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
      std::vector<std::shared_ptr<CDataReader>> readers_to_apply;
        
      // Lock the sync map only while extracting the relevant shared pointers to the Datareaders.
      // Apply the samples to the readers afterward.
      {
        // apply sample to data reader
        const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
        auto res = m_topic_name_datareader_map.equal_range(ecal_sample.topic.tname);
        std::transform(
          res.first, res.second, std::back_inserter(readers_to_apply), [](const auto& match) { return match.second; }
        );
      }

      const auto& ecal_sample_content = ecal_sample.content;
      for (const auto& reader : readers_to_apply)
      {
        applied_size = reader->AddSample(
          ecal_sample.topic.tid,
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

  bool CSubGate::ApplySample(const std::string& topic_name_, const std::string& topic_id_, const char* buf_, size_t len_, long long id_, long long clock_, long long time_, size_t hash_, eTLayerType layer_)
  {
    if (!m_created) return false;

    // apply sample to data reader
    size_t applied_size(0);
    std::vector<std::shared_ptr<CDataReader>> readers_to_apply;

    // Lock the sync map only while extracting the relevant shared pointers to the Datareaders.
    // Apply the samples to the readers afterwards.
    {
      const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
      auto res = m_topic_name_datareader_map.equal_range(topic_name_);
      std::transform(
        res.first, res.second, std::back_inserter(readers_to_apply), [](const auto& match) { return match.second; }
      );
    }


    for (const auto& reader : readers_to_apply)
    {
      applied_size = reader->AddSample(topic_id_, buf_, len_, id_, clock_, time_, hash_, layer_);
    }

    return (applied_size > 0);
  }

  void CSubGate::ApplyLocPubRegistration(const Registration::Sample& ecal_sample_)
  {
    if(!m_created) return;

    // check topic name
    const auto&        ecal_topic = ecal_sample_.topic;
    const std::string& topic_name = ecal_topic.tname;
    if (topic_name.empty()) return;

    // store description
    const std::string& topic_id = ecal_topic.tid;
    ApplyTopicDescription(topic_name, ecal_topic.tdatatype);

    // get process id
    const std::string process_id = std::to_string(ecal_sample_.topic.pid);

    // handle local publisher connection
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    auto res = m_topic_name_datareader_map.equal_range(topic_name);
    for (auto iter = res.first; iter != res.second; ++iter)
    {
      // apply layer specific parameter
      for (const auto& tlayer : ecal_sample_.topic.tlayer)
      {
        iter->second->ApplyLocLayerParameter(process_id, topic_id, tlayer.type, tlayer.par_layer);
      }
      // inform for local publisher connection
      iter->second->ApplyLocPublication(process_id, topic_id, ecal_topic.tdatatype);
    }
  }

  void CSubGate::ApplyLocPubUnregistration(const Registration::Sample& ecal_sample_)
  {
    if (!m_created) return;

    // check topic name
    const auto&        ecal_topic = ecal_sample_.topic;
    const std::string& topic_name = ecal_topic.tname;
    const std::string& topic_id   = ecal_topic.tid;
    const std::string  process_id = std::to_string(ecal_sample_.topic.pid);

    // store description
    ApplyTopicDescription(topic_name, ecal_topic.tdatatype);

    // unregister local publisher
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    auto res = m_topic_name_datareader_map.equal_range(topic_name);
    for (auto iter = res.first; iter != res.second; ++iter)
    {
      iter->second->RemoveLocPublication(process_id, topic_id);
    }
  }

  void CSubGate::ApplyExtPubRegistration(const Registration::Sample& ecal_sample_)
  {
    if(!m_created) return;

    const auto&        ecal_topic = ecal_sample_.topic;
    const std::string& host_name  = ecal_topic.hname;
    const std::string& topic_name = ecal_topic.tname;
    const std::string& topic_id   = ecal_topic.tid;
    const std::string  process_id = std::to_string(ecal_topic.pid);

    // handle external publisher connection
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    auto res = m_topic_name_datareader_map.equal_range(topic_name);
    for (auto iter = res.first; iter != res.second; ++iter)
    {
      // apply layer specific parameter
      for (const auto& tlayer : ecal_sample_.topic.tlayer)
      {
        iter->second->ApplyExtLayerParameter(host_name, tlayer.type, tlayer.par_layer);
      }

      // inform for external publisher connection
      iter->second->ApplyExtPublication(host_name, process_id, topic_id, ecal_topic.tdatatype);
    }
  }

  void CSubGate::ApplyExtPubUnregistration(const Registration::Sample& ecal_sample_)
  {
    if (!m_created) return;

    const auto& ecal_sample = ecal_sample_.topic;
    const std::string& host_name  = ecal_sample.hname;
    const std::string& topic_name = ecal_sample.tname;
    const std::string& topic_id   = ecal_sample.tid;
    const std::string  process_id = std::to_string(ecal_sample.pid);

    // unregister local subscriber
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    auto res = m_topic_name_datareader_map.equal_range(topic_name);
    for (auto iter = res.first; iter != res.second; ++iter)
    {
      iter->second->RemoveExtPublication(host_name, process_id, topic_id);
    }
  }

  void CSubGate::RefreshRegistrations()
  {
    if (!m_created) return;

    // refresh reader registrations
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    for (const auto& iter : m_topic_name_datareader_map)
    {
      iter.second->RefreshRegistration();
    }
  }
}
