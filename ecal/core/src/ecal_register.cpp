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
 * @brief  eCAL common register class
**/

#include <ecal/ecal.h>

#include "ecal_def.h"
#include "ecal_config_hlp.h"
#include "ecal_globals.h"
#include "ecal_register.h"
#include "ecal_timegate.h"
#include "pubsub/ecal_pubgate.h"
#include "pubsub/ecal_subgate.h"
#include "service/ecal_servicegate.h"
#include "service/ecal_clientgate.h"

#include <sstream>
#include <iostream>
#include <chrono>
#include <thread>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include "ecal/pb/ecal.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace eCAL
{
  extern eCAL_Process_eSeverity  g_process_severity;
  extern std::string             g_process_info;

  extern std::atomic<long long>  g_process_wbytes;
  extern std::atomic<long long>  g_process_wbytes_sum;

  extern std::atomic<long long>  g_process_rbytes;
  extern std::atomic<long long>  g_process_rbytes_sum;

  std::atomic<bool> CEntityRegister::m_created;

  CEntityRegister::CEntityRegister() :
                    m_multicast_group(NET_UDP_MULTICAST_GROUP),
                    m_reg_refresh(CMN_REGISTRATION_REFRESH),
                    m_reg_topics(false),
                    m_reg_services(false),
                    m_reg_process(false)
  {
  };

  CEntityRegister::~CEntityRegister()
  {
    Destroy();
  }

  void CEntityRegister::Create(bool topics_, bool services_, bool process_)
  {
    if(m_created) return;

    m_multicast_group = eCALPAR(NET, UDP_MULTICAST_GROUP);
    m_reg_refresh     = eCALPAR(CMN, REGISTRATION_REFRESH);

    m_reg_topics      = topics_;
    m_reg_services    = services_;
    m_reg_process     = process_;

    SSenderAttr attr;
    bool local_only = !eCALPAR(NET, ENABLED);
    // for local only communication we switch to local broadcasting to bypass vpn's or firewalls
    if (local_only)
    {
      attr.ipaddr    = "127.255.255.255";
      attr.broadcast = true;
    }
    else
    {
      attr.ipaddr    = eCALPAR(NET, UDP_MULTICAST_GROUP);
      attr.broadcast = false;
    }
    attr.port     = eCALPAR(NET, UDP_MULTICAST_PORT) + NET_UDP_MULTICAST_PORT_REG_OFF;
    attr.loopback = true;
    attr.ttl      = eCALPAR(NET, UDP_MULTICAST_TTL);
    attr.sndbuf   = eCALPAR(NET, UDP_MULTICAST_SNDBUF);

    m_multicast_group = attr.ipaddr;

    m_reg_snd.Create(attr);
    m_reg_snd_thread.Start(eCALPAR(CMN, REGISTRATION_REFRESH), std::bind(&CEntityRegister::RegisterSendThread, this));

    m_created = true;
  }

  void CEntityRegister::Destroy()
  {
    if(!m_created) return;

    m_reg_snd_thread.Stop();

    m_created = false;
  }

  bool CEntityRegister::RegisterTopic(const std::string& topic_name_, const std::string& topic_id_, const eCAL::pb::Sample& ecal_sample_, const bool force_)
  {
    if(!m_created)    return(false);
    if(!m_reg_topics) return (false);

    std::lock_guard<std::mutex> lock(m_topics_map_sync);
    m_topics_map[topic_name_ + topic_id_] = ecal_sample_;
    if(force_)
    {
      RegisterProcess();
      RegisterSample(topic_name_, ecal_sample_);
    }

    return(true);
  }

  bool CEntityRegister::UnregisterTopic(const std::string& topic_name_, const std::string& topic_id_)
  {
    if(!m_created) return(false);

    SampleMapT::iterator iter;
    std::lock_guard<std::mutex> lock(m_topics_map_sync);
    iter = m_topics_map.find(topic_name_ + topic_id_);
    if(iter != m_topics_map.end())
    {
      m_topics_map.erase(iter);
      return(true);
    }

    return(false);
  }

  bool CEntityRegister::RegisterServer(const std::string& service_name_, const std::string& service_id_, const eCAL::pb::Sample& ecal_sample_, const bool force_)
  {
    if(!m_created)      return(false);
    if(!m_reg_services) return(false);

    std::lock_guard<std::mutex> lock(m_server_map_sync);
    m_server_map[service_name_ + service_id_] = ecal_sample_;
    if(force_)
    {
      RegisterProcess();
      RegisterSample(service_name_, ecal_sample_);
    }

    return(true);
  }

  bool CEntityRegister::UnregisterServer(const std::string& service_name_, const std::string& service_id_)
  {
    if(!m_created) return(false);

    SampleMapT::iterator iter;
    std::lock_guard<std::mutex> lock(m_server_map_sync);
    iter = m_server_map.find(service_name_ + service_id_);
    if(iter != m_server_map.end())
    {
      m_server_map.erase(iter);
      return(true);
    }

    return(false);
  }

  bool CEntityRegister::RegisterClient(const std::string& client_name_, const std::string& client_id_, const eCAL::pb::Sample& ecal_sample_, const bool force_)
  {
    if (!m_created)      return(false);
    if (!m_reg_services) return(false);

    std::lock_guard<std::mutex> lock(m_client_map_sync);
    m_client_map[client_name_ + client_id_] = ecal_sample_;
    if (force_)
    {
      RegisterProcess();
      RegisterSample(client_name_, ecal_sample_);
    }

    return(true);
  }

  bool CEntityRegister::UnregisterClient(const std::string& client_name_, const std::string& client_id_)
  {
    if (!m_created) return(false);

    SampleMapT::iterator iter;
    std::lock_guard<std::mutex> lock(m_client_map_sync);
    iter = m_client_map.find(client_name_ + client_id_);
    if (iter != m_client_map.end())
    {
      m_client_map.erase(iter);
      return(true);
    }

    return(false);
  }

  size_t CEntityRegister::RegisterProcess()
  {
    if(!m_created)     return(0);
    if(!m_reg_process) return(0);

    eCAL::pb::Sample process_sample;
    process_sample.set_cmd_type(eCAL::pb::bct_reg_process);
    auto process_sample_mutable_process = process_sample.mutable_process();
    process_sample_mutable_process->set_hname(Process::GetHostName());
    process_sample_mutable_process->set_pid(Process::GetProcessID());
    process_sample_mutable_process->set_pname(Process::GetProcessName());
    process_sample_mutable_process->set_uname(Process::GetUnitName());
    process_sample_mutable_process->set_pparam(Process::GetProcessParameter());
    process_sample_mutable_process->set_pmemory(Process::GetProcessMemory());
    process_sample_mutable_process->set_pcpu(Process::GetProcessCpuUsage());
    process_sample_mutable_process->set_usrptime(static_cast<float>(Logging::GetCoreTime()));
    process_sample_mutable_process->set_datawrite(google::protobuf::int64(Process::GetWBytes()));
    process_sample_mutable_process->set_dataread(google::protobuf::int64(Process::GetRBytes()));
    process_sample_mutable_process->mutable_state()->set_severity(eCAL::pb::eProcessSeverity(g_process_severity));
    process_sample_mutable_process->mutable_state()->set_info(g_process_info);
    if (!g_timegate())
    {
      process_sample_mutable_process->set_tsync_state(eCAL::pb::eTSyncState::tsync_none);
    }
    else
    {
      if (!g_timegate()->IsSynchronized())
      {
        process_sample_mutable_process->set_tsync_state(eCAL::pb::eTSyncState::tsync_none);
      }
      else
      {
        switch (g_timegate()->GetSyncMode())
        {
        case CTimeGate::eTimeSyncMode::realtime:
          process_sample_mutable_process->set_tsync_state(eCAL::pb::eTSyncState::tsync_realtime);
          break;
        case CTimeGate::eTimeSyncMode::replay:
          process_sample_mutable_process->set_tsync_state(eCAL::pb::eTSyncState::tsync_replay);
          break;
        default:
          process_sample_mutable_process->set_tsync_state(eCAL::pb::eTSyncState::tsync_none);
          break;
        }
      }
      process_sample_mutable_process->set_tsync_mod_name(g_timegate()->GetName());
    }

    // eCAL initialization state
    unsigned int comp_state(g_globals()->GetComponents());
    process_sample_mutable_process->set_component_init_state(google::protobuf::int32(comp_state));
    std::string component_info;
    if (comp_state & Init::Publisher)   component_info += "|pub";
    if (comp_state & Init::Subscriber)  component_info += "|sub";
    if (comp_state & Init::Service)     component_info += "|srv";
    if (comp_state & Init::Monitoring)  component_info += "|mon";
    if (comp_state & Init::Logging)     component_info += "|log";
    if (comp_state & Init::TimeSync)    component_info += "|time";
    if (!component_info.empty()) component_info = component_info.substr(1);
    process_sample_mutable_process->set_component_init_info(component_info);

    process_sample_mutable_process->set_ecal_runtime_version(eCAL::GetVersionString());

    // register sample
    size_t sent_sum = RegisterSample(Process::GetHostName(), process_sample);

    return(sent_sum);
  }

  size_t CEntityRegister::RegisterServer()
  {
    if(!m_created)      return(0);
    if(!m_reg_services) return(0);

    size_t sent_sum(0);
    std::lock_guard<std::mutex> lock(m_server_map_sync);
    for(SampleMapT::const_iterator iter = m_server_map.begin(); iter != m_server_map.end(); ++iter)
    {
      // register sample
      sent_sum += RegisterSample(iter->second.service().sname(), iter->second);
    }

    return(sent_sum);
  }

  size_t CEntityRegister::RegisterClient()
  {
    if (!m_created)      return(0);
    if (!m_reg_services) return(0);

    size_t sent_sum(0);
    std::lock_guard<std::mutex> lock(m_client_map_sync);
    for (SampleMapT::const_iterator iter = m_client_map.begin(); iter != m_client_map.end(); ++iter)
    {
      // register sample
      sent_sum += RegisterSample(iter->second.client().sname(), iter->second);
    }

    return(sent_sum);
  }

  size_t CEntityRegister::RegisterTopics()
  {
    if(!m_created)    return(0);
    if(!m_reg_topics) return(0);

    size_t sent_sum(0);
    std::lock_guard<std::mutex> lock(m_topics_map_sync);
    for(SampleMapT::const_iterator iter = m_topics_map.begin(); iter != m_topics_map.end(); ++iter)
    {
      sent_sum += RegisterSample(iter->second.topic().tname(), iter->second);
    }

    return(sent_sum);
  }

  size_t CEntityRegister::RegisterSample(const std::string& sample_name_, const eCAL::pb::Sample& sample_)
  {
    if(!m_created) return(0);

    // send sample
    size_t sent_size = SendSample(&m_reg_snd, sample_name_, sample_, m_multicast_group, -1);

    return(sent_size);
  }

  int CEntityRegister::RegisterSendThread()
  {
    if(!m_created) return(0);

    // calculate average receive bytes
    g_process_rbytes = static_cast<long long>(((double)g_process_rbytes_sum / m_reg_refresh)*1000.0);
    g_process_rbytes_sum = 0;

    // calculate average write bytes
    g_process_wbytes = static_cast<long long>(((double)g_process_wbytes_sum / m_reg_refresh)*1000.0);
    g_process_wbytes_sum = 0;

    // refresh subscriber registration
    if (g_subgate()) g_subgate()->RefreshRegistrations();

    // refresh publisher registration
    if (g_pubgate()) g_pubgate()->RefreshRegistrations();

    // refresh server registration
    if (g_servicegate()) g_servicegate()->RefreshRegistrations();

    // refresh client registration
    if (g_clientgate()) g_clientgate()->RefreshRegistrations();

    // overall registration udp send size for debugging
    /*size_t sent_sum(0);*/

    // register process
    /*sent_sum += */RegisterProcess();

    // register server
    /*sent_sum += */RegisterServer();

    // register clients
    /*sent_sum += */RegisterClient();

    // register topics
    /*sent_sum += */RegisterTopics();

    return(0);
  };
};
