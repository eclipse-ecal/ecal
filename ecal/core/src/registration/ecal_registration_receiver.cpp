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

  CRegistrationReceiver::CRegistrationReceiver()
    : m_use_registration_udp(false)
    , m_use_registration_shm(false)
    , m_sample_applier(Config::IsNetworkEnabled(), false, Process::GetHostGroupName(), Process::GetProcessID())
  {
    // Connect User registration callback and gates callback with the sample applier
    m_sample_applier.SetCustomApplySampleCallback("gates", [](const eCAL::Registration::Sample& sample_)
      {
        Registration::CSampleApplierGates::ApplySample(sample_);
      });
    m_sample_applier.SetCustomApplySampleCallback("custom_registration", [this](const eCAL::Registration::Sample& sample_)
      {
        m_user_applier.ApplySample(sample_);
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
    return m_user_applier.AddRegistrationCallback(event_, callback_);
  }

  bool CRegistrationReceiver::RemRegistrationCallback(enum eCAL_Registration_Event event_)
  {
    return m_user_applier.RemRegistrationCallback(event_);
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
