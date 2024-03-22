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

namespace eCAL
{
  std::atomic<bool> CRegistrationProvider::m_created;

  CRegistrationProvider::CRegistrationProvider() :
                    m_use_registration_udp(false),
                    m_use_registration_shm(false)
  {
  }

  CRegistrationProvider::~CRegistrationProvider()
  {
    Destroy();
  }

  void CRegistrationProvider::Create()
  {
    if(m_created) return;

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

    // add process unregistration sample
    AddSample2SampleList(GetProcessUnregisterSample());

    if (m_use_registration_udp)
    {
      // send process unregistration sample over udp
      SendSampleList2UDP();

      // destroy udp registration sample sender
      m_reg_sample_snd.reset();
    }

#if ECAL_CORE_REGISTRATION_SHM
    if (m_use_registration_shm)
    {
      // broadcast process unregistration sample over shm
      SendSampleList2SHM();

      // destroy shm registration sample writer
      m_memfile_broadcast_writer.Unbind();
      m_memfile_broadcast.Destroy();
    }
#endif

    m_created = false;
  }

  bool CRegistrationProvider::ApplySample(const Registration::Sample& sample_, const bool force_)
  {
    if (!m_created) return(false);

    // forward all registration samples to outside "customer" (e.g. monitoring, descgate)
    {
      const std::lock_guard<std::mutex> lock(m_callback_custom_apply_sample_map_mtx);
      for (const auto& iter : m_callback_custom_apply_sample_map)
      {
        iter.second(sample_);
      }
    }

    // update sample list
    AddSample2SampleList(sample_);

    // if registration is forced
    if (force_)
    {
      // send single registration sample over udp
      SendSample2UDP(sample_);

#if ECAL_CORE_REGISTRATION_SHM
      // broadcast (updated) sample list over shm
      SendSampleList2SHM();
#endif
    }

    return(true);
  }

  void CRegistrationProvider::SetCustomApplySampleCallback(const std::string& customer_, const ApplySampleCallbackT& callback_)
  {
    const std::lock_guard<std::mutex> lock(m_callback_custom_apply_sample_map_mtx);
    m_callback_custom_apply_sample_map[customer_] = callback_;
  }

  void CRegistrationProvider::RemCustomApplySampleCallback(const std::string& customer_)
  {
    const std::lock_guard<std::mutex> lock(m_callback_custom_apply_sample_map_mtx);
    auto iter = m_callback_custom_apply_sample_map.find(customer_);
    if (iter != m_callback_custom_apply_sample_map.end())
    {
      m_callback_custom_apply_sample_map.erase(iter);
    }
  }

  void CRegistrationProvider::AddSample2SampleList(const Registration::Sample& sample_)
  {
    const std::lock_guard<std::mutex> lock(m_sample_list_mtx);
    m_sample_list.samples.push_back(sample_);
  }

  bool CRegistrationProvider::SendSample2UDP(const Registration::Sample& sample_)
  {
    if (!m_created) return(false);

    if (m_use_registration_udp && m_reg_sample_snd)
    {
      // lock sample buffer
      const std::lock_guard<std::mutex> lock(m_sample_buffer_mtx);

      // serialize single sample
      if (SerializeToBuffer(sample_, m_sample_buffer))
      {
        // send single sample over udp
        return m_reg_sample_snd->Send("reg_sample", m_sample_buffer) != 0;
      }
    }
    return(false);
  }

  bool CRegistrationProvider::SendSampleList2UDP()
  {
    if (!m_created) return(false);
    bool return_value{ true };

    // lock sample list
    const std::lock_guard<std::mutex> lock(m_sample_list_mtx);

    // send all (single) samples over udp
    if (m_use_registration_udp && m_reg_sample_snd)
    {
      for (const auto& sample : m_sample_list.samples)
      {
        return_value &= SendSample2UDP(sample);
      }
    }

    return return_value;
  }

#if ECAL_CORE_REGISTRATION_SHM
  bool CRegistrationProvider::SendSampleList2SHM()
  {
    if (!m_created) return(false);

    bool return_value{ true };

    // send sample list over shm
    if (m_use_registration_shm)
    {
      // lock sample list
      const std::lock_guard<std::mutex> lock(m_sample_list_mtx);

      // serialize whole sample list
      if (SerializeToBuffer(m_sample_list, m_sample_list_buffer))
      {
        if (!m_sample_list_buffer.empty())
        {
          // broadcast sample list over shm
          return_value &= m_memfile_broadcast_writer.Write(m_sample_list_buffer.data(), m_sample_list_buffer.size());
        }
      }
    }
    return return_value;
  }
#endif

  void CRegistrationProvider::ClearSampleList()
  {
    // lock sample list
    const std::lock_guard<std::mutex> lock(m_sample_list_mtx);
    // clear sample list
    m_sample_list.samples.clear();
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

    // send out sample list over udp
    SendSampleList2UDP();

#if ECAL_CORE_REGISTRATION_SHM
    // broadcast sample list over shm
    SendSampleList2SHM();
#endif

    // clear registration sample list
    ClearSampleList();

    // add process registration sample to internal sample list as first sample (for next registration loop)
    AddSample2SampleList(GetProcessRegisterSample());
  }

  Registration::Sample CRegistrationProvider::GetProcessRegisterSample()
  {
    Registration::Sample process_sample;
    process_sample.cmd_type                     = bct_reg_process;
    auto& process_sample_process                = process_sample.process;
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

    return process_sample;
  }

  Registration::Sample CRegistrationProvider::GetProcessUnregisterSample()
  {
    Registration::Sample process_sample;
    process_sample.cmd_type      = bct_unreg_process;
    auto& process_sample_process = process_sample.process;
    process_sample_process.hname = Process::GetHostName();
    process_sample_process.pid   = Process::GetProcessID();
    process_sample_process.pname = Process::GetProcessName();
    process_sample_process.uname = Process::GetUnitName();

    return process_sample;
  }
}
