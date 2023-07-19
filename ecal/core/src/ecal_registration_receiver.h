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
#include "ecal_thread.h"

#include "io/rcv_sample.h"

#include "io/ecal_memfile_broadcast.h"
#include "io/ecal_memfile_broadcast_reader.h"

#include <string>
#include <atomic>

#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#include <ecal/core/pb/ecal.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace eCAL
{
  class CUdpRegistrationReceiver : public CSampleReceiver
  {
    bool HasSample(const std::string& /*sample_name_*/) override { return(true); };
    bool ApplySample(const eCAL::pb::Sample& ecal_sample_, eCAL::pb::eTLayerType layer_) override;
  };

  class CMemfileRegistrationReceiver
  {
  public:
    bool Create(eCAL::CMemoryFileBroadcastReader* memfile_broadcast_reader_);
    bool Receive();
    bool Destroy();

    bool ApplySample(const eCAL::pb::Sample& ecal_sample_);

  private:
    bool m_created = false;
    eCAL::CMemoryFileBroadcastReader* m_memfile_broadcast_reader = nullptr;
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

    bool ApplySample(const eCAL::pb::Sample& ecal_sample_);

    bool AddRegistrationCallback(enum eCAL_Registration_Event event_, const RegistrationCallbackT& callback_);
    bool RemRegistrationCallback(enum eCAL_Registration_Event event_);

    void SetCustomApplySampleCallback(const ApplySampleCallbackT& callback_);
    void RemCustomApplySampleCallback();


  protected:
    void ApplySubscriberRegistration(const eCAL::pb::Sample& ecal_sample_);
    void ApplyPublisherRegistration(const eCAL::pb::Sample& ecal_sample_);

    bool IsHostGroupMember(const eCAL::pb::Sample & ecal_sample_);

    static std::atomic<bool>         m_created;
    bool                             m_network;
    bool                             m_loopback;
                                     
    RegistrationCallbackT            m_callback_pub;
    RegistrationCallbackT            m_callback_sub;
    RegistrationCallbackT            m_callback_service;
    RegistrationCallbackT            m_callback_client;
    RegistrationCallbackT            m_callback_process;
                                     
    CUDPReceiver                     m_reg_rcv;
    CThread                          m_reg_rcv_thread;
    CUdpRegistrationReceiver         m_reg_rcv_process;

    eCAL::CMemoryFileBroadcast       m_memfile_broadcast;
    eCAL::CMemoryFileBroadcastReader m_memfile_broadcast_reader;
    CMemfileRegistrationReceiver     m_memfile_reg_rcv;
    CThread                          m_memfile_reg_rcv_thread;

    bool                             m_use_network_monitoring;
    bool                             m_use_shm_monitoring;

    ApplySampleCallbackT             m_callback_custom_apply_sample;

    std::string                      m_host_group_name;
  };
};
