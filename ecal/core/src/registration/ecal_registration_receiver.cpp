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
#include "ecal_global_accessors.h"

#include "pubsub/ecal_subgate.h"
#include "pubsub/ecal_pubgate.h"
#include "service/ecal_clientgate.h"

#include "io/udp/ecal_udp_configurations.h"
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

namespace eCAL
{
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
                         m_use_registration_udp(false),
                         m_use_registration_shm(false),
                         m_callback_custom_apply_sample([](const auto&) {}),
                         m_host_group_name(Process::GetHostGroupName())
  {
  }

  CRegistrationReceiver::~CRegistrationReceiver()
  {
    Destroy();
  }

  void CRegistrationReceiver::Create()
  {
    if(m_created) return;

    // network mode
    m_network = Config::IsNetworkEnabled();

    // receive registration from shared memory and or udp
    m_use_registration_udp = !Config::Experimental::IsNetworkMonitoringDisabled();
    m_use_registration_shm     = Config::Experimental::IsShmMonitoringEnabled();

    if (m_use_registration_udp)
    {
      // set network attributes
      IO::UDP::SReceiverAttr attr;
      attr.address   = UDP::GetRegistrationAddress();
      attr.port      = UDP::GetRegistrationPort();
      attr.broadcast = UDP::IsBroadcast();
      attr.loopback  = true;
      attr.rcvbuf    = Config::GetUdpMulticastRcvBufSizeBytes();

      // start registration sample receiver
      m_registration_receiver = std::make_shared<UDP::CSampleReceiver>(attr, std::bind(&CRegistrationReceiver::HasSample, this, std::placeholders::_1), std::bind(&CRegistrationReceiver::ApplySerializedSample, this, std::placeholders::_1, std::placeholders::_2));
    }

#if ECAL_CORE_REGISTRATION_SHM
    if (m_use_registration_shm)
    {
      m_memfile_broadcast.Create(Config::Experimental::GetShmMonitoringDomain(), Config::Experimental::GetShmMonitoringQueueSize());
      m_memfile_broadcast.FlushLocalEventQueue();
      m_memfile_broadcast_reader.Bind(&m_memfile_broadcast);

      m_memfile_reg_rcv.Create(&m_memfile_broadcast_reader);
    }
#endif

    m_created = true;
  }

  void CRegistrationReceiver::Destroy()
  {
    if(!m_created) return;

    // stop network registration receive thread
    m_registration_receiver = nullptr;

    // stop network registration receive thread
    if (m_use_registration_udp)
    {
      m_registration_receiver = nullptr;
    }

#if ECAL_CORE_REGISTRATION_SHM
    if (m_use_registration_shm)
    {
      // stop memfile registration receive thread and unbind reader
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

  bool CRegistrationReceiver::ApplySerializedSample(const char* serialized_sample_data_, size_t serialized_sample_size_)
  {
    if(!m_created) return false;

    Registration::Sample ecal_sample;
    if (!DeserializeFromBuffer(serialized_sample_data_, serialized_sample_size_, ecal_sample)) return false;

    return ApplySample(ecal_sample);
  }

  bool CRegistrationReceiver::ApplySample(const Registration::Sample& ecal_sample_)
  {
    if (!m_created) return false;

    // forward all registration samples to outside "customer" (e.g. Monitoring)
    {
      const std::lock_guard<std::mutex> lock(m_callback_custom_apply_sample_mtx);
      m_callback_custom_apply_sample(ecal_sample_);
    }

    std::string reg_sample;
    if (m_callback_pub
      || m_callback_sub
      || m_callback_service
      || m_callback_client
      || m_callback_process
      )
    {
      SerializeToBuffer(ecal_sample_, reg_sample);
    }

    switch (ecal_sample_.cmd_type)
    {
    case bct_none:
    case bct_set_sample:
      break;
    case bct_reg_process:
    case bct_unreg_process:
      // unregistration event not implemented currently
      if (m_callback_process) m_callback_process(reg_sample.c_str(), static_cast<int>(reg_sample.size()));
      break;
#if ECAL_CORE_SERVICE
    case bct_reg_service:
      if (g_clientgate() != nullptr) g_clientgate()->ApplyServiceRegistration(ecal_sample_);
      if (m_callback_service) m_callback_service(reg_sample.c_str(), static_cast<int>(reg_sample.size()));
      break;
    case bct_unreg_service:
      // current client implementation doesn't need that information
      if (m_callback_service) m_callback_service(reg_sample.c_str(), static_cast<int>(reg_sample.size()));
      break;
#endif
    case bct_reg_client:
    case bct_unreg_client:
      // current service implementation doesn't need that information
      if (m_callback_client) m_callback_client(reg_sample.c_str(), static_cast<int>(reg_sample.size()));
      break;
    case bct_reg_subscriber:
    case bct_unreg_subscriber:
      ApplySubscriberRegistration(ecal_sample_);
      if (m_callback_sub) m_callback_sub(reg_sample.c_str(), static_cast<int>(reg_sample.size()));
      break;
    case bct_reg_publisher:
    case bct_unreg_publisher:
      ApplyPublisherRegistration(ecal_sample_);
      if (m_callback_pub) m_callback_pub(reg_sample.c_str(), static_cast<int>(reg_sample.size()));
      break;
    default:
      Logging::Log(log_level_debug1, "CRegistrationReceiver::ApplySample : unknown sample type");
      break;
    }

    return true;
  }

  bool CRegistrationReceiver::AddRegistrationCallback(enum eCAL_Registration_Event event_, const RegistrationCallbackT& callback_)
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

  void CRegistrationReceiver::ApplySubscriberRegistration(const Registration::Sample& ecal_sample_)
  {
#if ECAL_CORE_PUBLISHER
    // process registrations from same host group
    if (IsHostGroupMember(ecal_sample_))
    {
      // do not register local entities, only if loop back flag is set true
      if (m_loopback || (ecal_sample_.topic.pid != Process::GetProcessID()))
      {
        if (g_pubgate() != nullptr)
        {
          switch (ecal_sample_.cmd_type)
          {
          case bct_reg_subscriber:
            g_pubgate()->ApplyLocSubRegistration(ecal_sample_);
            break;
          case bct_unreg_subscriber:
            g_pubgate()->ApplyLocSubUnregistration(ecal_sample_);
            break;
          default:
            break;
          }
        }
      }
    }
    // process external registrations
    else
    {
      if (m_network)
      {
        if (g_pubgate() != nullptr)
        {
          switch (ecal_sample_.cmd_type)
          {
          case bct_reg_subscriber:
            g_pubgate()->ApplyExtSubRegistration(ecal_sample_);
            break;
          case bct_unreg_subscriber:
            g_pubgate()->ApplyExtSubUnregistration(ecal_sample_);
            break;
          default:
            break;
          }
        }
      }
    }
#endif
  }

  void CRegistrationReceiver::ApplyPublisherRegistration(const Registration::Sample& ecal_sample_)
  {
#if ECAL_CORE_SUBSCRIBER
    // process registrations from same host group 
    if (IsHostGroupMember(ecal_sample_))
    {
      // do not register local entities, only if loop back flag is set true
      if (m_loopback || (ecal_sample_.topic.pid != Process::GetProcessID()))
      {
        if (g_subgate() != nullptr)
        {
          switch (ecal_sample_.cmd_type)
          {
          case bct_reg_publisher:
            g_subgate()->ApplyLocPubRegistration(ecal_sample_);
            break;
          case bct_unreg_publisher:
            g_subgate()->ApplyLocPubUnregistration(ecal_sample_);
            break;
          default:
            break;
          }
        }
      }
    }
    // process external registrations
    else
    {
      if (m_network)
      {
        if (g_subgate() != nullptr)
        {
          switch (ecal_sample_.cmd_type)
          {
          case bct_reg_publisher:
            g_subgate()->ApplyExtPubRegistration(ecal_sample_);
            break;
          case bct_unreg_publisher:
            g_subgate()->ApplyExtPubUnregistration(ecal_sample_);
            break;
          default:
            break;
          }
        }
      }
    }
#endif
  }

  bool CRegistrationReceiver::IsHostGroupMember(const Registration::Sample& ecal_sample_)
  {
    const std::string& sample_host_group_name = ecal_sample_.topic.hgname.empty() ? ecal_sample_.topic.hname : ecal_sample_.topic.hgname;

    if (sample_host_group_name.empty() || m_host_group_name.empty()) 
      return false;
    if (sample_host_group_name != m_host_group_name) 
      return false;

    return true;
  }

  void CRegistrationReceiver::SetCustomApplySampleCallback(const ApplySampleCallbackT& callback_)
  {
    const std::lock_guard<std::mutex> lock(m_callback_custom_apply_sample_mtx);
    m_callback_custom_apply_sample = callback_;
  }

  void CRegistrationReceiver::RemCustomApplySampleCallback()
  {
    const std::lock_guard<std::mutex> lock(m_callback_custom_apply_sample_mtx);
    m_callback_custom_apply_sample = [](const auto&) {};
  }
}
