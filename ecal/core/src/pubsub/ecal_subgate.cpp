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

#include <ecal/ecal.h>

#ifdef ECAL_OS_LINUX
//#include <sys/types.h>
//#include <sys/socket.h>
#endif

#include "ecal_def.h"
#include "ecal_descgate.h"

#include "pubsub/ecal_subgate.h"

////////////////////////////////////////////////////////
// local events
////////////////////////////////////////////////////////
namespace eCAL
{
  ECAL_API const EventHandleT& ShutdownProcEvent()
  {
    static EventHandleT evt;
    static std::string event_name(EVENT_SHUTDOWN_PROC + std::string("_") + std::to_string(Process::GetProcessID()));
    if (!gEventIsValid(evt))
    {
      gOpenEvent(&evt, event_name);
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
    std::unique_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    for (auto iter = m_topic_name_datareader_map.begin(); iter != m_topic_name_datareader_map.end(); ++iter)
    {
      iter->second->Destroy();
    }

    m_created = false;
  }

  bool CSubGate::Register(const std::string& topic_name_, CDataReader* datareader_)
  {
    if(!m_created) return(false);

    // register reader
    std::unique_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    m_topic_name_datareader_map.emplace(std::pair<std::string, CDataReader*>(topic_name_, datareader_));

    return(true);
  }

  bool CSubGate::Unregister(const std::string& topic_name_, CDataReader* datareader_)
  {
    if(!m_created) return(false);
    bool ret_state = false;

    std::unique_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    auto res = m_topic_name_datareader_map.equal_range(topic_name_);
    for(TopicNameDataReaderMapT::iterator iter = res.first; iter != res.second; ++iter)
    {
      if(iter->second == datareader_)
      {
        m_topic_name_datareader_map.erase(iter);
        break;
      }
    }

    return(ret_state);
  }

  bool CSubGate::HasSample(const std::string& sample_name_)
  {
    std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    return(m_topic_name_datareader_map.find(sample_name_) != m_topic_name_datareader_map.end());
  }

  size_t CSubGate::ApplySample(const eCAL::pb::Sample& ecal_sample_, eCAL::pb::eTLayerType layer_)
  {
    if(!m_created) return 0;

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
      auto& ecal_sample_content         = ecal_sample_.content();
      auto& ecal_sample_content_payload = ecal_sample_content.payload();
      g_process_rbytes_sum += ecal_sample_.content().payload().size();

      // apply sample to data reader
      std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
      auto res = m_topic_name_datareader_map.equal_range(ecal_sample_.topic().tname());
      for (auto it = res.first; it != res.second; ++it)
      {
        sent = it->second->AddSample(
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

    return sent;
  }

  size_t CSubGate::ApplySample(const std::string& topic_name_, const std::string& topic_id_, const char* buf_, size_t len_, long long id_, long long clock_, long long time_, size_t hash_, eCAL::pb::eTLayerType layer_)
  {
    if(!m_created) return 0;

    // update globals
    g_process_rclock++;
    g_process_rbytes_sum += len_;

    // apply sample to data reader
    size_t sent(0);
    std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    auto res = m_topic_name_datareader_map.equal_range(topic_name_);
    for (auto it = res.first; it != res.second; ++it)
    {
      sent = it->second->AddSample(topic_id_, buf_, len_, id_, clock_, time_, hash_, layer_);
    }

    return sent;
  }

  void CSubGate::ApplyLocPubRegistration(const eCAL::pb::Sample& ecal_sample_)
  {
    if(!m_created) return;

    // check topic name
    auto ecal_sample_topic = ecal_sample_.topic();
    std::string topic_name = ecal_sample_topic.tname();
    if (topic_name.empty()) return;

    // store description
    if (g_descgate()) g_descgate()->ApplyDescription(topic_name, ecal_sample_topic.ttype(), ecal_sample_topic.tdesc());

    // get process id
    std::string process_id = std::to_string(ecal_sample_.topic().pid());

    // handle local publisher connection
    std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    auto res = m_topic_name_datareader_map.equal_range(topic_name);
    for (TopicNameDataReaderMapT::iterator iter = res.first; iter != res.second; ++iter)
    {
      // apply layer specific parameter
      for (auto tlayer : ecal_sample_topic.tlayer())
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
        // REMOVE ME IN VERSION 6
        // ----------------------------------------------------------------------
          // if 'tlayer.par_layer' was not used and
        // 'tlayer.par_shm' is set
        if (writer_par.empty() && !tlayer.par_shm().empty())
        {
          writer_par = "#PAR_SHM#" + tlayer.par_shm();
        }
        // ----------------------------------------------------------------------
        // REMOVE ME IN VERSION 6
        // ----------------------------------------------------------------------

        iter->second->ApplyLocLayerParameter(process_id, tlayer.type(), writer_par);
      }
      // inform for local publisher connection
      iter->second->ApplyLocPublication(process_id);
    }
  }

  void CSubGate::ApplyExtPubRegistration(const eCAL::pb::Sample& ecal_sample_)
  {
    if(!m_created) return;

    auto sample_topic = ecal_sample_.topic();
    std::string host_name  = sample_topic.hname();
    std::string topic_name = sample_topic.tname();

    // store description
    if (g_descgate()) g_descgate()->ApplyDescription(topic_name, sample_topic.ttype(), sample_topic.tdesc());

    // handle external publisher connection
    std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    auto res = m_topic_name_datareader_map.equal_range(topic_name);
    for (TopicNameDataReaderMapT::iterator iter = res.first; iter != res.second; ++iter)
    {
      // apply layer specific parameter
      for (auto tlayer : ecal_sample_.topic().tlayer())
      {
        // layer parameter as protobuf message
        // this parameter is not used at all currently
        // for external publisher registrations
        std::string writer_par = tlayer.par_layer().SerializeAsString();
        iter->second->ApplyExtLayerParameter(host_name, tlayer.type(), writer_par);
      }
      // inform for external publisher connection
      iter->second->ApplyExtPublication(host_name);
    }
  }

  void CSubGate::RefreshRegistrations()
  {
    if (!m_created) return;

    // refresh reader registrations
    std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    for (auto iter : m_topic_name_datareader_map)
    {
      iter.second->RefreshRegistration();
    }
  }

  int CSubGate::CheckTimeouts()
  {
    if (!m_created) return(0);

    // check subscriber timeouts
    std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_datareader_sync);
    for (auto iter = m_topic_name_datareader_map.begin(); iter != m_topic_name_datareader_map.end(); ++iter)
    {
      iter->second->CheckReceiveTimeout();
    }

    // signal shutdown if eCAL is not okay
    bool ecal_is_ok = (g_globals_ctx != nullptr) && !gWaitForEvent(ShutdownProcEvent(), 0);
    if (!ecal_is_ok)
    {
      g_shutdown = 1;
    }

    return(0);
  }
};
