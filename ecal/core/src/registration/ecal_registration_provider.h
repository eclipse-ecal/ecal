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

#pragma once

#include "io/udp/ecal_udp_sample_sender.h"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <registration/ecal_registration_sender.h>
#include "util/ecal_thread.h"

namespace eCAL
{
  class CRegistrationProvider
  {
  public:
    CRegistrationProvider();
    ~CRegistrationProvider();

    void Start();
    void Stop();

    bool RegisterSample(const Registration::Sample& sample_);
    bool UnregisterSample(const Registration::Sample& sample_);

  protected:
    void ProcessSingleSample(const Registration::Sample& sample_);
    void AddSingleSample(const Registration::Sample& sample_);

    void TriggerRegisterSendThread();
    void RegisterSendThread();

    static std::atomic<bool>                    m_created;

    std::unique_ptr<CRegistrationSender>        m_reg_sender;
    std::shared_ptr<CCallbackThread>            m_reg_sample_snd_thread;

    std::condition_variable                     m_reg_sample_snd_thread_cv;
    std::mutex                                  m_reg_sample_snd_thread_cv_mtx;
    bool                                        m_reg_sample_snd_thread_trigger;

    std::mutex                                  m_applied_sample_list_mtx;
    Registration::SampleList                    m_applied_sample_list;
      
    bool                                        m_use_registration_udp;
    bool                                        m_use_registration_shm;
  };
}
