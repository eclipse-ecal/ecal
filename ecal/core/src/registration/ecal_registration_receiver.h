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
#include "registration/ecal_registration_sample_applier.h"
#include "registration/ecal_registration_sample_applier_gates.h"
#include "config/attributes/registration_attributes.h"

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

  namespace Registration
  {
    template<typename T>
    class CTimeoutProvider;
  }
  class CCallbackThread;

  class CRegistrationReceiver
  {
  public:
    CRegistrationReceiver(const Registration::SAttributes& attr_);
    ~CRegistrationReceiver();

    //what about the rest of the rule of 5?

    void Start();
    void Stop();

    using ApplySampleCallbackT = std::function<void(const Registration::Sample&)>;
    void SetCustomApplySampleCallback(const std::string& customer_, const ApplySampleCallbackT& callback_);
    void RemCustomApplySampleCallback(const std::string& customer_);

  private:
    // why is this a static variable? can someone explain?
    static std::atomic<bool>              m_created;

    // this class gets samples and tracks them for timouts
    std::unique_ptr<Registration::CTimeoutProvider<std::chrono::steady_clock>> m_timeout_provider;
    std::unique_ptr<CCallbackThread>                                           m_timeout_provider_thread;

    std::unique_ptr<CRegistrationReceiverUDP> m_registration_receiver_udp;
#if ECAL_CORE_REGISTRATION_SHM
    std::unique_ptr<CRegistrationReceiverSHM> m_registration_receiver_shm;
#endif

    // This class distributes samples to all everyone who is interested in being notified about samples
    Registration::CSampleApplier     m_sample_applier;

    Registration::SAttributes        m_attributes;
  };
}
