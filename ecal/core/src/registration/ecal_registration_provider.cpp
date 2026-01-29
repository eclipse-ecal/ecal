/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 * Copyright 2026 AUMOVIO and subsidiaries. All rights reserved.
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

  CRegistrationProvider::CRegistrationProvider(SRegistrationProviderContext& context_) 
    : m_context(std::move(context_))
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
    if (m_context.memfile_map && m_context.attributes.transport_mode == Registration::eTransportMode::shm)
    {
      m_reg_sender = std::make_unique<CRegistrationSenderSHM>(Registration::BuildSHMAttributes(m_context.attributes), m_context.memfile_map);
    } else
#endif
    if (m_context.attributes.transport_mode == Registration::eTransportMode::udp)
    {
      m_reg_sender = std::make_unique<CRegistrationSenderUDP>(Registration::BuildUDPSenderAttributes(m_context.attributes));
    }
    else
    {
      if (m_context.log_provider) m_context.log_provider->Log(Logging::log_level_error, "[CRegistrationProvider] No registration layer enabled.");
      return;
    }

    // start cyclic registration thread
    m_reg_sample_snd_thread = std::make_shared<CCallbackThread>(std::bind(&CRegistrationProvider::RegisterSendThread, this));
    m_reg_sample_snd_thread->start(std::chrono::milliseconds(m_context.attributes.refresh));

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
    const std::lock_guard<std::mutex> lock(m_applied_sample_list_mtx);
    m_applied_sample_list.push_back(sample_);
  }

  void CRegistrationProvider::RegisterSendThread()
  {
    // collect all registrations and send them out cyclic
    {
      // create sample list
      m_send_thread_sample_list.clear();

      // and add process registration sample
      m_send_thread_sample_list.push_back(Registration::GetProcessRegisterSample());

#if ECAL_CORE_SUBSCRIBER
      // add subscriber registrations
      if (m_context.subgate) m_context.subgate->GetRegistrations(m_send_thread_sample_list);
#endif

#if ECAL_CORE_PUBLISHER
      // add publisher registrations
      if (m_context.pubgate) m_context.pubgate->GetRegistrations(m_send_thread_sample_list);
#endif

#if ECAL_CORE_SERVICE
      // add server registrations
      if (m_context.servicegate) m_context.servicegate->GetRegistrations(m_send_thread_sample_list);

      // add client registrations
      if (m_context.clientgate) m_context.clientgate->GetRegistrations(m_send_thread_sample_list);
#endif

      // append applied samples list to sample list
      if (!m_applied_sample_list.empty())
      {
        const std::lock_guard<std::mutex> lock(m_applied_sample_list_mtx);
        std::copy(m_applied_sample_list.begin(), m_applied_sample_list.end(), std::back_inserter(m_send_thread_sample_list));
        m_applied_sample_list.clear();
      }

      // send collected registration sample list
      m_reg_sender->SendSampleList(m_send_thread_sample_list);
    }
  }
}
