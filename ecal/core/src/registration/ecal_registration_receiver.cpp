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

#include "registration/ecal_registration_timeout_provider.h"
#include "util/ecal_thread.h"

#include "registration/udp/ecal_registration_receiver_udp.h"
#if ECAL_CORE_REGISTRATION_SHM
#include "registration/shm/ecal_registration_receiver_shm.h"
#endif
#include "io/udp/ecal_udp_configurations.h"
#include <ecal/config.h>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

#include "config/builder/udp_shm_attribute_builder.h"
#include "config/builder/sample_applier_attribute_builder.h"

namespace eCAL
{
  //////////////////////////////////////////////////////////////////
  // CRegistrationReceiver
  //////////////////////////////////////////////////////////////////
  std::atomic<bool> CRegistrationReceiver::m_created;

  CRegistrationReceiver::CRegistrationReceiver(const Registration::SAttributes& attr_)
    : m_timeout_provider(nullptr)
    , m_timeout_provider_thread(nullptr)
    , m_registration_receiver_udp(nullptr)
    , m_registration_receiver_shm(nullptr)   
    , m_sample_applier(Registration::SampleApplier::BuildSampleApplierAttributes(attr_))
    , m_attributes(attr_)
  {
    // Connect User registration callback and gates callback with the sample applier
    m_sample_applier.SetCustomApplySampleCallback("gates", [](const eCAL::Registration::Sample& sample_)
      {
        Registration::CSampleApplierGates::ApplySample(sample_);
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

    m_timeout_provider = std::make_unique<Registration::CTimeoutProvider<std::chrono::steady_clock>>(
      m_attributes.timeout,
      [this](const Registration::Sample& sample_)
      {
        return m_sample_applier.ApplySample(sample_);
      }
      );
    m_sample_applier.SetCustomApplySampleCallback("timeout", [this](const eCAL::Registration::Sample& sample_)
      {
        m_timeout_provider->ApplySample(sample_);
      });
    m_timeout_provider_thread = std::make_unique<CCallbackThread>([this]() {m_timeout_provider->CheckForTimeouts(); });
    m_timeout_provider_thread->start(std::chrono::milliseconds(100));

    // Why do we have here different behaviour than in the registration provider?
    if (m_attributes.udp_enabled)    
    {
      m_registration_receiver_udp = std::make_unique<CRegistrationReceiverUDP>([this](const Registration::Sample& sample_) {return m_sample_applier.ApplySample(sample_);}, Registration::BuildUDPReceiverAttributes(m_attributes));
    }

#if ECAL_CORE_REGISTRATION_SHM
    if (m_attributes.shm_enabled)
    {
      m_registration_receiver_shm = std::make_unique<CRegistrationReceiverSHM>([this](const Registration::Sample& sample_) {return m_sample_applier.ApplySample(sample_); }, Registration::BuildSHMAttributes(m_attributes));
    }
#endif

    m_created = true;
  }

  void CRegistrationReceiver::Stop()
  {
    if(!m_created) return;

    // stop network registration receive thread
    if (m_attributes.udp_enabled)
    {
      m_registration_receiver_udp = nullptr;
    }

#if ECAL_CORE_REGISTRATION_SHM
    if (m_attributes.shm_enabled)
    {
      m_registration_receiver_shm = nullptr;
    }
#endif

    m_timeout_provider_thread = nullptr;
    m_sample_applier.RemCustomApplySampleCallback("timeout");
    m_timeout_provider = nullptr;

    m_created          = false;
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
