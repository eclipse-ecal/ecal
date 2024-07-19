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

#pragma once

#include <cstddef>
#include <ecal/ecal.h>

#include "serialization/ecal_struct_sample_registration.h"

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace eCAL
{
  class CRegistrationReceiverUDP;
  class CRegistrationReceiverSHM;

  class CRegistrationReceiver
  {
  public:
    CRegistrationReceiver();
    ~CRegistrationReceiver();

    void Start();
    void Stop();

    void EnableLoopback(bool state_);

    bool ApplySample(const Registration::Sample& sample_);

    bool AddRegistrationCallback(enum eCAL_Registration_Event event_, const RegistrationCallbackT& callback_);
    bool RemRegistrationCallback(enum eCAL_Registration_Event event_);

    using ApplySampleCallbackT = std::function<void(const Registration::Sample&)>;
    void SetCustomApplySampleCallback(const std::string& customer_, const ApplySampleCallbackT& callback_);
    void RemCustomApplySampleCallback(const std::string& customer_);

  protected:
    void ApplyServiceRegistration(const eCAL::Registration::Sample& sample_);

    void ApplySubscriberRegistration(const eCAL::Registration::Sample& sample_);
    void ApplyPublisherRegistration(const eCAL::Registration::Sample& sample_);

    bool IsHostGroupMember(const eCAL::Registration::Sample& sample_);
    bool ShouldProcessRegistration(const Registration::Sample& sample_);

    static std::atomic<bool>              m_created;
    bool                                  m_network;
    bool                                  m_loopback;
                                     
    RegistrationCallbackT                 m_callback_pub;
    RegistrationCallbackT                 m_callback_sub;
    RegistrationCallbackT                 m_callback_service;
    RegistrationCallbackT                 m_callback_client;
    RegistrationCallbackT                 m_callback_process;

    std::unique_ptr<CRegistrationReceiverUDP> m_registration_receiver_udp;
#if ECAL_CORE_REGISTRATION_SHM
    std::unique_ptr<CRegistrationReceiverSHM> m_registration_receiver_shm;
#endif

    bool                                  m_use_registration_udp;
    bool                                  m_use_registration_shm;

    std::mutex                            m_callback_custom_apply_sample_map_mtx;
    std::map<std::string, ApplySampleCallbackT> m_callback_custom_apply_sample_map;

    std::string                           m_host_group_name;
  };
}
