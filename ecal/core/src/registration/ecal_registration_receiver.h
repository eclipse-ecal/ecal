/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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

#include <ecal/ecal.h>

#include "ecal_def.h"

#include "io/udp/ecal_udp_sample_receiver.h"

#include "io/shm/ecal_memfile_broadcast.h"
#include "io/shm/ecal_memfile_broadcast_reader.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#include <ecal/core/pb/ecal.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace eCAL
{
  class CMemfileRegistrationReceiver
  {
  public:
    void Create(CMemoryFileBroadcastReader* memfile_broadcast_reader_);
    void Destroy();

  private:
    void Receive();

    CMemoryFileBroadcastReader*      m_memfile_broadcast_reader = nullptr;
    std::shared_ptr<CCallbackThread>  m_memfile_broadcast_reader_thread;

    bool m_created = false;
  };

  using ApplySampleCallbackT = std::function<void (const eCAL::pb::Sample &)>;

  class CRegistrationReceiver
  {
  public:
    CRegistrationReceiver();
    ~CRegistrationReceiver();

    void Create();
    void Destroy();

    void EnableLoopback(bool state_);
    bool LoopBackEnabled() const { return m_loopback; };

    bool HasSample(const std::string& /*sample_name_*/) { return(true); };
    bool ApplySample(const eCAL::pb::Sample& ecal_sample_);

    bool AddRegistrationCallback(enum eCAL_Registration_Event event_, const RegistrationCallbackT& callback_);
    bool RemRegistrationCallback(enum eCAL_Registration_Event event_);

    void SetCustomApplySampleCallback(const ApplySampleCallbackT& callback_);
    void RemCustomApplySampleCallback();

  protected:
    void ApplySubscriberRegistration(const eCAL::pb::Sample& ecal_sample_);
    void ApplyPublisherRegistration(const eCAL::pb::Sample& ecal_sample_);

    bool IsHostGroupMember(const eCAL::pb::Sample & ecal_sample_);

    static std::atomic<bool>              m_created;
    bool                                  m_network;
    bool                                  m_loopback;
                                     
    RegistrationCallbackT                 m_callback_pub;
    RegistrationCallbackT                 m_callback_sub;
    RegistrationCallbackT                 m_callback_service;
    RegistrationCallbackT                 m_callback_client;
    RegistrationCallbackT                 m_callback_process;
                                     
    std::shared_ptr<UDP::CSampleReceiver> m_registration_receiver;

    CMemoryFileBroadcast                  m_memfile_broadcast;
    CMemoryFileBroadcastReader            m_memfile_broadcast_reader;

    CMemfileRegistrationReceiver          m_memfile_reg_rcv;

    bool                                  m_use_network_monitoring;
    bool                                  m_use_shm_monitoring;

    std::mutex                            m_callback_custom_apply_sample_mtx;
    ApplySampleCallbackT                  m_callback_custom_apply_sample;

    std::string                           m_host_group_name;
  };
}
