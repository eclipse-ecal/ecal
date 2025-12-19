/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 * Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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
 * These information will be send cyclic (registration refresh) via UDP or SHM to external eCAL processes.
 * 
**/
#include "ecal_registration_provider.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <iterator>
#include <memory>
#include <mutex>
#include <string>

#include <ecal/config.h>
#include <ecal_globals.h>
#include "ecal_def.h"

#include <registration/ecal_process_registration.h>
#include <registration/udp/ecal_registration_sender_udp.h>
#if ECAL_CORE_REGISTRATION_SHM
#include <registration/shm/ecal_registration_sender_shm.h>
#endif

#include "config/builder/udp_shm_attribute_builder.h"


namespace eCAL
{
  std::atomic<bool> CRegistrationProvider::m_created;

  CRegistrationProvider::CRegistrationProvider(const Registration::SAttributes& attr_) :
                    m_attributes(attr_)
  {
  }

  CRegistrationProvider::~CRegistrationProvider()
  {
    Stop();
  }

  void CRegistrationProvider::Start()
  {
    if(m_created) return;

#if ECAL_CORE_REGISTRATION_SHM
    if (m_attributes.transport_mode == Registration::eTransportMode::shm)
    {
      m_reg_sender = std::make_unique<CRegistrationSenderSHM>(Registration::BuildSHMAttributes(m_attributes));
    } else
#endif
    if (m_attributes.transport_mode == Registration::eTransportMode::udp)
    {
      m_reg_sender = std::make_unique<CRegistrationSenderUDP>(Registration::BuildUDPSenderAttributes(m_attributes));
    }
    else
    {
      eCAL::Logging::Log(Logging::log_level_error, "[CRegistrationProvider] No registration layer enabled.");
      return;
    }

    // start cyclic registration thread
    m_reg_sample_snd_thread = std::make_shared<CCallbackThread>(std::bind(&CRegistrationProvider::RegisterSendThread, this));
    m_reg_sample_snd_thread->start(std::chrono::milliseconds(m_attributes.refresh));

    m_created = true;
  }

  void CRegistrationProvider::Stop()
  {
    if(!m_created) return;

    // add unregistration sample to registration loop
    AddSingleSample(Registration::GetProcessUnregisterSample());

    // wake up registration thread the last time
    m_reg_sample_snd_thread->trigger();

    // stop cyclic registration thread
    m_reg_sample_snd_thread->stop();

    // delete registration sender
    m_reg_sender.reset();

    m_created = false;
  }

  // (re)register single sample
  bool CRegistrationProvider::RegisterSample(const Registration::Sample& sample_)
  {
    if (!m_created) return(false);

    // add registration sample to registration loop
    AddSingleSample(sample_);

    // wake up registration thread
    m_reg_sample_snd_thread->trigger();

    return(true);
  }

  // unregister single sample
  bool CRegistrationProvider::UnregisterSample(const Registration::Sample& sample_)
  {
    if (!m_created) return(false);

    // add registration sample to registration loop, no need to force registration thread to send
    AddSingleSample(sample_);

    return(true);
  }

  void CRegistrationProvider::AddSingleSample(const Registration::Sample& sample_)
  {
    const std::lock_guard<std::mutex> lock(m_sample_db_mtx);
    m_sample_db.insert_or_assign(sample_.identifier.entity_id, sample_);
  }

  void CRegistrationProvider::RegisterSendThread()
  {
    // collect all registrations and send them out cyclic
    {
      const std::lock_guard<std::mutex> lock(m_sample_db_mtx);
      // and add process registration sample
      UpdateProcessRegistration(m_sample_db);

#if ECAL_CORE_SUBSCRIBER
      // add subscriber registrations
      auto subgate = g_subgate();
      if (subgate) subgate->UpdateRegistrationDatabase(m_sample_db);
#endif

#if ECAL_CORE_PUBLISHER
      // add publisher registrations
      auto pubgate = g_pubgate();
      if (pubgate) pubgate->UpdateRegistrationDatabase(m_sample_db);
#endif

#if ECAL_CORE_SERVICE
      // add server registrations
      auto servicegate = g_servicegate();
      if (servicegate) servicegate->UpdateRegistrationDatabase(m_sample_db);

      // add client registrations
      auto clientgate = g_clientgate();
      if (clientgate) clientgate->UpdateRegistrationDatabase(m_sample_db);
#endif

      // send collected registration sample list
      m_reg_sender->SendSamples(m_sample_db);
    }
  }
}
