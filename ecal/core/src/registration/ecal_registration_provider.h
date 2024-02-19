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
#include <unordered_map>
#include <vector>

namespace eCAL
{
  class CRegistrationProvider
  {
  public:
    CRegistrationProvider();
    ~CRegistrationProvider();

    void Create(bool topics_, bool services_, bool process_);
    void Destroy();

    bool RegisterTopic(const std::string& topic_name_, const std::string& topic_id_, const Registration::Sample& ecal_sample_, bool force_);
    bool UnregisterTopic(const std::string& topic_name_, const std::string& topic_id_, const Registration::Sample& ecal_sample_, bool force_);

    bool RegisterServer(const std::string& service_name_, const std::string& service_id_, const Registration::Sample& ecal_sample_, bool force_);
    bool UnregisterServer(const std::string& service_name_, const std::string& service_id_, const Registration::Sample& ecal_sample_, bool force_);

    bool RegisterClient(const std::string& client_name_, const std::string& client_id_, const Registration::Sample& ecal_sample_, bool force_);
    bool UnregisterClient(const std::string& client_name_, const std::string& client_id_, const Registration::Sample& ecal_sample_, bool force_);

  protected:
    bool RegisterProcess();
    bool UnregisterProcess();
      
    bool RegisterTopics();

    bool RegisterServer();
    bool RegisterClient();

    bool ApplySample(const std::string& sample_name_, const eCAL::Registration::Sample& sample_);
      
    void RegisterSendThread();

    bool SendSampleList(bool reset_sample_list_ = true);

    static std::atomic<bool>            m_created;
    bool                                m_reg_topics;
    bool                                m_reg_services;
    bool                                m_reg_process;

    std::shared_ptr<UDP::CSampleSender> m_reg_sample_snd;
    std::shared_ptr<CCallbackThread>    m_reg_sample_snd_thread;

    std::mutex                          m_sample_buffer_sync;
    std::vector<char>                   m_sample_buffer;

    using SampleMapT = std::unordered_map<std::string, Registration::Sample>;
    std::mutex                          m_topics_map_sync;
    SampleMapT                          m_topics_map;

    std::mutex                          m_server_map_sync;
    SampleMapT                          m_server_map;

    std::mutex                          m_client_map_sync;
    SampleMapT                          m_client_map;

#if ECAL_CORE_REGISTRATION_SHM
    std::mutex                          m_sample_list_sync;
    Registration::SampleList            m_sample_list;
    std::vector<char>                   m_sample_list_buffer;

    CMemoryFileBroadcast                m_memfile_broadcast;
    CMemoryFileBroadcastWriter          m_memfile_broadcast_writer;
#endif

    bool                                m_use_registration_udp;
    bool                                m_use_registration_shm;
  };
}
