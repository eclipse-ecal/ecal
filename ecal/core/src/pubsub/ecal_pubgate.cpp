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
 * @brief  eCAL publisher gateway class
**/

#include <ecal/ecal.h>

#include "ecal_def.h"
#include "ecal_config_reader_hlp.h"
#include "ecal_pubgate.h"
#include "ecal_descgate.h"

#include <iterator>
#include <atomic>

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
  };

  CPubGate::~CPubGate()
  {
    Destroy();
  }

  void CPubGate::Create()
  {
    if(m_created) return;

    m_created = true;
  }

  void CPubGate::Destroy()
  {
    if(!m_created) return;

    // destroy all remaining publisher
    std::unique_lock<std::shared_timed_mutex> lock(m_topic_name_datawriter_sync);
    for (auto iter = m_topic_name_datawriter_map.begin(); iter != m_topic_name_datawriter_map.end(); ++iter)
    {
      iter->second->Destroy();
    }

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

  bool CPubGate::Register(const std::string& topic_name_, CDataWriter* datawriter_)
  {
    if(!m_created) return(false);

    // register writer and multicast group
    std::unique_lock<std::shared_timed_mutex> lock(m_topic_name_datawriter_sync);
    m_topic_name_datawriter_map.emplace(std::pair<std::string, CDataWriter*>(topic_name_, datawriter_));

    return(true);
  }

  bool CPubGate::Unregister(const std::string& topic_name_, CDataWriter* datawriter_)
  {
    if(!m_created) return(false);
    bool ret_state = false;

    std::unique_lock<std::shared_timed_mutex> lock(m_topic_name_datawriter_sync);
    auto res = m_topic_name_datawriter_map.equal_range(topic_name_);
    for(TopicNameDataWriterMapT::iterator iter = res.first; iter != res.second; ++iter)
    {
      if(iter->second == datawriter_)
      {
        m_topic_name_datawriter_map.erase(iter);
        break;
      }
    }

    return(ret_state);
  }

  void CPubGate::ApplyLocSubRegistration(const eCAL::pb::Sample& ecal_sample_)
  {
    if(!m_created) return;

    const auto& ecal_sample_topic = ecal_sample_.topic();
    const std::string& topic_name = ecal_sample_topic.tname();
    const std::string& topic_id   = ecal_sample_topic.tid();
    const std::string& topic_type = ecal_sample_topic.ttype();
    const std::string& topic_desc = ecal_sample_topic.tdesc();
    const std::string  process_id = std::to_string(ecal_sample_topic.pid());

    std::string reader_par;
    for (const auto& layer : ecal_sample_topic.tlayer())
    {
      // layer parameter as protobuf message
      // this parameter is not used at all currently
      // for local subscriber registrations
      reader_par = layer.par_layer().SerializeAsString();
    }

    // store description
    if (g_descgate())
    {
      // Calculate the quality of the current info
      ::eCAL::CDescGate::QualityFlags quality = ::eCAL::CDescGate::QualityFlags::NO_QUALITY;
      if (!topic_type.empty())
        quality |= ::eCAL::CDescGate::QualityFlags::TYPE_AVAILABLE;
      if (!topic_desc.empty())
        quality |= ::eCAL::CDescGate::QualityFlags::DESCRIPTION_AVAILABLE;
      quality |= ::eCAL::CDescGate::QualityFlags::INFO_COMES_FROM_CORRECT_TOPIC;

      g_descgate()->ApplyTopicDescription(topic_name, topic_type, topic_desc, quality);
    }

    // register local subscriber
    std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datawriter_sync);
    auto res = m_topic_name_datawriter_map.equal_range(topic_name);
    for(TopicNameDataWriterMapT::const_iterator iter = res.first; iter != res.second; ++iter)
    {
      iter->second->ApplyLocSubscription(process_id, topic_id, topic_type, topic_desc, reader_par);
    }
  }

  void CPubGate::ApplyExtSubRegistration(const eCAL::pb::Sample& ecal_sample_)
  {
    if(!m_created) return;

    const auto& ecal_sample_topic = ecal_sample_.topic();
    const std::string& host_name  = ecal_sample_topic.hname();
    const std::string& topic_name = ecal_sample_topic.tname();
    const std::string& topic_id   = ecal_sample_topic.tid();
    const std::string& topic_type = ecal_sample_topic.ttype();
    const std::string& topic_desc = ecal_sample_topic.tdesc();
    const std::string  process_id = std::to_string(ecal_sample_topic.pid());

    std::string reader_par;
    for (const auto& layer : ecal_sample_topic.tlayer())
    {
      // layer parameter as protobuf message
      // this parameter is not used at all currently
      // for external subscriber registrations
      reader_par = layer.par_layer().SerializeAsString();
    }

    // store description
    if (g_descgate())
    {
      // Calculate the quality of the current info
      ::eCAL::CDescGate::QualityFlags quality = ::eCAL::CDescGate::QualityFlags::NO_QUALITY;
      if (!topic_type.empty())
        quality |= ::eCAL::CDescGate::QualityFlags::TYPE_AVAILABLE;
      if (!topic_desc.empty())
        quality |= ::eCAL::CDescGate::QualityFlags::DESCRIPTION_AVAILABLE;
      quality |= ::eCAL::CDescGate::QualityFlags::INFO_COMES_FROM_CORRECT_TOPIC;

      g_descgate()->ApplyTopicDescription(topic_name, topic_type, topic_desc, quality);
    }

    // register external subscriber
    std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datawriter_sync);
    auto res = m_topic_name_datawriter_map.equal_range(topic_name);
    for(TopicNameDataWriterMapT::const_iterator iter = res.first; iter != res.second; ++iter)
    {
      iter->second->ApplyExtSubscription(host_name, process_id, topic_id, topic_type, topic_desc, reader_par);
    }
  }

  void CPubGate::RefreshRegistrations()
  {
    if (!m_created) return;

    // refresh publisher registrations
    std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datawriter_sync);
    for (auto iter : m_topic_name_datawriter_map)
    {
      iter.second->RefreshRegistration();
    }
  }
};
