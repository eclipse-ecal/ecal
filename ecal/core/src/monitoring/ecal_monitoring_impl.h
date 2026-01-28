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
 * @brief  Global monitoring class (implementation)
**/

#pragma once

#include <ecal/types/monitoring.h>

#include "ecal_def.h"

#include "serialization/ecal_serialize_sample_registration.h"

#include <memory>
#include <map>
#include <mutex>
#include <set>
#include <string>

namespace eCAL
{
  namespace Logging
  {
    class CLogProvider;
  }
  ////////////////////////////////////////
  // Monitoring Declaration
  ////////////////////////////////////////
  class CMonitoringImpl
  {
  public:
    CMonitoringImpl(std::shared_ptr<Logging::CLogProvider> log_provider_);
    ~CMonitoringImpl() = default;

    void Create();
    void Destroy();

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

    using TopicMapT = std::map<EntityIdT, Monitoring::STopic>;
    struct STopicMap
    {
      explicit STopicMap() :
        map(std::make_unique<TopicMapT>())
      {
      };
      std::mutex                  sync;
      std::unique_ptr<TopicMapT>  map;
    };

    using ProcessMapT = std::map<EntityIdT, Monitoring::SProcess>;
    struct SProcessMap
    {
      explicit SProcessMap() :
        map(std::make_unique<ProcessMapT>())
      {
      };
      std::mutex                    sync;
      std::unique_ptr<ProcessMapT>  map;
    };

    using ServerMapT = std::map<EntityIdT, Monitoring::SServer>;
    struct SServerMap
    {
      explicit SServerMap() :
        map(std::make_unique<ServerMapT>())
      {
      };
      std::mutex                      sync;
      std::unique_ptr<ServerMapT>     map;
    };

    using ClientMapT = std::map<EntityIdT, Monitoring::SClient>;
    struct SClientMap
    {
      explicit SClientMap() :
        map(std::make_unique<ClientMapT>())
      {
      };
      std::mutex                      sync;
      std::unique_ptr<ClientMapT>     map;
    };

    STopicMap* GetMap(enum ePubSub pubsub_type_);

    void MonitorProcs(Monitoring::SMonitoring& monitoring_);
    void MonitorServer(Monitoring::SMonitoring& monitoring_);
    void MonitorClients(Monitoring::SMonitoring& monitoring_);
    void MonitorTopics(STopicMap& map_, Monitoring::SMonitoring& monitoring_, const std::string& direction_);

    bool                                         m_init;

    // database
    SProcessMap                               m_process_map;
    STopicMap                                 m_publisher_map;
    STopicMap                                 m_subscriber_map;
    SServerMap                                m_server_map;
    SClientMap                                m_client_map;
    
    std::shared_ptr<Logging::CLogProvider>    m_log_provider;
  };
}
