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
 * @brief  eCAL registration receiver
 *
 * Receives registration information from external eCAL processes and forwards them to
 * the internal publisher/subscriber, server/clients.
 *
**/

#include "ecal_registration_receiver.h"

#include "pubsub/ecal_subgate.h"
#include "pubsub/ecal_pubgate.h"
#include "service/ecal_clientgate.h"
#include "service/ecal_servicegate.h"

namespace eCAL
{
  size_t CUdpRegistrationReceiver::ApplySample(const eCAL::pb::Sample& ecal_sample_, eCAL::pb::eTLayerType /*layer_*/)
  {
    if (!g_registration_receiver()) return 0;
    return g_registration_receiver()->ApplySample(ecal_sample_);
  };

#ifndef ECAL_LAYER_ICEORYX
  //////////////////////////////////////////////////////////////////
  // CMemfileRegistrationReceiver
  //////////////////////////////////////////////////////////////////

  bool CMemfileRegistrationReceiver::Create(eCAL::CMemoryFileBroadcastReader* memfile_broadcast_reader_)
  {
    if (m_created) return false;
    m_memfile_broadcast_reader = memfile_broadcast_reader_;
    m_created = true;
    return true;
  }

  bool CMemfileRegistrationReceiver::Receive()
  {
    if (!m_created) return false;

    MemfileBroadcastMessageListT message_list;
    if(!m_memfile_broadcast_reader->Read(message_list, 0))
      return false;

    eCAL::pb::SampleList sample_list;
    bool return_value {true};

    for(const auto& message: message_list)
    {
      if(sample_list.ParseFromArray(message.data, static_cast<int>(message.size)))
      {
        for(const auto& sample: sample_list.samples())
        {
          return_value &= ApplySample(sample);
        }
      }
      else
        return_value = false;
    }
    return return_value;
  }

  bool CMemfileRegistrationReceiver::Destroy()
  {
    if (!m_created) return false;
    m_memfile_broadcast_reader = nullptr;
    m_created = false;
    return true;
  }

  bool CMemfileRegistrationReceiver::ApplySample(const eCAL::pb::Sample& ecal_sample_)
  {
    if (!g_registration_receiver()) return 0;
    return (g_registration_receiver()->ApplySample(ecal_sample_) != 0);
  }
#endif

  //////////////////////////////////////////////////////////////////
  // CRegistrationReceiver
  //////////////////////////////////////////////////////////////////
  std::atomic<bool> CRegistrationReceiver::m_created;

  CRegistrationReceiver::CRegistrationReceiver() :
                         m_network(NET_ENABLED),
                         m_loopback(false),
                         m_callback_pub(nullptr),
                         m_callback_sub(nullptr),
                         m_callback_service(nullptr),
                         m_callback_client(nullptr),
                         m_callback_process(nullptr),
                         m_use_network_monitoring(false),
                         m_use_shm_monitoring(false),
                         m_callback_custom_apply_sample([](const auto&){})

  {
  };

  CRegistrationReceiver::~CRegistrationReceiver()
  {
    Destroy();
  }

  void CRegistrationReceiver::Create()
  {
    if(m_created) return;

    // network mode
    m_network = Config::IsNetworkEnabled();

    m_use_shm_monitoring = Config::Experimental::IsShmMonitoringEnabled();
    m_use_network_monitoring = !Config::Experimental::IsNetworkMonitoringDisabled();

    if (m_use_network_monitoring)
    {
      // start registration receive thread
      SReceiverAttr attr;
      bool local_only = !Config::IsNetworkEnabled();
      // for local only communication we switch to local broadcasting to bypass vpn's or firewalls
      if (local_only)
      {
        attr.ipaddr = "127.255.255.255";
        attr.broadcast = true;
      }
      else
      {
        attr.ipaddr = Config::GetUdpMulticastGroup();
        attr.broadcast = false;
      }
      attr.port = Config::GetUdpMulticastPort() + NET_UDP_MULTICAST_PORT_REG_OFF;
      attr.loopback = true;
      attr.rcvbuf = Config::GetUdpMulticastRcvBufSizeBytes();

      m_reg_rcv.Create(attr);
      m_reg_rcv_thread.Start(0, std::bind(&CUdpRegistrationReceiver::Receive, &m_reg_rcv_process, &m_reg_rcv));
    }

#ifndef ECAL_LAYER_ICEORYX
    if (m_use_shm_monitoring)
    {
      m_memfile_broadcast.Create(Config::Experimental::GetShmMonitoringDomain(), Config::Experimental::GetShmMonitoringQueueSize());
      m_memfile_broadcast.FlushLocalEventQueue();
      m_memfile_broadcast_reader.Bind(&m_memfile_broadcast);

      m_memfile_reg_rcv.Create(&m_memfile_broadcast_reader);
      m_memfile_reg_rcv_thread.Start(Config::GetRegistrationRefreshMs() / 2 , std::bind(&CMemfileRegistrationReceiver::Receive, &m_memfile_reg_rcv));
    }
#endif

    m_created = true;
  }

  void CRegistrationReceiver::Destroy()
  {
    if(!m_created) return;

    if(m_use_network_monitoring)
      // stop network registration receive thread
      m_reg_rcv_thread.Stop();

#ifndef ECAL_LAYER_ICEORYX
    if(m_use_shm_monitoring)
    {
      // stop memfile registration receive thread and unbind reader
      m_memfile_reg_rcv_thread.Stop();
      m_memfile_broadcast_reader.Unbind();
      m_memfile_broadcast.Destroy();
    }
#endif

    // reset callbacks
    m_callback_pub     = nullptr;
    m_callback_sub     = nullptr;
    m_callback_service = nullptr;
    m_callback_client  = nullptr;
    m_callback_process = nullptr;

    // finished
    m_created          = false;
  }

  void CRegistrationReceiver::EnableLoopback(bool state_)
  {
    m_loopback = state_;
  }

  size_t CRegistrationReceiver::ApplySample(const eCAL::pb::Sample& ecal_sample_)
  {
    if(!m_created) return 0;

    m_callback_custom_apply_sample(ecal_sample_);

    std::string reg_sample;
    if ( m_callback_pub
      || m_callback_sub
      || m_callback_service
      || m_callback_client
      || m_callback_process
      )
    {
      reg_sample = ecal_sample_.SerializeAsString();
    }

    switch(ecal_sample_.cmd_type())
    {
    case eCAL::pb::bct_none:
    case eCAL::pb::bct_set_sample:
      break;
    case eCAL::pb::bct_reg_process:
      if (m_callback_process) m_callback_process(reg_sample.c_str(), static_cast<int>(reg_sample.size()));
      break;
    case eCAL::pb::bct_reg_service:
      if (m_callback_service) m_callback_service(reg_sample.c_str(), static_cast<int>(reg_sample.size()));
      if (g_clientgate())  g_clientgate()->ApplyServiceRegistration(ecal_sample_);
      break;
    case eCAL::pb::bct_reg_client:
      if (m_callback_client) m_callback_client(reg_sample.c_str(), static_cast<int>(reg_sample.size()));
      if (g_servicegate()) g_servicegate()->ApplyClientRegistration(ecal_sample_);
      break;
    case eCAL::pb::bct_reg_subscriber:
      {
        // process local subscriber registrations
        if(IsLocalHost(ecal_sample_))
        {
          // do not register subscriber of the same process
          // only if loop back flag is set true
          if(m_loopback || (ecal_sample_.topic().pid() != Process::GetProcessID()))
          {
            if (g_pubgate()) g_pubgate()->ApplyLocSubRegistration(ecal_sample_);
          }
        }
        // process external subscriber registrations
        else
        {
          if(m_network)
          {
            if (g_pubgate()) g_pubgate()->ApplyExtSubRegistration(ecal_sample_);
          }
        }
        if (m_callback_sub) m_callback_sub(reg_sample.c_str(), static_cast<int>(reg_sample.size()));
      }
      break;
    case eCAL::pb::bct_reg_publisher:
      {
        // process local publisher registrations
        if(IsLocalHost(ecal_sample_))
        {
          // do not register publisher of the same process
          // only if loop back flag is set true
          if(m_loopback || (ecal_sample_.topic().pid() != Process::GetProcessID()))
          {
            if (g_subgate()) g_subgate()->ApplyLocPubRegistration(ecal_sample_);
          }
        }
        else
        {
          if(m_network)
          {
            if (g_subgate()) g_subgate()->ApplyExtPubRegistration(ecal_sample_);
          }
        }
        if (m_callback_pub) m_callback_pub(reg_sample.c_str(), static_cast<int>(reg_sample.size()));
      }
      break;
    default:
      {
        eCAL::Logging::Log(log_level_debug1, "CRegGate::ApplySample : unknown sample type");
      }
      break;
    }

    return 0;
  }

  bool CRegistrationReceiver::AddRegistrationCallback(enum eCAL_Registration_Event event_, RegistrationCallbackT callback_)
  {
    if (!m_created) return false;
    switch (event_)
    {
    case reg_event_publisher:
      m_callback_pub = callback_;
      return true;
    case reg_event_subscriber:
      m_callback_sub = callback_;
      return true;
    case reg_event_service:
      m_callback_service = callback_;
      return true;
    case reg_event_client:
      m_callback_client = callback_;
      return true;
    case reg_event_process:
      m_callback_process = callback_;
      return true;
    default:
      return false;
    }
  }

  bool CRegistrationReceiver::RemRegistrationCallback(enum eCAL_Registration_Event event_)
  {
    if (!m_created) return false;
    switch (event_)
    {
    case reg_event_publisher:
      m_callback_pub = nullptr;
      return true;
    case reg_event_subscriber:
      m_callback_sub = nullptr;
      return true;
    case reg_event_service:
      m_callback_service = nullptr;
      return true;
    case reg_event_client:
      m_callback_client = nullptr;
      return true;
    case reg_event_process:
      m_callback_process = nullptr;
      return true;
    default:
      return false;
    }
  }

  bool CRegistrationReceiver::IsLocalHost(const eCAL::pb::Sample& ecal_sample_)
  {
    const std::string host_name = ecal_sample_.topic().hname();
    if (host_name.empty()) return false;
    if (host_name != eCAL::Process::GetHostName()) return false;
    return true;
  }

  void CRegistrationReceiver::SetCustomApplySampleCallback(const ApplySampleCallbackT& callback_)
  {
    m_callback_custom_apply_sample = callback_;
  }

  void CRegistrationReceiver::RemCustomApplySampleCallback()
  {
    m_callback_custom_apply_sample = [](const auto&){};
  }
};
