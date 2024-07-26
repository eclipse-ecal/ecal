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
    m_reg_sample_snd_thread->start(std::chrono::milliseconds(0));

    m_created = true;
  }

  void CRegistrationProvider::Stop()
  {
    if(!m_created) return;

    // stop cyclic registration thread
    m_reg_sample_snd_thread->stop();

    // send process unregistration sample
    //m_reg_sender->SendSample(Registration::GetProcessUnregisterSample());

    // delete registration sender
    m_reg_sender.reset();

    m_created = false;
  }

  // register single sample (currently we do not differ between registration/unregistration)
  bool CRegistrationProvider::RegisterSample(const Registration::Sample& sample_)
  {
    if (!m_created) return(false);
    ProcessSingleSample(sample_);
    return(true);
  }

  // unregister single sample (currently we do not differ between registration/unregistration)
  bool CRegistrationProvider::UnregisterSample(const Registration::Sample& sample_)
  {
    if (!m_created) return(false);
    ProcessSingleSample(sample_);
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

  void CRegistrationProvider::ForwardSample(const Registration::Sample& sample_)
  {
    const std::lock_guard<std::mutex> lock(m_callback_custom_apply_sample_map_mtx);
    for (const auto& iter : m_callback_custom_apply_sample_map)
    {
      iter.second(sample_);
    }
  }

  void CRegistrationProvider::ProcessSingleSample(const Registration::Sample& sample_)
  {
    // forward registration sample to outside "customer" (currently monitoring and descgate)
    ForwardSample(sample_);

    // force rgistration thread to send
    TriggerRegisterSendThread();
  }

  void CRegistrationProvider::TriggerRegisterSendThread()
  {
    {
      const std::lock_guard<std::mutex> lock(m_reg_sample_snd_thread_cv_mtx);
      m_reg_sample_snd_thread_trigger = true;
    }
    m_reg_sample_snd_thread_cv.notify_one();
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

      // send registration sample list
      m_reg_sender->SendSampleList(sample_list);

      // wait for trigger or registration refresh timeout
      {
        std::unique_lock<std::mutex> lock(m_reg_sample_snd_thread_cv_mtx);
        m_reg_sample_snd_thread_cv.wait_for(lock, std::chrono::milliseconds(Config::GetRegistrationRefreshMs()), [this] { return m_reg_sample_snd_thread_trigger; });
        m_reg_sample_snd_thread_trigger = false;
      }
    }
  }
}
