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
    ~CMonitoringImpl();

    void Create();
    void Destroy();

    void SetExclFilter(const std::string& filter_);
    void SetInclFilter(const std::string& filter_);
    void SetFilterState(bool state_);

    void GetMonitoringMsg(eCAL::pb::Monitoring& monitoring_);
    void GetLoggingMsg(eCAL::pb::Logging& logging_);

    int PubMonitoring(bool state_, std::string& name_);
    int PubLogging(bool state_, std::string& name_);

    enum ePubSub
    {
      publisher = 1,
      subscriber = 2,
    };

    bool HasSample(const std::string& /* sample_name_ */) { return(true); };
    size_t ApplySample(const eCAL::pb::Sample& ecal_sample_, eCAL::pb::eTLayerType /*layer_*/);

    bool RegisterProcess(const eCAL::pb::Sample& sample_);
    bool RegisterServer(const eCAL::pb::Sample& sample_);
    bool RegisterClient(const eCAL::pb::Sample& sample_);
    bool RegisterTopic(const eCAL::pb::Sample& sample_, enum ePubSub pubsub_type_);
    void RegisterLogMessage(const eCAL::pb::LogMessage& log_msg_);

  protected:
    struct STopicMon
    {
      STopicMon()
      {
        rclock                = 0;
        pid                   = 0;
        tlayer_ecal_udp_mc    = false;
        tlayer_ecal_shm       = false;
        tlayer_ecal_tcp       = false;
        tlayer_inproc         = false;
        tsize                 = 0;
        connections_loc       = 0;
        connections_ext       = 0;
        did                   = 0;
        dclock                = 0;
        ddropped              = 0;
        dfreq                 = 0;
      };

      int                                rclock;
      std::string                        hname;
      int                                pid;
      std::string                        pname;
      std::string                        uname;
      std::string                        domain;
      std::string                        tname;
      std::string                        tid;
      std::string                        ttype;
      std::string                        tdesc;
      std::map<std::string, std::string> attr;
      bool                               tlayer_ecal_udp_mc;
      bool                               tlayer_ecal_shm;
      bool                               tlayer_ecal_tcp;
      bool                               tlayer_inproc;
      int                                tsize;
      int                                connections_loc;
      int                                connections_ext;
      long long                          did;
      long long                          dclock;
      long long                          ddropped;
      long                               dfreq;
    };
    typedef eCAL::Util::CExpMap<std::string, STopicMon> TopicMonMapT;

    struct STopicMonMap
    {
      explicit STopicMonMap(const std::chrono::milliseconds& timeout_) :
        map(new TopicMonMapT(timeout_))
      {
      };
      std::mutex                     sync;
      std::unique_ptr<TopicMonMapT>  map;
    };

    struct SProcessMon
    {
      SProcessMon()
      {
        rclock = 0;
        pid = 0;
        pmemory = 0;
        pcpu = 0.0f;
        usrptime = 0.0f;
        datawrite = 0;
        dataread = 0;
        state_severity = 0;
        state_severity_level = 0;
        tsync_state = 0;
        component_init_state = 0;
      };

      int            rclock;
      std::string    hname;
      std::string    pname;
      std::string    uname;
      int            pid;
      std::string    pparam;
      long long      pmemory;
      float          pcpu;
      float          usrptime;
      long long      datawrite;
      long long      dataread;
      int            state_severity;
      int            state_severity_level;
      std::string    state_info;
      int            tsync_state;
      std::string    tsync_mod_name;
      int            component_init_state;
      std::string    component_init_info;
      std::string    ecal_runtime_version;
    };
    typedef eCAL::Util::CExpMap<std::string, SProcessMon> ProcessMonMapT;

    struct SProcessMonMap
    {
      explicit SProcessMonMap(const std::chrono::milliseconds& timeout_) :
        map(new ProcessMonMapT(timeout_))
      {
      };
      std::mutex                       sync;
      std::unique_ptr<ProcessMonMapT>  map;
    };

    struct SMethodMon
    {
      SMethodMon()
      {
        call_count = 0;
      };
      std::string  mname;
      std::string  req_type;
      std::string  resp_type;
      long long    call_count;
    };

    struct SServerMon
    {
      SServerMon()
      {
        rclock   = 0;
        pid      = 0;
        tcp_port = 0;
      };

      int                      rclock;
      std::string              hname;
      std::string              sname;
      std::string              sid;
      std::string              pname;
      std::string              uname;
      int                      pid;
      int                      tcp_port;
      std::vector<SMethodMon>  methods;
    };
    typedef eCAL::Util::CExpMap<std::string, SServerMon> ServerMonMapT;

    struct SServerMonMap
    {
      explicit SServerMonMap(const std::chrono::milliseconds& timeout_) :
        map(new ServerMonMapT(timeout_))
      {
      };
      std::mutex                      sync;
      std::unique_ptr<ServerMonMapT>  map;
    };

    struct SClientMon
    {
      SClientMon()
      {
        rclock = 0;
        pid = 0;
      };

      int          rclock;
      std::string  hname;
      std::string  sname;
      std::string  sid;
      std::string  pname;
      std::string  uname;
      int          pid;
    };
    typedef eCAL::Util::CExpMap<std::string, SClientMon> ClientMonMapT;

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
    typedef std::set<std::string, InsensitiveCompare> StrICaseSetT;

    STopicMonMap* GetMap(enum ePubSub pubsub_type_);


    void MonitorProcs(eCAL::pb::Monitoring& monitoring_);
    void MonitorServer(eCAL::pb::Monitoring& monitoring_);
    void MonitorClients(eCAL::pb::Monitoring& monitoring_);
    void MonitorTopics(STopicMonMap& map_, eCAL::pb::Monitoring& monitoring_, const std::string& direction_);

    void Tokenize(const std::string& str, StrICaseSetT& tokens, const std::string& delimiters, bool trimEmpty);

    bool                                         m_init;
    bool                                         m_network;
    std::string                                  m_host_name;

    std::mutex                                   m_topic_filter_excl_mtx;
    std::string                                  m_topic_filter_excl_s;
    StrICaseSetT                                 m_topic_filter_excl;

    std::mutex                                   m_topic_filter_incl_mtx;
    std::string                                  m_topic_filter_incl_s;
    StrICaseSetT                                 m_topic_filter_incl;

    // database
    STopicMonMap                                 m_publisher_map;
    STopicMonMap                                 m_subscriber_map;
    SProcessMonMap                               m_process_map;
    SServerMonMap                                m_server_map;
    SClientMonMap                                m_client_map;

    // logging
    typedef std::list<eCAL::pb::LogMessage> LogMessageListT;
    std::mutex                                   m_log_msglist_sync;
    LogMessageListT                              m_log_msglist;

    // worker threads
    std::shared_ptr<CRegistrationReceiveThread>  m_reg_rcv_threadcaller;
    std::shared_ptr<CLoggingReceiveThread>       m_log_rcv_threadcaller;
    std::shared_ptr<CMonLogPublishingThread>     m_pub_threadcaller;
  };
}
