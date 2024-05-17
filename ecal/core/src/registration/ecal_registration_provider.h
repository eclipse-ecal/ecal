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

#include "util/ecal_thread.h"

#if ECAL_CORE_REGISTRATION_SHM
#include "shm/ecal_memfile_broadcast.h"
#include "shm/ecal_memfile_broadcast_writer.h"
#endif

#include "serialization/ecal_serialize_sample_registration.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace eCAL
{
  class CRegistrationProvider
  {
  public:
    CRegistrationProvider();
    ~CRegistrationProvider();

    void Start();
    void Stop();

    bool ApplySample(const Registration::Sample& sample_, bool force_);

    using ApplySampleCallbackT = std::function<void(const Registration::Sample&)>;
    void SetCustomApplySampleCallback(const std::string& customer_, const ApplySampleCallbackT& callback_);
    void RemCustomApplySampleCallback(const std::string& customer_);

  protected:
    void AddSample2SampleList(const Registration::Sample& sample_);
    bool SendSample2UDP(const Registration::Sample& sample_);

    bool SendSampleList2UDP();
#if ECAL_CORE_REGISTRATION_SHM
    bool SendSampleList2SHM();
#endif
    void ClearSampleList();

    void RegisterSendThread();

    Registration::Sample GetProcessRegisterSample();
    Registration::Sample GetProcessUnregisterSample();

    static std::atomic<bool>            m_created;

    std::shared_ptr<UDP::CSampleSender> m_reg_sample_snd;
    std::shared_ptr<CCallbackThread>    m_reg_sample_snd_thread;

    std::mutex                          m_sample_buffer_mtx;
    std::vector<char>                   m_sample_buffer;

    std::mutex                          m_sample_list_mtx;
    Registration::SampleList            m_sample_list;

#if ECAL_CORE_REGISTRATION_SHM
    std::vector<char>                   m_sample_list_buffer;
    CMemoryFileBroadcast                m_memfile_broadcast;
    CMemoryFileBroadcastWriter          m_memfile_broadcast_writer;
#endif

    bool                                m_use_registration_udp;
    bool                                m_use_registration_shm;

    std::mutex                            m_callback_custom_apply_sample_map_mtx;
    std::map<std::string, ApplySampleCallbackT> m_callback_custom_apply_sample_map;
  };
}
