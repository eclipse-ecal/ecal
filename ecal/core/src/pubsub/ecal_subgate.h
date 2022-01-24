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
 * @brief  eCAL subscriber gateway class
**/

#pragma once

#include "ecal_thread.h"

#include "readwrite/ecal_reader.h"

#include <atomic>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace eCAL
{
  class CSubGate
  {
  public:
    CSubGate();
    ~CSubGate();

    void Create();
    void Destroy();

    bool Register(const std::string& topic_name_, CDataReader* datareader_);
    bool Unregister(const std::string& topic_name_, CDataReader* datareader_);

    bool HasSample(const std::string& sample_name_);
    size_t ApplySample(const eCAL::pb::Sample& ecal_sample_, eCAL::pb::eTLayerType layer_);
    size_t ApplySample(const std::string& topic_name_, const std::string& topic_id_, const char* buf_, size_t len_, long long id_, long long clock_, long long time_, size_t hash_, eCAL::pb::eTLayerType layer_);

    void ApplyLocPubRegistration(const eCAL::pb::Sample& ecal_sample_);
    void ApplyExtPubRegistration(const eCAL::pb::Sample& ecal_sample_);

    void RefreshRegistrations();

  protected:
    static std::atomic<bool> m_created;

    // database data reader
    typedef std::unordered_multimap<std::string, CDataReader*> TopicNameDataReaderMapT;
    std::shared_timed_mutex  m_topic_name_datareader_sync;
    TopicNameDataReaderMapT  m_topic_name_datareader_map;

    eCAL::CThread            m_subtimeout_thread;
    int CheckTimeouts();
  };
};
