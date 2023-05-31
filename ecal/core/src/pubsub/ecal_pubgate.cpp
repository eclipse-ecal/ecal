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

#include "ecal_config_reader_hlp.h"
#include "ecal_pubgate.h"
#include "ecal_descgate.h"
#include "ecal_sample_to_topicinfo.h"

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
  }

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
    const std::unique_lock<std::shared_timed_mutex> lock(m_topic_name_datawriter_sync);
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
    const std::unique_lock<std::shared_timed_mutex> lock(m_topic_name_datawriter_sync);
    m_topic_name_datawriter_map.emplace(std::pair<std::string, CDataWriter*>(topic_name_, datawriter_));

    return(true);
  }

  bool CPubGate::Unregister(const std::string& topic_name_, CDataWriter* datawriter_)
  {
    if(!m_created) return(false);
    bool ret_state = false;

    const std::unique_lock<std::shared_timed_mutex> lock(m_topic_name_datawriter_sync);
    auto res = m_topic_name_datawriter_map.equal_range(topic_name_);
    for(TopicNameDataWriterMapT::iterator iter = res.first; iter != res.second; ++iter)
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

  void CPubGate::ApplyLocSubRegistration(const eCAL::pb::Sample& ecal_sample_)
  {
    if(!m_created) return;

    const auto& ecal_sample = ecal_sample_.topic();
    const std::string& topic_name = ecal_sample.tname();
    const std::string& topic_id   = ecal_sample.tid();
    STopicInformation topic_information{ eCALSampleToTopicInformation(ecal_sample_) };
    const std::string  process_id = std::to_string(ecal_sample.pid());

    std::string reader_par;
    for (const auto& layer : ecal_sample.tlayer())
    {
      // layer parameter as protobuf message
      // this parameter is not used at all currently
      // for local subscriber registrations
      reader_par = layer.par_layer().SerializeAsString();
    }

    // store description
    ApplyTopicToDescGate(topic_name, topic_information);

    // register local subscriber
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datawriter_sync);
    auto res = m_topic_name_datawriter_map.equal_range(topic_name);
    for(TopicNameDataWriterMapT::const_iterator iter = res.first; iter != res.second; ++iter)
    {
      iter->second->ApplyLocSubscription(process_id, topic_id, topic_information, reader_par);
    }
  }

  void CPubGate::ApplyLocSubUnregistration(const eCAL::pb::Sample& ecal_sample_)
  {
    if (!m_created) return;

    const auto& ecal_sample = ecal_sample_.topic();
    const std::string& topic_name = ecal_sample.tname();
    const std::string& topic_id   = ecal_sample.tid();
    const std::string  process_id = std::to_string(ecal_sample.pid());

    // unregister local subscriber
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datawriter_sync);
    auto res = m_topic_name_datawriter_map.equal_range(topic_name);
    for (TopicNameDataWriterMapT::const_iterator iter = res.first; iter != res.second; ++iter)
    {
      iter->second->RemoveLocSubscription(process_id, topic_id);
    }
  }

  void CPubGate::ApplyExtSubRegistration(const eCAL::pb::Sample& ecal_sample_)
  {
    if(!m_created) return;

    const auto& ecal_sample = ecal_sample_.topic();
    const std::string& host_name  = ecal_sample.hname();
    const std::string& topic_name = ecal_sample.tname();
    const std::string& topic_id   = ecal_sample.tid();
    STopicInformation topic_information{ eCALSampleToTopicInformation(ecal_sample_) };
    const std::string  process_id = std::to_string(ecal_sample.pid());

    std::string reader_par;
    for (const auto& layer : ecal_sample.tlayer())
    {
      // layer parameter as protobuf message
      // this parameter is not used at all currently
      // for external subscriber registrations
      reader_par = layer.par_layer().SerializeAsString();
    }

    // store description
    ApplyTopicToDescGate(topic_name, topic_information);

    // register external subscriber
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datawriter_sync);
    auto res = m_topic_name_datawriter_map.equal_range(topic_name);
    for(TopicNameDataWriterMapT::const_iterator iter = res.first; iter != res.second; ++iter)
    {
      iter->second->ApplyExtSubscription(host_name, process_id, topic_id, topic_information, reader_par);
    }
  }

  void CPubGate::ApplyExtSubUnregistration(const eCAL::pb::Sample& ecal_sample_)
  {
    if (!m_created) return;

    const auto& ecal_sample = ecal_sample_.topic();
    const std::string& host_name  = ecal_sample.hname();
    const std::string& topic_name = ecal_sample.tname();
    const std::string& topic_id   = ecal_sample.tid();
    const std::string  process_id = std::to_string(ecal_sample.pid());

    // unregister external subscriber
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datawriter_sync);
    auto res = m_topic_name_datawriter_map.equal_range(topic_name);
    for (TopicNameDataWriterMapT::const_iterator iter = res.first; iter != res.second; ++iter)
    {
      iter->second->RemoveExtSubscription(host_name, process_id, topic_id);
    }
  }

  void CPubGate::RefreshRegistrations()
  {
    if (!m_created) return;

    // refresh publisher registrations
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datawriter_sync);
    for (auto iter : m_topic_name_datawriter_map)
    {
      iter.second->RefreshRegistration();
    }
  }

  bool CPubGate::ApplyTopicToDescGate(const std::string& topic_name_, const STopicInformation& topic_info_)
  {
    if (g_descgate() != nullptr)
    {
      // Calculate the quality of the current info
      ::eCAL::CDescGate::QualityFlags quality = ::eCAL::CDescGate::QualityFlags::NO_QUALITY;
      if (!topic_info_.type.empty() || !topic_info_.encoding.empty())
        quality |= ::eCAL::CDescGate::QualityFlags::TYPE_AVAILABLE;
      if (!topic_info_.descriptor.empty())
        quality |= ::eCAL::CDescGate::QualityFlags::DESCRIPTION_AVAILABLE;
      quality |= ::eCAL::CDescGate::QualityFlags::INFO_COMES_FROM_CORRECT_ENTITY;

      return g_descgate()->ApplyTopicDescription(topic_name_, topic_info_, quality);
    }
    return false;
  }
};
