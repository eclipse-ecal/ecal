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

#include <algorithm>

#include <ecal/ecal.h>

#ifdef ECAL_OS_LINUX
//#include <sys/types.h>
//#include <sys/socket.h>
#endif

#include "ecal_def.h"
#include "ecal_event_internal.h"
#include "ecal_descgate.h"

#include "pubsub/ecal_subgate.h"
#include "ecal_sample_to_topicinfo.h"

////////////////////////////////////////////////////////
// local events
////////////////////////////////////////////////////////
namespace eCAL
{
  ECAL_API const EventHandleT& ShutdownProcEvent()
  {
    static EventHandleT evt;
    static const std::string event_name(EVENT_SHUTDOWN_PROC + std::string("_") + std::to_string(Process::GetProcessID()));
    if (!gEventIsValid(evt))
    {
      gOpenNamedEvent(&evt, event_name, true);
    }
    return(evt);
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

    // start timeout thread
    m_subtimeout_thread.Start(CMN_DATAREADER_TIMEOUT_DTIME, std::bind(&CSubGate::CheckTimeouts, this));
    m_created = true;
  }

  void CSubGate::Destroy()
  {
    if(!m_created) return;

    // stop timeout thread
    m_subtimeout_thread.Stop();

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
    for(TopicNameDataReaderMapT::iterator iter = res.first; iter != res.second; ++iter)
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

  bool CSubGate::ApplySample(const eCAL::pb::Sample& ecal_sample_, eCAL::pb::eTLayerType layer_)
  {
    if(!m_created) return false;

    size_t sent(0);
    switch (ecal_sample_.cmd_type())
    {
    case eCAL::pb::bct_set_sample:
    {
#ifndef NDEBUG
      // check layer
      if (layer_ == eCAL::pb::eTLayerType::tl_none)
      {
        // log it
        eCAL::Logging::Log(log_level_error, ecal_sample_.topic().tname() + " : payload received without layer definition !");
      }
#endif

      // update globals
      g_process_rclock++;
      const auto& ecal_sample_content         = ecal_sample_.content();
      const auto& ecal_sample_content_payload = ecal_sample_content.payload();
      g_process_rbytes_sum += ecal_sample_.content().payload().size();

      std::vector<std::shared_ptr<CDataReader>> readers_to_apply;

      // Lock the sync map only while extracting the relevant shared pointers to the Datareaders.
      // Apply the samples to the readers afterwards.
      {
        // apply sample to data reader
        const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
        auto res = m_topic_name_datareader_map.equal_range(ecal_sample_.topic().tname());
        std::transform(
          res.first, res.second, std::back_inserter(readers_to_apply), [](const auto& match) { return match.second; }
        );
      }

      for (const auto& reader : readers_to_apply)
      {
        sent = reader->AddSample(
          ecal_sample_.topic().tid(),
          ecal_sample_content_payload.data(),
          ecal_sample_content_payload.size(),
          ecal_sample_content.id(),
          ecal_sample_content.clock(),
          ecal_sample_content.time(),
          static_cast<size_t>(ecal_sample_content.hash()),
          layer_
        );
      }
    }
    break;
    default:
      break;
    }

    return (sent > 0);
  }

  bool CSubGate::ApplySample(const std::string& topic_name_, const std::string& topic_id_, const char* buf_, size_t len_, long long id_, long long clock_, long long time_, size_t hash_, eCAL::pb::eTLayerType layer_)
  {
    if(!m_created) return false;

    // update globals
    g_process_rclock++;
    g_process_rbytes_sum += len_;

    // apply sample to data reader
    size_t sent(0);
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
      sent = reader->AddSample(topic_id_, buf_, len_, id_, clock_, time_, hash_, layer_);
    }

    return (sent > 0);
  }

  void CSubGate::ApplyLocPubRegistration(const eCAL::pb::Sample& ecal_sample_)
  {
    if(!m_created) return;

    // check topic name
    const auto& ecal_sample = ecal_sample_.topic();
    const std::string& topic_name = ecal_sample.tname();
    if (topic_name.empty()) return;

    const std::string& topic_id   = ecal_sample.tid();
    SDataTypeInformation topic_info{ eCALSampleToTopicInformation(ecal_sample_) };

    // store description
    ApplyTopicToDescGate(topic_name, topic_info);

    // get process id
    const std::string process_id = std::to_string(ecal_sample_.topic().pid());

    // handle local publisher connection
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    auto res = m_topic_name_datareader_map.equal_range(topic_name);
    for (TopicNameDataReaderMapT::iterator iter = res.first; iter != res.second; ++iter)
    {
      // apply layer specific parameter
      for (const auto& tlayer : ecal_sample.tlayer())
      {
        // layer parameter for local publisher registrations
        // ---------------------------------------------------------------
        // eCAL version > 5.8.13/5.9.0:
        //    new layer parameter 'tlayer.par_layer'
        //    protobuf message is serialized into reader parameter string
        // ---------------------------------------------------------------
        // eCAL version <= 5.8.13/5.9.0:
        //    old layer parameter 'tlayer.par_shm'
        //    contains memory file name for shared memory layer
        //    the memory file name will be prefixed by '#PAR_SHM#' for
        //    later check in ecal_reader_shm SetConnectionParameter()
        // ---------------------------------------------------------------

        // first check for new behavior
        std::string writer_par = tlayer.par_layer().SerializeAsString();

        // ----------------------------------------------------------------------
        // REMOVE ME IN ECAL6
        // ----------------------------------------------------------------------
          // if 'tlayer.par_layer' was not used and
        // 'tlayer.par_shm' is set
        if (writer_par.empty() && !tlayer.par_shm().empty())
        {
          writer_par = "#PAR_SHM#" + tlayer.par_shm();
        }
        // ----------------------------------------------------------------------
        // REMOVE ME IN ECAL6
        // ----------------------------------------------------------------------

        iter->second->ApplyLocLayerParameter(process_id, topic_id, tlayer.type(), writer_par);
      }
      // inform for local publisher connection
      iter->second->ApplyLocPublication(process_id, topic_id, topic_info);
    }
  }

  void CSubGate::ApplyLocPubUnregistration(const eCAL::pb::Sample& ecal_sample_)
  {
    if (!m_created) return;

    // check topic name
    const auto& ecal_sample = ecal_sample_.topic();
    const std::string& topic_name = ecal_sample.tname();
    const std::string& topic_id   = ecal_sample.tid();
    const std::string process_id  = std::to_string(ecal_sample_.topic().pid());

    // unregister local publisher
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    auto res = m_topic_name_datareader_map.equal_range(topic_name);
    for (TopicNameDataReaderMapT::iterator iter = res.first; iter != res.second; ++iter)
    {
      iter->second->RemoveLocPublication(process_id, topic_id);
    }
  }

  void CSubGate::ApplyExtPubRegistration(const eCAL::pb::Sample& ecal_sample_)
  {
    if(!m_created) return;

    const auto& ecal_sample = ecal_sample_.topic();
    const std::string& host_name  = ecal_sample.hname();
    const std::string& topic_name = ecal_sample.tname();
    const std::string& topic_id   = ecal_sample.tid();
    SDataTypeInformation topic_info{ eCALSampleToTopicInformation(ecal_sample_) };
    const std::string  process_id = std::to_string(ecal_sample.pid());

    // store description
    ApplyTopicToDescGate(topic_name, topic_info);

    // handle external publisher connection
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    auto res = m_topic_name_datareader_map.equal_range(topic_name);
    for (TopicNameDataReaderMapT::iterator iter = res.first; iter != res.second; ++iter)
    {
      // apply layer specific parameter
      for (const auto& tlayer : ecal_sample_.topic().tlayer())
      {
        // layer parameter as protobuf message
        const std::string writer_par = tlayer.par_layer().SerializeAsString();
        iter->second->ApplyExtLayerParameter(host_name, tlayer.type(), writer_par);
      }
      // inform for external publisher connection
      iter->second->ApplyExtPublication(host_name, process_id, topic_id, topic_info);
    }
  }

  void CSubGate::ApplyExtPubUnregistration(const eCAL::pb::Sample& ecal_sample_)
  {
    if (!m_created) return;

    const auto& ecal_sample = ecal_sample_.topic();
    const std::string& host_name  = ecal_sample.hname();
    const std::string& topic_name = ecal_sample.tname();
    const std::string& topic_id   = ecal_sample.tid();
    const std::string  process_id = std::to_string(ecal_sample.pid());

    // unregister local subscriber
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    auto res = m_topic_name_datareader_map.equal_range(topic_name);
    for (TopicNameDataReaderMapT::iterator iter = res.first; iter != res.second; ++iter)
    {
      iter->second->RemoveExtPublication(host_name, process_id, topic_id);
    }
  }

  void CSubGate::RefreshRegistrations()
  {
    if (!m_created) return;

    // refresh reader registrations
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    for (auto iter : m_topic_name_datareader_map)
    {
      iter.second->RefreshRegistration();
    }
  }

  int CSubGate::CheckTimeouts()
  {
    if (!m_created) return(0);

    // check subscriber timeouts
    const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    for (auto iter = m_topic_name_datareader_map.begin(); iter != m_topic_name_datareader_map.end(); ++iter)
    {
      iter->second->CheckReceiveTimeout();
    }

    // signal shutdown if eCAL is not okay
    const bool ecal_is_ok = (g_globals_ctx != nullptr) && !gWaitForEvent(ShutdownProcEvent(), 0);
    if (!ecal_is_ok)
    {
      g_shutdown = 1;
    }

    return(0);
  }

  bool CSubGate::ApplyTopicToDescGate(const std::string& topic_name_, const SDataTypeInformation& topic_info_)
  {
    if (g_descgate() != nullptr)
    {
      // Calculate the quality of the current info
      ::eCAL::CDescGate::QualityFlags quality = ::eCAL::CDescGate::QualityFlags::NO_QUALITY;
      if (!topic_info_.name.empty() || !topic_info_.encoding.empty())
        quality |= ::eCAL::CDescGate::QualityFlags::TYPE_AVAILABLE;
      if (!topic_info_.descriptor.empty())
        quality |= ::eCAL::CDescGate::QualityFlags::DESCRIPTION_AVAILABLE;
      quality |= ::eCAL::CDescGate::QualityFlags::INFO_COMES_FROM_CORRECT_ENTITY;
      quality |= ::eCAL::CDescGate::QualityFlags::INFO_COMES_FROM_PRODUCER;

      return g_descgate()->ApplyTopicDescription(topic_name_, topic_info_, quality);
    }
    return false;
  }
}
