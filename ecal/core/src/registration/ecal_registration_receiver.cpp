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
  void CGatesApplier::ApplySample(const eCAL::Registration::Sample& sample_)
  {
    switch (sample_.cmd_type)
    {
    case bct_none:
    case bct_set_sample:
    case bct_reg_process:
    case bct_unreg_process:
      break;
#if ECAL_CORE_SERVICE
    case bct_reg_service:
      if (g_clientgate()) g_clientgate()->ApplyServiceRegistration(sample_);
      break;
#endif
    case bct_unreg_service:
      break;
    case bct_reg_client:
    case bct_unreg_client:
      // current client implementation doesn't need that information
      break;
#if ECAL_CORE_PUBLISHER
    case bct_reg_subscriber:
      if (g_pubgate()) g_pubgate()->ApplySubRegistration(sample_);
      break;
    case bct_unreg_subscriber:
      if (g_pubgate()) g_pubgate()->ApplySubUnregistration(sample_);
      break;
#endif
#if ECAL_CORE_SUBSCRIBER
    case bct_reg_publisher:
      if (g_subgate()) g_subgate()->ApplyPubRegistration(sample_);
      break;
    case bct_unreg_publisher:
      if (g_subgate()) g_subgate()->ApplyPubUnregistration(sample_);
      break;
#endif
    default:
      Logging::Log(log_level_debug1, "CGatesApplier::ApplySample : unknown sample type");
      break;
    }
  }


  bool CRegistrationCallbackApplier::AddRegistrationCallback(eCAL_Registration_Event event_, const RegistrationCallbackT& callback_)
  {
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

  bool CRegistrationCallbackApplier::RemRegistrationCallback(eCAL_Registration_Event event_)
  {
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

  void CRegistrationCallbackApplier::ApplySample(const eCAL::Registration::Sample& sample_)
  {
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
      reg_callback = m_callback_service;
      break;
    case bct_reg_client:
    case bct_unreg_client:
      // current client implementation doesn't need that information
      reg_callback = m_callback_client;
      break;
    case bct_reg_subscriber:
    case bct_unreg_subscriber:
      reg_callback = m_callback_sub;
      break;
    case bct_reg_publisher:
    case bct_unreg_publisher:
      reg_callback = m_callback_pub;
      break;
    default:
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
  }

  //////////////////////////////////////////////////////////////////
  // CRegistrationReceiver
  //////////////////////////////////////////////////////////////////
  std::atomic<bool> CRegistrationReceiver::m_created;

  CRegistrationReceiver::CRegistrationReceiver()
    : m_use_registration_udp(false)
    , m_use_registration_shm(false)
    , m_sample_applier(Config::IsNetworkEnabled(), false, Process::GetHostGroupName(), Process::GetProcessID())
  {
    // Connect User registration callback and gates callback with the sample applier
    m_sample_applier.SetCustomApplySampleCallback("gates", [this](const eCAL::Registration::Sample& sample_)
      {
        m_gates_applier.ApplySample(sample_);
      });
    m_sample_applier.SetCustomApplySampleCallback("custom_registration", [this](const eCAL::Registration::Sample& sample_)
      {
        m_custom_sample_applier.ApplySample(sample_);
      });

  }

  CRegistrationReceiver::~CRegistrationReceiver()
  {
    Stop();

    m_sample_applier.RemCustomApplySampleCallback("custom_registration");
    m_sample_applier.RemCustomApplySampleCallback("gates");
  }

  void CRegistrationReceiver::Start()
  {
    if(m_created) return;

    // receive registration via udp or shared memory
    m_use_registration_shm = Config::IsShmRegistrationEnabled();
    m_use_registration_udp = !m_use_registration_shm;

    if (m_use_registration_udp)
    {
      m_registration_receiver_udp = std::make_unique<CRegistrationReceiverUDP>([this](const Registration::Sample& sample_) {return m_sample_applier.ApplySample(sample_); });
    }

#if ECAL_CORE_REGISTRATION_SHM
    if (m_use_registration_shm)
    {
      m_registration_receiver_shm = std::make_unique<CRegistrationReceiverSHM>([this](const Registration::Sample& sample_) {return m_sample_applier.ApplySample(sample_); });
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

    m_created          = false;
  }

  void CRegistrationReceiver::EnableLoopback(bool state_)
  {
    m_sample_applier.EnableLoopback(state_);
  }



  bool CRegistrationReceiver::AddRegistrationCallback(enum eCAL_Registration_Event event_, const RegistrationCallbackT& callback_)
  {
    return m_custom_sample_applier.AddRegistrationCallback(event_, callback_);
  }

  bool CRegistrationReceiver::RemRegistrationCallback(enum eCAL_Registration_Event event_)
  {
    return m_custom_sample_applier.RemRegistrationCallback(event_);
  }

  void CRegistrationReceiver::SetCustomApplySampleCallback(const std::string& customer_, const ApplySampleCallbackT& callback_)
  {
    m_sample_applier.SetCustomApplySampleCallback(customer_, callback_);
  }

  void CRegistrationReceiver::RemCustomApplySampleCallback(const std::string& customer_)
  {
    m_sample_applier.RemCustomApplySampleCallback(customer_);
  }

}
