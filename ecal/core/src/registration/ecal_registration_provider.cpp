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
 * @brief  eCAL registration provider
 * 
 * All process internal publisher/subscriber, server/clients register here with all their attributes.
 * 
 * These information will be send cyclic (registration refresh) via UDP to external eCAL processes.
 * 
**/
#include "ecal_registration_provider.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>

#include <ecal/ecal_config.h>
#include <ecal_globals.h>
#include "ecal_def.h"

#include <registration/ecal_process_registration.h>
#include <registration/udp/ecal_registration_sender_udp.h>
#if ECAL_CORE_REGISTRATION_SHM
#include <registration/shm/ecal_registration_sender_shm.h>
#endif

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
    Stop();
  }

  void CRegistrationProvider::Start()
  {
    if(m_created) return;

    // send registration over udp or shared memory
    m_use_registration_shm = Config::IsShmRegistrationEnabled();
    m_use_registration_udp = !m_use_registration_shm;

   // TODO Create the registration sender
#if ECAL_CORE_REGISTRATION_SHM
    if (m_use_registration_shm)
    {
      m_reg_sender = std::make_unique<CRegistrationSenderSHM>();
    }
    else
    {
#endif
      m_reg_sender = std::make_unique<CRegistrationSenderUDP>();
#if ECAL_CORE_REGISTRATION_SHM
    }
#endif

    // start cyclic registration thread
    m_reg_sample_snd_thread = std::make_shared<CCallbackThread>(std::bind(&CRegistrationProvider::RegisterSendThread, this));
    m_reg_sample_snd_thread->start(std::chrono::milliseconds(Config::GetRegistrationRefreshMs()));

    m_created = true;
  }

  void CRegistrationProvider::Stop()
  {
    if(!m_created) return;

    // stop cyclic registration thread
    m_reg_sample_snd_thread->stop();

    // send process unregistration sample
    SendSample(Registration::GetProcessUnregisterSample());

    m_reg_sender.reset();

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

    if (force_)
    {
      // send sample
      SendSample(sample_);
    }
    else
    {
      // add sample to sample list and send it later
      AddSample2SampleList(sample_);
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

  void CRegistrationProvider::SendSample(const Registration::Sample& sample_)
  {
    Registration::SampleList sample_list;
    sample_list.samples.push_back(sample_);
    m_reg_sender->SendSampleList(sample_list);
  }

  void CRegistrationProvider::RegisterSendThread()
  {
    // collect all registrations and send them out
    // the internal list already contain elements here:
    //   one process registration sample
    //   one or more registration/unregistration samples added by AddSample2SampleList
    {
      // lock sample list
      std::lock_guard<std::mutex> lock(m_sample_list_mtx);

#if ECAL_CORE_SUBSCRIBER
      // add subscriber registrations
      if (g_subgate() != nullptr) g_subgate()->GetRegistrations(m_sample_list);
#endif

#if ECAL_CORE_PUBLISHER
      // add publisher registrations
      if (g_pubgate() != nullptr) g_pubgate()->GetRegistrations(m_sample_list);
#endif

#if ECAL_CORE_SERVICE
      // add server registrations
      if (g_servicegate() != nullptr) g_servicegate()->GetRegistrations(m_sample_list);

      // add client registrations
      if (g_clientgate() != nullptr) g_clientgate()->GetRegistrations(m_sample_list);
#endif

      // send registration sample list
      m_reg_sender->SendSampleList(m_sample_list);

      // clear it
      m_sample_list.samples.clear();

      // and add process registration sample to internal sample list as first sample (for next registration loop)
      m_sample_list.samples.push_back(Registration::GetProcessRegisterSample());
    }
  }
}
