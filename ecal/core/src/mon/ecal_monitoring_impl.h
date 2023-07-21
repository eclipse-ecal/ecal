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

#include "ecal_monitoring_threads.h"
#include "ecal_expmap.h"
#include "io/rcv_sample.h"

#include <memory>
#include <mutex>
#include <string>

namespace eCAL
{
  ////////////////////////////////////////
  // Monitoring Declaration
  ////////////////////////////////////////
  class CMonitoringImpl : public CSampleReceiver
  {
  public:
    CMonitoringImpl();
    ~CMonitoringImpl() override = default;

    void Create();
    void Destroy();

    void SetExclFilter(const std::string& filter_);
    void SetInclFilter(const std::string& filter_);
    void SetFilterState(bool state_);

    void GetMonitoringPb(eCAL::pb::Monitoring& monitoring_, unsigned int entities_);
    void GetMonitoringStructs(eCAL::Monitoring::SMonitoring& monitoring_, unsigned int entities_);
    void GetLogging(eCAL::pb::Logging& logging_);

    int PubMonitoring(bool state_, std::string& name_);
    int PubLogging(bool state_, std::string& name_);

    bool ApplySample(const eCAL::pb::Sample& ecal_sample_, eCAL::pb::eTLayerType /*layer_*/) override;

  protected:
    bool HasSample(const std::string& /* sample_name_ */) override { return(true); };

    bool RegisterProcess(const eCAL::pb::Sample& sample_);
    bool UnregisterProcess(const eCAL::pb::Sample& sample_);

    bool RegisterServer(const eCAL::pb::Sample& sample_);
    bool UnregisterServer(const eCAL::pb::Sample& sample_);

    bool RegisterClient(const eCAL::pb::Sample& sample_);
    bool UnregisterClient(const eCAL::pb::Sample& sample_);

    enum ePubSub
    {
      publisher = 1,
      subscriber = 2,
    };

    bool RegisterTopic(const eCAL::pb::Sample& sample_, enum ePubSub pubsub_type_);
    bool UnregisterTopic(const eCAL::pb::Sample& sample_, enum ePubSub pubsub_type_);

    void RegisterLogMessage(const eCAL::pb::LogMessage& log_msg_);

    using TopicMonMapT = eCAL::Util::CExpMap<std::string, eCAL::Monitoring::STopicMon>;
    struct STopicMonMap
    {
      explicit STopicMonMap(const std::chrono::milliseconds& timeout_) :
        map(new TopicMonMapT(timeout_))
      {
      };
      std::mutex                     sync;
      std::unique_ptr<TopicMonMapT>  map;
    };

    using ProcessMonMapT = eCAL::Util::CExpMap<std::string, eCAL::Monitoring::SProcessMon>;
    struct SProcessMonMap
    {
      explicit SProcessMonMap(const std::chrono::milliseconds& timeout_) :
        map(new ProcessMonMapT(timeout_))
      {
      };
      std::mutex                       sync;
      std::unique_ptr<ProcessMonMapT>  map;
    };

    using ServerMonMapT = eCAL::Util::CExpMap<std::string, eCAL::Monitoring::SServerMon>;
    struct SServerMonMap
    {
      explicit SServerMonMap(const std::chrono::milliseconds& timeout_) :
        map(new ServerMonMapT(timeout_))
      {
      };
      std::mutex                      sync;
      std::unique_ptr<ServerMonMapT>  map;
    };

    using ClientMonMapT = eCAL::Util::CExpMap<std::string, eCAL::Monitoring::SClientMon>;
    struct SClientMonMap
    {
      explicit SClientMonMap(const std::chrono::milliseconds& timeout_) :
        map(new ClientMonMapT(timeout_))
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

    void MonitorProcs(eCAL::pb::Monitoring& monitoring_);
    void MonitorServer(eCAL::pb::Monitoring& monitoring_);
    void MonitorClients(eCAL::pb::Monitoring& monitoring_);
    void MonitorTopics(STopicMonMap& map_, eCAL::pb::Monitoring& monitoring_, const std::string& direction_);

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

    // logging
    using LogMessageListT = std::list<eCAL::pb::LogMessage>;
    std::mutex                                   m_log_msglist_sync;
    LogMessageListT                              m_log_msglist;

    // worker threads
    std::shared_ptr<CLoggingReceiveThread>       m_log_rcv_threadcaller;
    std::shared_ptr<CMonLogPublishingThread>     m_pub_threadcaller;
  };
}
