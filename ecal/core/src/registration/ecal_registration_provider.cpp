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
 * These information will be send cyclic (registration refresh) via UDP or SHM to external eCAL processes.
 * 
**/
#include "ecal_registration_provider.h"

#include <atomic>
#include <chrono>
#include <functional>
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
    m_applied_sample_list.samples.push_back(sample_);
  }

  void CRegistrationProvider::RegisterSendThread()
  {
    // collect all registrations and send them out cyclic
    {
      // create sample list
      Registration::SampleList sample_list;

      // and add process registration sample
      sample_list.samples.push_back(Registration::GetProcessRegisterSample());

#if ECAL_CORE_SUBSCRIBER
      // add subscriber registrations
      if (g_subgate() != nullptr) g_subgate()->GetRegistrations(sample_list);
#endif

#if ECAL_CORE_PUBLISHER
      // add publisher registrations
      if (g_pubgate() != nullptr) g_pubgate()->GetRegistrations(sample_list);
#endif

#if ECAL_CORE_SERVICE
      // add server registrations
      if (g_servicegate() != nullptr) g_servicegate()->GetRegistrations(sample_list);

      // add client registrations
      if (g_clientgate() != nullptr) g_clientgate()->GetRegistrations(sample_list);
#endif

      // send collected registration sample list
      m_reg_sender->SendSampleList(sample_list);

      // send asynchronously applied samples at the end of the registration loop
      {
        const std::lock_guard<std::mutex> lock(m_applied_sample_list_mtx);
        m_reg_sender->SendSampleList(m_applied_sample_list);
        m_applied_sample_list.samples.clear();
      }
    }
  }
}
