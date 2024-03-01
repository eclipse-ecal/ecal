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
 * @brief  eCAL registration provider
 * 
 * All process internal publisher/subscriber, server/clients register here with all their attributes.
 * 
 * These information will be send cyclic (registration refresh) via UDP to external eCAL processes.
 * 
**/

#include <atomic>
#include <ecal/ecal_config.h>

#include "ecal_def.h"
#include "ecal_globals.h"
#include "ecal_registration_provider.h"

#include "io/udp/ecal_udp_configurations.h"
#include "io/udp/ecal_udp_sample_sender.h"

#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>

namespace
{
  // TODO: remove me with new CDescGate
  bool ApplyTopicDescription(const std::string& topic_name_, const eCAL::SDataTypeInformation& topic_info_, bool topic_is_a_publisher_)
  {
    if (eCAL::g_descgate() != nullptr)
    {
      // calculate the quality of the current info
      eCAL::CDescGate::QualityFlags quality = eCAL::CDescGate::QualityFlags::NO_QUALITY;
      if (!topic_info_.encoding.empty() || !topic_info_.name.empty())
        quality |= eCAL::CDescGate::QualityFlags::TYPE_AVAILABLE;
      if (!topic_info_.descriptor.empty())
        quality |= eCAL::CDescGate::QualityFlags::DESCRIPTION_AVAILABLE;
      if (topic_is_a_publisher_)
        quality |= eCAL::CDescGate::QualityFlags::INFO_COMES_FROM_PRODUCER;
      quality |= eCAL::CDescGate::QualityFlags::INFO_COMES_FROM_THIS_PROCESS;
      quality |= eCAL::CDescGate::QualityFlags::INFO_COMES_FROM_CORRECT_ENTITY;
      // update description
      return eCAL::g_descgate()->ApplyTopicDescription(topic_name_, topic_info_, quality);
    }
    return false;
  }

  // TODO: remove me with new CDescGate
  bool ApplyServiceDescription(const std::string& service_name_, const std::string& method_name_,
    const eCAL::SDataTypeInformation& request_type_information_,
    const eCAL::SDataTypeInformation& response_type_information_)
  {
    if (eCAL::g_descgate() != nullptr)
    {
      // Calculate the quality of the current info
      eCAL::CDescGate::QualityFlags quality = eCAL::CDescGate::QualityFlags::NO_QUALITY;
      if (!(request_type_information_.name.empty() && response_type_information_.name.empty()))
        quality |= eCAL::CDescGate::QualityFlags::TYPE_AVAILABLE;
      if (!(request_type_information_.descriptor.empty() && response_type_information_.descriptor.empty()))
        quality |= eCAL::CDescGate::QualityFlags::DESCRIPTION_AVAILABLE;
      quality |= eCAL::CDescGate::QualityFlags::INFO_COMES_FROM_THIS_PROCESS;

      return eCAL::g_descgate()->ApplyServiceDescription(service_name_, method_name_, request_type_information_, response_type_information_, quality);
    }
    return false;
  }
}

namespace eCAL
{
  std::atomic<bool> CRegistrationProvider::m_created;

  CRegistrationProvider::CRegistrationProvider() :
                    m_reg_topics(false),
                    m_reg_services(false),
                    m_reg_process(false),
                    m_use_registration_udp(false),
                    m_use_registration_shm(false)
  {
  }

  CRegistrationProvider::~CRegistrationProvider()
  {
    Destroy();
  }

  void CRegistrationProvider::Create(bool topics_, bool services_, bool process_)
  {
    if(m_created) return;

    m_reg_topics   = topics_;
    m_reg_services = services_;
    m_reg_process  = process_;

    // send registration to shared memory and to udp
    m_use_registration_udp = !Config::Experimental::IsNetworkMonitoringDisabled();
    m_use_registration_shm = Config::Experimental::IsShmMonitoringEnabled();

    if (m_use_registration_udp)
    {
      // set network attributes
      IO::UDP::SSenderAttr attr;
      attr.address   = UDP::GetRegistrationAddress();
      attr.port      = UDP::GetRegistrationPort();
      attr.ttl       = UDP::GetMulticastTtl();
      attr.broadcast = UDP::IsBroadcast();
      attr.loopback  = true;
      attr.sndbuf    = Config::GetUdpMulticastSndBufSizeBytes();

      // create udp registration sender
      m_reg_sample_snd = std::make_shared<UDP::CSampleSender>(attr);
    }

#if ECAL_CORE_REGISTRATION_SHM
    if (m_use_registration_shm)
    {
      std::cout << "Shared memory monitoring is enabled (domain: " << Config::Experimental::GetShmMonitoringDomain() << " - queue size: " << Config::Experimental::GetShmMonitoringQueueSize() << ")" << '\n';
      m_memfile_broadcast.Create(Config::Experimental::GetShmMonitoringDomain(), Config::Experimental::GetShmMonitoringQueueSize());
      m_memfile_broadcast_writer.Bind(&m_memfile_broadcast);
    }
#endif

    // start cyclic registration thread
    m_reg_sample_snd_thread = std::make_shared<CCallbackThread>(std::bind(&CRegistrationProvider::RegisterSendThread, this));
    m_reg_sample_snd_thread->start(std::chrono::milliseconds(Config::GetRegistrationRefreshMs()));

    m_created = true;
  }

  void CRegistrationProvider::Destroy()
  {
    if(!m_created) return;

    // stop cyclic registration thread
    m_reg_sample_snd_thread->stop();

    // send one last (un)registration message to the world
    // thank you and goodbye :-)
    UnregisterProcess();

    // destroy registration sample sender
    m_reg_sample_snd.reset();

#if ECAL_CORE_REGISTRATION_SHM
    if (m_use_registration_shm)
    {
      m_memfile_broadcast_writer.Unbind();
      m_memfile_broadcast.Destroy();
    }
#endif

    m_created = false;
  }

  bool CRegistrationProvider::RegisterTopic(const std::string& topic_name_, const std::string& topic_id_, const Registration::Sample& ecal_sample_, const bool force_)
  {
    if (!m_created)    return(false);
    if (!m_reg_topics) return(false);

    const std::lock_guard<std::mutex> lock(m_topics_map_sync);
    m_topics_map[topic_name_ + topic_id_] = ecal_sample_;
    if(force_)
    {
      RegisterProcess();
      // apply registration sample
      ApplySample(topic_name_, ecal_sample_);
      // apply registration sample to shm registration
      SendSampleList(false);
    }

    return(true);
  }

  bool CRegistrationProvider::UnregisterTopic(const std::string& topic_name_, const std::string& topic_id_, const Registration::Sample& ecal_sample_, const bool force_)
  {
    if(!m_created) return(false);

    if (force_)
    {
      // apply unregistration sample
      ApplySample(topic_name_, ecal_sample_);
      // apply registration sample to shm registration
      SendSampleList(false);
    }

    SampleMapT::iterator iter;
    const std::lock_guard<std::mutex> lock(m_topics_map_sync);
    iter = m_topics_map.find(topic_name_ + topic_id_);
    if(iter != m_topics_map.end())
    {
      m_topics_map.erase(iter);
      return(true);
    }

    return(false);
  }

  bool CRegistrationProvider::RegisterServer(const std::string& service_name_, const std::string& service_id_, const Registration::Sample& ecal_sample_, bool force_)
  {
    if (!m_created)      return(false);
    if (!m_reg_services) return(false);

    const std::lock_guard<std::mutex> lock(m_server_map_sync);
    m_server_map[service_name_ + service_id_] = ecal_sample_;
    if (force_)
    {
      RegisterProcess();
      // apply registration sample
      ApplySample(service_name_, ecal_sample_);
      // apply registration sample to shm registration
      SendSampleList(false);
    }

    return(true);
  }

  bool CRegistrationProvider::UnregisterServer(const std::string& service_name_, const std::string& service_id_, const Registration::Sample& ecal_sample_, bool force_)
  {
    if (!m_created) return(false);

    if (force_)
    {
      // apply unregistration sample
      ApplySample(service_name_, ecal_sample_);
      // apply registration sample to shm registration
      SendSampleList(false);
    }

    SampleMapT::iterator iter;
    const std::lock_guard<std::mutex> lock(m_server_map_sync);
    iter = m_server_map.find(service_name_ + service_id_);
    if (iter != m_server_map.end())
    {
      m_server_map.erase(iter);
      return(true);
    }

    return(false);
  }

  bool CRegistrationProvider::RegisterClient(const std::string& client_name_, const std::string& client_id_, const Registration::Sample& ecal_sample_, bool force_)
  {
    if (!m_created)      return(false);
    if (!m_reg_services) return(false);

    const std::lock_guard<std::mutex> lock(m_client_map_sync);
    m_client_map[client_name_ + client_id_] = ecal_sample_;
    if (force_)
    {
      RegisterProcess();
      // apply registration sample
      ApplySample(client_name_, ecal_sample_);
      // apply registration sample to shm registration
      SendSampleList(false);
    }

    return(true);
  }

  bool CRegistrationProvider::UnregisterClient(const std::string& client_name_, const std::string& client_id_, const Registration::Sample& ecal_sample_, bool force_)
  {
    if (!m_created) return(false);

    if (force_)
    {
      // apply unregistration sample
      ApplySample(client_name_, ecal_sample_);
      // apply registration sample to shm registration
      SendSampleList(false);
    }

    SampleMapT::iterator iter;
    const std::lock_guard<std::mutex> lock(m_client_map_sync);
    iter = m_client_map.find(client_name_ + client_id_);
    if (iter != m_client_map.end())
    {
      m_client_map.erase(iter);
      return(true);
    }

    return(false);
  }

  bool CRegistrationProvider::RegisterProcess()
  {
    if (!m_created)     return(false);
    if (!m_reg_process) return(false);

    Registration::Sample process_sample;
    process_sample.cmd_type = bct_reg_process;
    auto& process_sample_process = process_sample.process;
    process_sample_process.hname                = Process::GetHostName();
    process_sample_process.hgname               = Process::GetHostGroupName();
    process_sample_process.pid                  = Process::GetProcessID();
    process_sample_process.pname                = Process::GetProcessName();
    process_sample_process.uname                = Process::GetUnitName();
    process_sample_process.pparam               = Process::GetProcessParameter();
    process_sample_process.state.severity       = static_cast<Registration::eProcessSeverity>(g_process_severity);
    process_sample_process.state.severity_level = static_cast<Registration::eProcessSeverityLevel>(g_process_severity_level);
    process_sample_process.state.info           = g_process_info;
#if ECAL_CORE_TIMEPLUGIN
    if (g_timegate() == nullptr)
    {
      process_sample_process.tsync_state = Registration::eTSyncState::tsync_none;
    }
    else
    {
      if (!g_timegate()->IsSynchronized())
      {
        process_sample_process.tsync_state = Registration::eTSyncState::tsync_none;
      }
      else
      {
        switch (g_timegate()->GetSyncMode())
        {
        case CTimeGate::eTimeSyncMode::realtime:
          process_sample_process.tsync_state = Registration::eTSyncState::tsync_realtime;
          break;
        case CTimeGate::eTimeSyncMode::replay:
          process_sample_process.tsync_state = Registration::eTSyncState::tsync_replay;
          break;
        default:
          process_sample_process.tsync_state = Registration::eTSyncState::tsync_none;
          break;
        }
      }
      process_sample_process.tsync_mod_name = g_timegate()->GetName();
    }
#endif

    // eCAL initialization state
    const unsigned int comp_state(g_globals()->GetComponents());
    process_sample_process.component_init_state = static_cast<int32_t>(comp_state);
    std::string component_info;
    if ((comp_state & Init::Publisher)  != 0u) component_info += "|pub";
    if ((comp_state & Init::Subscriber) != 0u) component_info += "|sub";
    if ((comp_state & Init::Logging)    != 0u) component_info += "|log";
    if ((comp_state & Init::TimeSync)   != 0u) component_info += "|time";
    if (!component_info.empty()) component_info = component_info.substr(1);
    process_sample_process.component_init_info = component_info;

    process_sample_process.ecal_runtime_version = GetVersionString();

    // apply registration sample
    const bool return_value = ApplySample(Process::GetHostName(), process_sample);

    return return_value;
  }

  bool CRegistrationProvider::UnregisterProcess()
  {
	  if (!m_created)     return(false);
	  if (!m_reg_process) return(false);

	  Registration::Sample process_sample;
	  process_sample.cmd_type = bct_unreg_process;
	  auto& process_sample_process = process_sample.process;
	  process_sample_process.hname = Process::GetHostName();
	  process_sample_process.pid   = Process::GetProcessID();
	  process_sample_process.pname = Process::GetProcessName();
	  process_sample_process.uname = Process::GetUnitName();

    // apply unregistration sample
    const bool return_value = ApplySample(Process::GetHostName(), process_sample);

	  return return_value;
  }

  bool CRegistrationProvider::RegisterTopics()
  {
    if (!m_created)    return(false);
    if (!m_reg_topics) return(false);

    bool return_value {true};
    const std::lock_guard<std::mutex> lock(m_topics_map_sync);
    for(SampleMapT::const_iterator iter = m_topics_map.begin(); iter != m_topics_map.end(); ++iter)
    {
      //////////////////////////////////////////////
      // update description
      //////////////////////////////////////////////
      // read attributes
      const std::string topic_name(iter->second.topic.tname);
      const bool topic_is_a_publisher(iter->second.cmd_type == eCAL::bct_reg_publisher);

      SDataTypeInformation topic_info;
      const auto& topic_datatype = iter->second.topic.tdatatype;
      topic_info.encoding   = topic_datatype.encoding;
      topic_info.name       = topic_datatype.name;
      topic_info.descriptor = topic_datatype.descriptor;

      ApplyTopicDescription(topic_name, topic_info, topic_is_a_publisher);

      //////////////////////////////////////////////
      // send sample to registration layer
      //////////////////////////////////////////////
      return_value &= ApplySample(iter->second.topic.tname, iter->second);
    }

    return return_value;
  }

  bool CRegistrationProvider::RegisterServer()
  {
    if (!m_created)      return(false);
    if (!m_reg_services) return(false);

    bool return_value {true};
    const std::lock_guard<std::mutex> lock(m_server_map_sync);
    for (SampleMapT::const_iterator iter = m_server_map.begin(); iter != m_server_map.end(); ++iter)
    {
      //////////////////////////////////////////////
      // update description
      //////////////////////////////////////////////
      const auto& ecal_sample_service = iter->second.service;
      for (const auto& method : ecal_sample_service.methods)
      {
        SDataTypeInformation request_type;
        request_type.name        = method.req_type;
        request_type.descriptor  = method.req_desc;

        SDataTypeInformation response_type;
        response_type.name       = method.resp_type;
        response_type.descriptor = method.resp_desc;

        ApplyServiceDescription(ecal_sample_service.sname, method.mname, request_type, response_type);
      }

      //////////////////////////////////////////////
      // send sample to registration layer
      //////////////////////////////////////////////
      return_value &= ApplySample(iter->second.service.sname, iter->second);
    }

    return return_value;
  }

  bool CRegistrationProvider::RegisterClient()
  {
    if (!m_created)      return(false);
    if (!m_reg_services) return(false);

    bool return_value {true};
    const std::lock_guard<std::mutex> lock(m_client_map_sync);
    for (SampleMapT::const_iterator iter = m_client_map.begin(); iter != m_client_map.end(); ++iter)
    {
      // apply registration sample
      return_value &= ApplySample(iter->second.client.sname, iter->second);
    }

    return return_value;
  }

  bool CRegistrationProvider::ApplySample(const std::string& sample_name_, const Registration::Sample& sample_)
  {
    if(!m_created) return(false);

    bool return_value {true};

    if (m_use_registration_udp && m_reg_sample_snd)
    {
      const std::lock_guard<std::mutex> lock(m_sample_buffer_sync);
      if (SerializeToBuffer(sample_, m_sample_buffer))
      {
        return_value &= (m_reg_sample_snd->Send(sample_name_, m_sample_buffer) != 0);
      }
    }

#if ECAL_CORE_REGISTRATION_SHM
    if (m_use_registration_shm)
    {
      const std::lock_guard<std::mutex> lock(m_sample_list_sync);
      m_sample_list.samples.push_back(sample_);
    }
#endif

    return return_value;
  }

  bool CRegistrationProvider::SendSampleList(bool reset_sample_list_)
  {
    if (!m_created) return(false);
    bool return_value{ true };

#if ECAL_CORE_REGISTRATION_SHM
    if (m_use_registration_shm)
    {
      {
        const std::lock_guard<std::mutex> lock(m_sample_list_sync);
        if (SerializeToBuffer(m_sample_list, m_sample_list_buffer))
        {
          if (reset_sample_list_)
          {
            m_sample_list.samples.clear();
          }
        }
      }

      if (!m_sample_list_buffer.empty())
      {
        return_value &= m_memfile_broadcast_writer.Write(m_sample_list_buffer.data(), m_sample_list_buffer.size());
      }
    }
#endif

    return return_value;
  }

  void CRegistrationProvider::RegisterSendThread()
  {
#if ECAL_CORE_SUBSCRIBER
    // refresh subscriber registration
    if (g_subgate() != nullptr) g_subgate()->RefreshRegistrations();
#endif

#if ECAL_CORE_PUBLISHER
    // refresh publisher registration
    if (g_pubgate() != nullptr) g_pubgate()->RefreshRegistrations();
#endif

#if ECAL_CORE_SERVICE
    // refresh server registration
    if (g_servicegate() != nullptr) g_servicegate()->RefreshRegistrations();

    // refresh client registration
    if (g_clientgate() != nullptr) g_clientgate()->RefreshRegistrations();
#endif

    // register process
    RegisterProcess();

#if ECAL_CORE_SERVICE
    // register server
    RegisterServer();

    // register clients
    RegisterClient();
#endif

    // register topics
    RegisterTopics();

    // write sample list to shared memory
    SendSampleList();
  }
}
