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
 * @brief  Global monitoring class (implementation)
**/

#pragma once

#include <ecal/types/monitoring.h>

#include "ecal_def.h"
#include "util/ecal_expmap.h"

#include "serialization/ecal_serialize_sample_registration.h"

#include <memory>
#include <mutex>
#include <set>
#include <string>

#ifdef ECAL_OS_LINUX
#include <strings.h>  // strcasecmp
#endif

namespace eCAL
{
  ////////////////////////////////////////
  // Monitoring Declaration
  ////////////////////////////////////////
  class CMonitoringImpl
  {
  public:
    CMonitoringImpl();
    ~CMonitoringImpl() = default;

    void Create();
    void Destroy();

    void SetExclFilter(const std::string& filter_);
    void SetInclFilter(const std::string& filter_);
    void SetFilterState(bool state_);

    void GetMonitoring(std::string& monitoring_, unsigned int entities_);
    void GetMonitoring(Monitoring::SMonitoring& monitoring_, unsigned int entities_);

  protected:
    bool ApplySample(const Registration::Sample& ecal_sample_, eTLayerType /*layer_*/);

    bool RegisterProcess(const Registration::Sample& sample_);
    bool UnregisterProcess(const Registration::Sample& sample_);

    bool RegisterServer(const Registration::Sample& sample_);
    bool UnregisterServer(const Registration::Sample& sample_);

    bool RegisterClient(const Registration::Sample& sample_);
    bool UnregisterClient(const Registration::Sample& sample_);

    enum ePubSub
    {
      publisher = 1,
      subscriber = 2,
    };

    bool RegisterTopic(const Registration::Sample& sample_, enum ePubSub pubsub_type_);
    bool UnregisterTopic(const Registration::Sample& sample_, enum ePubSub pubsub_type_);

    using TopicMonMapT = Util::CExpMap<std::string, Monitoring::STopicMon>;
    struct STopicMonMap
    {
      explicit STopicMonMap(const std::chrono::milliseconds& timeout_) :
        map(std::make_unique<TopicMonMapT>(timeout_))
      {
      };
      std::mutex                     sync;
      std::unique_ptr<TopicMonMapT>  map;
    };

    using ProcessMonMapT = Util::CExpMap<std::string, Monitoring::SProcessMon>;
    struct SProcessMonMap
    {
      explicit SProcessMonMap(const std::chrono::milliseconds& timeout_) :
        map(std::make_unique<ProcessMonMapT>(timeout_))
      {
      };
      std::mutex                       sync;
      std::unique_ptr<ProcessMonMapT>  map;
    };

    using ServerMonMapT = Util::CExpMap<std::string, Monitoring::SServerMon>;
    struct SServerMonMap
    {
      explicit SServerMonMap(const std::chrono::milliseconds& timeout_) :
        map(std::make_unique<ServerMonMapT>(timeout_))
      {
      };
      std::mutex                      sync;
      std::unique_ptr<ServerMonMapT>  map;
    };

    using ClientMonMapT = Util::CExpMap<std::string, Monitoring::SClientMon>;
    struct SClientMonMap
    {
      explicit SClientMonMap(const std::chrono::milliseconds& timeout_) :
        map(std::make_unique<ClientMonMapT>(timeout_))
      {
      };
      std::mutex                      sync;
      std::unique_ptr<ClientMonMapT>  map;
    };

    struct InsensitiveCompare
    {
      bool operator() (const std::string& a, const std::string& b) const
      {
#ifdef ECAL_OS_WINDOWS
        return _stricmp(a.c_str(), b.c_str()) < 0;
#endif
#ifdef ECAL_OS_LINUX
        return strcasecmp(a.c_str(), b.c_str()) < 0;
#endif
      }
    };
    using StrICaseSetT = std::set<std::string, InsensitiveCompare>;

    STopicMonMap* GetMap(enum ePubSub pubsub_type_);

    void MonitorProcs(Monitoring::SMonitoring& monitoring_);
    void MonitorServer(Monitoring::SMonitoring& monitoring_);
    void MonitorClients(Monitoring::SMonitoring& monitoring_);
    void MonitorTopics(STopicMonMap& map_, Monitoring::SMonitoring& monitoring_, const std::string& direction_);

    void Tokenize(const std::string& str, StrICaseSetT& tokens, const std::string& delimiters, bool trimEmpty);

    bool                                         m_init;
    std::string                                  m_host_name;

    std::mutex                                   m_topic_filter_excl_mtx;
    std::string                                  m_topic_filter_excl_s;
    StrICaseSetT                                 m_topic_filter_excl;

    std::mutex                                   m_topic_filter_incl_mtx;
    std::string                                  m_topic_filter_incl_s;
    StrICaseSetT                                 m_topic_filter_incl;

    // database
    SProcessMonMap                               m_process_map;
    STopicMonMap                                 m_publisher_map;
    STopicMonMap                                 m_subscriber_map;
    SServerMonMap                                m_server_map;
    SClientMonMap                                m_clients_map;
  };
}
