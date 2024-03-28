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

#include "io/udp/ecal_udp_configurations.h"
#include "ecal_sample_to_topicinfo.h"

#include <chrono>

namespace eCAL
{
  //////////////////////////////////////////////////////////////////
  // CMemfileRegistrationReceiver
  //////////////////////////////////////////////////////////////////

  CMemfileRegistrationReceiver::CMemfileRegistrationReceiver() 
  = default;
  
  CMemfileRegistrationReceiver::~CMemfileRegistrationReceiver()
  {
    Destroy();
  }

  void CMemfileRegistrationReceiver::Create(eCAL::CMemoryFileBroadcastReader* memfile_broadcast_reader_)
  {
    if (m_created) return;

    // start memfile broadcast receive thread
    m_memfile_broadcast_reader = memfile_broadcast_reader_;
    m_memfile_broadcast_reader_thread = std::make_shared<CCallbackThread>(std::bind(&CMemfileRegistrationReceiver::Receive, this));
    m_memfile_broadcast_reader_thread->start(std::chrono::milliseconds(g_ecal_config()->registration_options.getRefreshMS()/2));

    m_created = true;
  }

  void CMemfileRegistrationReceiver::Destroy()
  {
    if (!m_created) return;

    // stop memfile broadcast receive thread
    m_memfile_broadcast_reader_thread->stop();
    m_memfile_broadcast_reader = nullptr;

    m_created = false;
  }

  void CMemfileRegistrationReceiver::Receive()
  {
    MemfileBroadcastMessageListT message_list;
    if (m_memfile_broadcast_reader->Read(message_list, 0))
    {
      eCAL::pb::SampleList sample_list;

      for (const auto& message : message_list)
      {
        if (sample_list.ParseFromArray(message.data, static_cast<int>(message.size)))
        {
          for (const auto& sample : sample_list.samples())
          {
            if (g_registration_receiver()) g_registration_receiver()->ApplySample(sample);
          }
        }
      }
    }
  }

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
                         m_callback_custom_apply_sample([](const auto&){}),
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
    m_network = g_ecal_config()->transport_layer_options.network_enabled;

    m_use_shm_monitoring = static_cast<bool>(Config::GetCurrentConfig()->monitoring_options.monitoring_mode & Config::MonitoringMode::shm_monitoring);
    m_use_network_monitoring = !Config::GetCurrentConfig()->monitoring_options.network_monitoring_disabled;

    if (m_use_network_monitoring)
    {
      // set network attributes
      IO::UDP::SReceiverAttr attr;
      attr.address   = UDP::GetRegistrationAddress();
      attr.port      = UDP::GetRegistrationPort();
      attr.broadcast = UDP::IsBroadcast();
      attr.loopback  = true;
      attr.rcvbuf    = Config::GetCurrentConfig()->transport_layer_options.mc_options.recbuf.get();

      // start registration sample receiver
      m_registration_receiver = std::make_shared<UDP::CSampleReceiver>(attr, std::bind(&CRegistrationReceiver::HasSample, this, std::placeholders::_1), std::bind(&CRegistrationReceiver::ApplySample, this, std::placeholders::_1));
    }

    if (m_use_shm_monitoring)
    {
      m_memfile_broadcast.Create(Config::GetCurrentConfig()->monitoring_options.shm_options.shm_monitoring_domain, Config::GetCurrentConfig()->monitoring_options.shm_options.shm_monitoring_queue_size);
      m_memfile_broadcast.FlushLocalEventQueue();
      m_memfile_broadcast_reader.Bind(&m_memfile_broadcast);

      m_memfile_reg_rcv.Create(&m_memfile_broadcast_reader);
    }

    m_created = true;
  }

  void CRegistrationReceiver::Destroy()
  {
    if(!m_created) return;

    // stop network registration receive thread
    if(m_use_network_monitoring)
    {
      m_registration_receiver = nullptr;
    }

    if(m_use_shm_monitoring)
    {
      // stop memfile registration receive thread and unbind reader
      m_memfile_broadcast_reader.Unbind();
      m_memfile_broadcast.Destroy();
    }

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

  bool CRegistrationReceiver::ApplySample(const eCAL::pb::Sample& ecal_sample_)
  {
    if(!m_created) return false;

    //Remove in eCAL6
    // for the time being we need to copy the incoming sample and set the incompatible fields
    eCAL::pb::Sample modified_ttype_sample;
    ModifyIncomingSampleForBackwardsCompatibility(ecal_sample_, modified_ttype_sample);

    // forward all registration samples to outside "customer" (e.g. Monitoring)
    {
      const std::lock_guard<std::mutex> lock(m_callback_custom_apply_sample_mtx);
      m_callback_custom_apply_sample(modified_ttype_sample);
    }

    std::string reg_sample;
    if ( m_callback_pub
      || m_callback_sub
      || m_callback_service
      || m_callback_client
      || m_callback_process
      )
    {
      reg_sample = modified_ttype_sample.SerializeAsString();
    }

    switch(modified_ttype_sample.cmd_type())
    {
    case eCAL::pb::bct_none:
    case eCAL::pb::bct_set_sample:
      break;
    case eCAL::pb::bct_reg_process:
    case eCAL::pb::bct_unreg_process:
      // unregistration event not implemented currently
      if (m_callback_process) m_callback_process(reg_sample.c_str(), static_cast<int>(reg_sample.size()));
      break;
    case eCAL::pb::bct_reg_service:
      if (g_clientgate() != nullptr)  g_clientgate()->ApplyServiceRegistration(modified_ttype_sample);
      if (m_callback_service) m_callback_service(reg_sample.c_str(), static_cast<int>(reg_sample.size()));
      break;
    case eCAL::pb::bct_unreg_service:
      // current client implementation doesn't need that information
      if (m_callback_service) m_callback_service(reg_sample.c_str(), static_cast<int>(reg_sample.size()));
      break;
    case eCAL::pb::bct_reg_client:
      // current service implementation doesn't need that information
      if (m_callback_client) m_callback_client(reg_sample.c_str(), static_cast<int>(reg_sample.size()));
      break;
    case eCAL::pb::bct_unreg_client:
      // current service implementation doesn't need that information
      if (m_callback_client) m_callback_client(reg_sample.c_str(), static_cast<int>(reg_sample.size()));
      break;
    case eCAL::pb::bct_reg_subscriber:
    case eCAL::pb::bct_unreg_subscriber:
      ApplySubscriberRegistration(modified_ttype_sample);
      if (m_callback_sub) m_callback_sub(reg_sample.c_str(), static_cast<int>(reg_sample.size()));
      break;
    case eCAL::pb::bct_reg_publisher:
    case eCAL::pb::bct_unreg_publisher:
      ApplyPublisherRegistration(modified_ttype_sample);
      if (m_callback_pub) m_callback_pub(reg_sample.c_str(), static_cast<int>(reg_sample.size()));
      break;
    default:
      eCAL::Logging::Log(log_level_debug1, "CRegistrationReceiver::ApplySample : unknown sample type");
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

  void CRegistrationReceiver::ApplySubscriberRegistration(const eCAL::pb::Sample& ecal_sample_)
  {
    // process registrations from same host group
    if (IsHostGroupMember(ecal_sample_))
    {
      // do not register local entities, only if loop back flag is set true
      if (m_loopback || (ecal_sample_.topic().pid() != Process::GetProcessID()))
      {
        if (g_pubgate() != nullptr)
        {
          switch (ecal_sample_.cmd_type())
          {
          case eCAL::pb::bct_reg_subscriber:
            g_pubgate()->ApplyLocSubRegistration(ecal_sample_);
            break;
          case eCAL::pb::bct_unreg_subscriber:
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
          switch (ecal_sample_.cmd_type())
          {
          case eCAL::pb::bct_reg_subscriber:
            g_pubgate()->ApplyExtSubRegistration(ecal_sample_);
            break;
          case eCAL::pb::bct_unreg_subscriber:
            g_pubgate()->ApplyExtSubUnregistration(ecal_sample_);
            break;
          default:
            break;
          }
        }
      }
    }
  }

  void CRegistrationReceiver::ApplyPublisherRegistration(const eCAL::pb::Sample& ecal_sample_)
  {
    // process registrations from same host group 
    if (IsHostGroupMember(ecal_sample_))
    {
      // do not register local entities, only if loop back flag is set true
      if (m_loopback || (ecal_sample_.topic().pid() != Process::GetProcessID()))
      {
        if (g_subgate() != nullptr)
        {
          switch (ecal_sample_.cmd_type())
          {
          case eCAL::pb::bct_reg_publisher:
            g_subgate()->ApplyLocPubRegistration(ecal_sample_);
            break;
          case eCAL::pb::bct_unreg_publisher:
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
          switch (ecal_sample_.cmd_type())
          {
          case eCAL::pb::bct_reg_publisher:
            g_subgate()->ApplyExtPubRegistration(ecal_sample_);
            break;
          case eCAL::pb::bct_unreg_publisher:
            g_subgate()->ApplyExtPubUnregistration(ecal_sample_);
            break;
          default:
            break;
          }
        }
      }
    }
  }

  bool CRegistrationReceiver::IsHostGroupMember(const eCAL::pb::Sample& ecal_sample_)
  {
    const std::string& sample_host_group_name = ecal_sample_.topic().hgname().empty() ? ecal_sample_.topic().hname() : ecal_sample_.topic().hgname();

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
    m_callback_custom_apply_sample = [](const auto&){};
  }
}
