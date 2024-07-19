/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

#include "registration/ecal_registration_receiver.h"

#include "registration/udp/ecal_registration_receiver_udp.h"
#if ECAL_CORE_REGISTRATION_SHM
#include "registration/shm/ecal_registration_receiver_shm.h"
#endif
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
                         m_network(Config::IsNetworkEnabled()),
                         m_loopback(false),
                         m_callback_pub(nullptr),
                         m_callback_sub(nullptr),
                         m_callback_service(nullptr),
                         m_callback_client(nullptr),
                         m_callback_process(nullptr),
                         m_use_registration_udp(false),
                         m_use_registration_shm(false),
                         m_host_group_name(Process::GetHostGroupName())
  {
  }

  CRegistrationReceiver::~CRegistrationReceiver()
  {
    Stop();
  }

  void CRegistrationReceiver::Start()
  {
    if(m_created) return;

    // receive registration via udp or shared memory
    m_use_registration_shm = Config::IsShmRegistrationEnabled();
    m_use_registration_udp = !m_use_registration_shm;

    if (m_use_registration_udp)
    {
      m_registration_receiver_udp = std::make_unique<CRegistrationReceiverUDP>([this](const Registration::Sample& sample_) {return this->ApplySample(sample_); });
    }

#if ECAL_CORE_REGISTRATION_SHM
    if (m_use_registration_shm)
    {
      m_registration_receiver_shm = std::make_unique<CRegistrationReceiverSHM>([this](const Registration::Sample& sample_) {return this->ApplySample(sample_); });
    }
#endif

    m_created = true;
  }

  void CRegistrationReceiver::Stop()
  {
    if(!m_created) return;

    // stop network registration receive thread
    if (m_use_registration_udp)
    {
      m_registration_receiver_udp = nullptr;
    }

#if ECAL_CORE_REGISTRATION_SHM
    if (m_use_registration_shm)
    {
      m_registration_receiver_shm = nullptr;
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

  bool CRegistrationReceiver::ApplySample(const Registration::Sample& sample_)
  {
    if (!m_created) return false;

    // forward all registration samples to outside "customer" (e.g. monitoring, descgate)
    {
      const std::lock_guard<std::mutex> lock(m_callback_custom_apply_sample_map_mtx);
      for (const auto& iter : m_callback_custom_apply_sample_map)
      {
        iter.second(sample_);
      }
    }

    // forward registration to defined gates
    // and store user registration callback
    RegistrationCallbackT reg_callback(nullptr);
    switch (sample_.cmd_type)
    {
    case bct_none:
    case bct_set_sample:
      break;
    case bct_reg_process:
    case bct_unreg_process:
      // unregistration event not implemented currently
      reg_callback = m_callback_process;
      break;
    case bct_reg_service:
    case bct_unreg_service:
      ApplyServiceRegistration(sample_);
      reg_callback = m_callback_service;
      break;
    case bct_reg_client:
    case bct_unreg_client:
      // current client implementation doesn't need that information
      reg_callback = m_callback_client;
      break;
    case bct_reg_subscriber:
    case bct_unreg_subscriber:
      ApplySubscriberRegistration(sample_);
      reg_callback = m_callback_sub;
      break;
    case bct_reg_publisher:
    case bct_unreg_publisher:
      ApplyPublisherRegistration(sample_);
      reg_callback = m_callback_pub;
      break;
    default:
      Logging::Log(log_level_debug1, "CRegistrationReceiver::ApplySample : unknown sample type");
      break;
    }

    // call user registration callback
    if (reg_callback)
    {
      std::string reg_sample;
      if (SerializeToBuffer(sample_, reg_sample))
      {
        reg_callback(reg_sample.c_str(), static_cast<int>(reg_sample.size()));
      }
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

  void CRegistrationReceiver::ApplyServiceRegistration(const eCAL::Registration::Sample& sample_)
  {
#if ECAL_CORE_SERVICE
    if (g_clientgate() == nullptr)           return;
    if (!ShouldProcessRegistration(sample_)) return;

    switch (sample_.cmd_type)
    {
    // current service implementation processes registration information only (not the unregistration)
    case bct_reg_service:
      g_clientgate()->ApplyServiceRegistration(sample_);
      break;
    default:
      break;
    }
#endif
  }

  void CRegistrationReceiver::ApplySubscriberRegistration(const Registration::Sample& sample_)
  {
#if ECAL_CORE_PUBLISHER
    if (g_pubgate() == nullptr)              return;
    if (!ShouldProcessRegistration(sample_)) return;

    switch (sample_.cmd_type)
    {
    case bct_reg_subscriber:
      g_pubgate()->ApplySubRegistration(sample_);
      break;
    case bct_unreg_subscriber:
      g_pubgate()->ApplySubUnregistration(sample_);
      break;
    default:
      break;
    }
#endif
  }

  void CRegistrationReceiver::ApplyPublisherRegistration(const Registration::Sample& sample_)
  {
#if ECAL_CORE_SUBSCRIBER
    if (g_subgate() == nullptr)              return;
    if (!ShouldProcessRegistration(sample_)) return;

    switch (sample_.cmd_type)
    {
    case bct_reg_publisher:
      g_subgate()->ApplyPubRegistration(sample_);
      break;
    case bct_unreg_publisher:
      g_subgate()->ApplyPubUnregistration(sample_);
      break;
    default:
      break;
    }
#endif
  }

  bool CRegistrationReceiver::IsHostGroupMember(const Registration::Sample& sample_)
  {
    std::string host_group_name;
    std::string host_name;
    switch (sample_.cmd_type)
    {
    case bct_reg_publisher:
    case bct_unreg_publisher:
    case bct_reg_subscriber:
    case bct_unreg_subscriber:
      host_group_name = sample_.topic.hgname;
      host_name = sample_.topic.hname;
      break;
    case bct_reg_service:
    case bct_unreg_service:
      //host_group_name = sample_.service.hgname;  // TODO: we need to add hgname attribute to services
      host_name = sample_.service.hname;
      break;
    case bct_reg_client:
    case bct_unreg_client:
      //host_group_name = sample_.client.hgname;  // TODO: we need to add hgname attribute to clients
      host_name = sample_.client.hname;
      break;
    default:
      break;
    }

    const std::string& sample_host_group_name = host_group_name.empty() ? host_name : host_group_name;

    if (sample_host_group_name.empty() || m_host_group_name.empty()) 
      return false;
    if (sample_host_group_name != m_host_group_name) 
      return false;

    return true;
  }

  bool CRegistrationReceiver::ShouldProcessRegistration(const Registration::Sample& sample_)
  {
    // check if the sample is from the same host group
    if (IsHostGroupMember(sample_))
    {
      // register if the sample is from another process
      // or if loopback mode is enabled
      return m_loopback || (sample_.topic.pid != Process::GetProcessID());
    }
    else
    {
      // if the sample is from an external host, register only if network mode is enabled
      return m_network;
    }

    // do not process the registration
    return false;
  }

  void CRegistrationReceiver::SetCustomApplySampleCallback(const std::string& customer_, const ApplySampleCallbackT& callback_)
  {
    const std::lock_guard<std::mutex> lock(m_callback_custom_apply_sample_map_mtx);
    m_callback_custom_apply_sample_map[customer_] = callback_;
  }

  void CRegistrationReceiver::RemCustomApplySampleCallback(const std::string& customer_)
  {
    const std::lock_guard<std::mutex> lock(m_callback_custom_apply_sample_map_mtx);
    auto iter = m_callback_custom_apply_sample_map.find(customer_);
    if(iter != m_callback_custom_apply_sample_map.end())
    {
      m_callback_custom_apply_sample_map.erase(iter);
    }
  }
}
