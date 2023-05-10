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

#include "ecal_thread.h"
#include "io/udp_sender.h"

#include "io/ecal_memfile_broadcast.h"
#include "io/ecal_memfile_broadcast_writer.h"

#include <atomic>
#include <mutex>
#include <string>
#include <unordered_map>
#include <memory>

#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#include <ecal/core/pb/ecal.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace eCAL
{
  class CRegistrationProvider
  {
  public:
    CRegistrationProvider();
    ~CRegistrationProvider();

    void Create(bool topics_, bool services_, bool process_);
    void Destroy();

    bool RegisterTopic(const std::string& topic_name_, const std::string& topic_id_, const eCAL::pb::Sample& ecal_sample_, bool force_);
    bool UnregisterTopic(const std::string& topic_name_, const std::string& topic_id_, const eCAL::pb::Sample& ecal_sample_, bool force_);

    bool RegisterServer(const std::string& service_name_, const std::string& service_id_, const eCAL::pb::Sample& ecal_sample_, bool force_);
    bool UnregisterServer(const std::string& service_name_, const std::string& service_id_, const eCAL::pb::Sample& ecal_sample_, bool force_);

    bool RegisterClient(const std::string& client_name_, const std::string& client_id_, const eCAL::pb::Sample& ecal_sample_, bool force_);
    bool UnregisterClient(const std::string& client_name_, const std::string& client_id_, const eCAL::pb::Sample& ecal_sample_, bool force_);

  protected:
    bool RegisterProcess();
    bool UnregisterProcess();
      
    bool RegisterServer();
    bool RegisterClient();
    bool RegisterTopics();

    bool ApplySample(const std::string& sample_name_, const eCAL::pb::Sample& sample_);
      
    int RegisterSendThread();

    bool ApplyTopicToDescGate(const std::string& topic_name_
      , const std::string& topic_type_
      , const std::string& topic_desc_
      , bool topic_is_a_publisher_);

    bool ApplyServiceToDescGate(const std::string& service_name_
      , const std::string& method_name_
      , const std::string& req_type_name_
      , const std::string& req_type_desc_
      , const std::string& resp_type_name_
      , const std::string& resp_type_desc_);

    bool SendSampleList(bool reset_sample_list_ = true);

    static std::atomic<bool>  m_created;
    std::string               m_multicast_group;
    int                       m_reg_refresh;
    bool                      m_reg_topics;
    bool                      m_reg_services;
    bool                      m_reg_process;

    CUDPSender                m_reg_snd;
    CThread                   m_reg_snd_thread;

    using SampleMapT = std::unordered_map<std::string, eCAL::pb::Sample>;
    std::mutex                m_topics_map_sync;
    SampleMapT                m_topics_map;

    std::mutex                m_server_map_sync;
    SampleMapT                m_server_map;

    std::mutex                m_client_map_sync;
    SampleMapT                m_client_map;

    std::mutex                m_sample_list_sync;
    eCAL::pb::SampleList      m_sample_list;
    std::string               m_sample_list_buffer;

    eCAL::CMemoryFileBroadcast m_memfile_broadcast;
    eCAL::CMemoryFileBroadcastWriter m_memfile_broadcast_writer;

    bool m_use_network_monitoring;
    bool m_use_shm_monitoring;
  };
};
