/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#pragma once


#include "registration/ecal_registration_sender.h"
#include "util/ecal_thread.h"
#include "config/attributes/registration_attributes.h"

#include <atomic>
#include <memory>
#include <mutex>

#include "util/ecal_thread.h"

namespace eCAL
{
  class CRegistrationProvider
  {
  public:
    CRegistrationProvider(const Registration::SAttributes& attr_);
    ~CRegistrationProvider();

    void Start();
    void Stop();

    bool RegisterSample(const Registration::Sample& sample_);
    bool UnregisterSample(const Registration::Sample& sample_);

    template<typename DurationType>
    void SetRegistrationRefresh(const DurationType& timeout)
    {
      m_reg_sample_snd_thread->setTimeout(timeout);
    }

  protected:
    void AddSingleSample(const Registration::Sample& sample_);
    void RegisterSendThread();

    static std::atomic<bool>             m_created;

    std::unique_ptr<CRegistrationSender> m_reg_sender;
    std::shared_ptr<CCallbackThread>     m_reg_sample_snd_thread;

    std::mutex                           m_applied_sample_list_mtx;
    Registration::SampleList             m_applied_sample_list;

    Registration::SampleList             m_send_thread_sample_list;

    Registration::SAttributes                  m_attributes;
  };
}
