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
 * @brief  eCAL common register class
**/

#pragma once

#include "ecal_global_accessors.h"

#include "ecal_thread.h"

#include "io/udp_sender.h"
#include "io/snd_sample.h"

#include <mutex>
#include <string>
#include <unordered_map>
#include <atomic>
#include <memory>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4800) // disable proto warnings
#endif
#include "ecal/pb/service.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace eCAL
{
  class CEntityRegister
  {
  public:
    CEntityRegister();
    ~CEntityRegister();

    void Create(bool topics_, bool services_, bool process_);
    void Destroy();

    bool RegisterTopic(const std::string& topic_name_, const std::string& topic_id_, const eCAL::pb::Sample& ecal_sample_, const bool force_);
    bool UnregisterTopic(const std::string& topic_name_, const std::string& topic_id_);

    bool RegisterService(const std::string& service_name_, const eCAL::pb::Sample& ecal_sample_, const bool force_);
    bool UnregisterService(const std::string& service_name_);

  protected:
    size_t RegisterProcess();
    size_t RegisterServices();
    size_t RegisterTopics();
    size_t RegisterSample(const std::string& sample_name_, const eCAL::pb::Sample& sample_);

    int RegisterSendThread();

    static std::atomic<bool>  m_created;
    std::string               m_multicast_group;
    int                       m_reg_refresh;
    bool                      m_reg_topics;
    bool                      m_reg_services;
    bool                      m_reg_process;

    CUDPSender                m_reg_snd;
    CThread                   m_reg_snd_thread;

    typedef std::unordered_map<std::string, eCAL::pb::Sample> SampleMapT;
    std::mutex                m_topics_map_sync;
    SampleMapT                m_topics_map;

    std::mutex                m_service_map_sync;
    SampleMapT                m_service_map;

    //eCAL::pb::Sample            m_process_sample;
  };
};
