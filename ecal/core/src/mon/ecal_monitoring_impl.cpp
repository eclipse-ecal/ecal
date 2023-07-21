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

#include <ecal/ecal.h>
#include <ecal/ecal_config.h>

#include "ecal_config_reader_hlp.h"
#include "ecal_monitoring_impl.h"

#include <regex>

#include "../ecal_registration_receiver.h"


namespace eCAL
{
  ////////////////////////////////////////
  // Monitoring Implementation
  ////////////////////////////////////////
  CMonitoringImpl::CMonitoringImpl() :
    m_init(false),
    m_process_map   (std::chrono::milliseconds(Config::GetMonitoringTimeoutMs())),
    m_publisher_map (std::chrono::milliseconds(Config::GetMonitoringTimeoutMs())),
    m_subscriber_map(std::chrono::milliseconds(Config::GetMonitoringTimeoutMs())),
    m_server_map    (std::chrono::milliseconds(Config::GetMonitoringTimeoutMs())),
    m_clients_map   (std::chrono::milliseconds(Config::GetMonitoringTimeoutMs()))
  {
  }

  void CMonitoringImpl::Create()
  {
    if (m_init) return;

    // get name of this host
    m_host_name = Process::GetHostName();

    // utilize registration receiver to enrich monitor information
    g_registration_receiver()->SetCustomApplySampleCallback([this](const auto& ecal_sample_){this->ApplySample(ecal_sample_, eCAL::pb::tl_none);});

    // start logging receive thread
    const CLoggingReceiveThread::LogMessageCallbackT logmsg_cb = std::bind(&CMonitoringImpl::RegisterLogMessage, this, std::placeholders::_1);
    m_log_rcv_threadcaller = std::make_shared<CLoggingReceiveThread>(logmsg_cb);
    m_log_rcv_threadcaller->SetNetworkMode(Config::IsNetworkEnabled());

    // start monitoring and logging publishing thread
    // we really need to remove this feature !
    const CMonLogPublishingThread::MonitoringCallbackT mon_cb = std::bind(&CMonitoringImpl::GetMonitoringPb, this, std::placeholders::_1, Monitoring::Entity::All);
    const CMonLogPublishingThread::LoggingCallbackT    log_cb = std::bind(&CMonitoringImpl::GetLogging, this, std::placeholders::_1);
    m_pub_threadcaller = std::make_shared<CMonLogPublishingThread>(mon_cb, log_cb);

    // setup blacklist and whitelist filter strings#
    m_topic_filter_excl_s = Config::GetMonitoringFilterExcludeList();
    m_topic_filter_incl_s = Config::GetMonitoringFilterIncludeList();

    // setup filtering on by default
    SetFilterState(true);

    m_init = true;
  }

  void CMonitoringImpl::Destroy()
  {
    g_registration_receiver()->RemCustomApplySampleCallback();
    m_init = false;
  }

  void CMonitoringImpl::SetExclFilter(const std::string& filter_)
  {
    m_topic_filter_excl_s = filter_;
  }

  void CMonitoringImpl::SetInclFilter(const std::string& filter_)
  {
    m_topic_filter_incl_s = filter_;
  }

  void CMonitoringImpl::SetFilterState(bool state_)
  {
    if (state_)
    {
      // create excluding filter list
      {
        const std::lock_guard<std::mutex> lock(m_topic_filter_excl_mtx);
        Tokenize(m_topic_filter_excl_s, m_topic_filter_excl, ",;", true);
      }

      // create including filter list
      {
        const std::lock_guard<std::mutex> lock(m_topic_filter_incl_mtx);
        Tokenize(m_topic_filter_incl_s, m_topic_filter_incl, ",;", true);
      }
    }
    else
    {
      {
        const std::lock_guard<std::mutex> lock(m_topic_filter_excl_mtx);
        m_topic_filter_excl.clear();
      }
      {
        const std::lock_guard<std::mutex> lock(m_topic_filter_incl_mtx);
        m_topic_filter_incl.clear();
      }
    }
  }

  bool CMonitoringImpl::ApplySample(const eCAL::pb::Sample& ecal_sample_, eCAL::pb::eTLayerType /*layer_*/)
  {
    switch (ecal_sample_.cmd_type())
    {
    case eCAL::pb::bct_none:
    case eCAL::pb::bct_set_sample:
      break;
    case eCAL::pb::bct_reg_process:
    {
      // register process
      RegisterProcess(ecal_sample_);
    }
    break;
    case eCAL::pb::bct_unreg_process:
    {
      // unregister process
      UnregisterProcess(ecal_sample_);
    }
    break;
    case eCAL::pb::bct_reg_service:
    {
      // register service
      RegisterServer(ecal_sample_);
    }
    break;
    case eCAL::pb::bct_unreg_service:
    {
      // unregister service
      UnregisterServer(ecal_sample_);
    }
    break;
    case eCAL::pb::bct_reg_client:
    {
      // register client
      RegisterClient(ecal_sample_);
    }
    break;
    case eCAL::pb::bct_unreg_client:
    {
      // unregister client
      UnregisterClient(ecal_sample_);
    }
    break;
    case eCAL::pb::bct_reg_publisher:
    {
      // register publisher
      RegisterTopic(ecal_sample_, CMonitoringImpl::publisher);
    }
    break;
    case eCAL::pb::bct_unreg_publisher:
    {
      // unregister publisher
      UnregisterTopic(ecal_sample_, CMonitoringImpl::publisher);
    }
    break;
    case eCAL::pb::bct_reg_subscriber:
    {
      // register subscriber
      RegisterTopic(ecal_sample_, CMonitoringImpl::subscriber);
    }
    break;
    case eCAL::pb::bct_unreg_subscriber:
    {
      // unregister subscriber
      UnregisterTopic(ecal_sample_, CMonitoringImpl::subscriber);
    }
    break;
    default:
    {
      eCAL::Logging::Log(log_level_debug1, "CMonitoringImpl::ApplySample : unknown sample type");  
    }
    break;
    }

    return true;
  }

  bool CMonitoringImpl::RegisterTopic(const eCAL::pb::Sample& sample_, enum ePubSub pubsub_type_)
  {
    const auto& sample_topic = sample_.topic();
    const int          process_id = sample_topic.pid();
    const std::string& topic_name = sample_topic.tname();
    const size_t       topic_size = static_cast<size_t>(sample_topic.tsize());
    bool               topic_tlayer_ecal_udp_mc(false);
    bool               topic_tlayer_ecal_shm(false);
    bool               topic_tlayer_ecal_tcp(false);
    bool               topic_tlayer_inproc(false);
    for (const auto& layer : sample_topic.tlayer())
    {
      topic_tlayer_ecal_udp_mc    |= (layer.type() == eCAL::pb::tl_ecal_udp_mc)    && layer.confirmed();
      topic_tlayer_ecal_shm       |= (layer.type() == eCAL::pb::tl_ecal_shm)       && layer.confirmed();
      topic_tlayer_ecal_tcp       |= (layer.type() == eCAL::pb::tl_ecal_tcp)       && layer.confirmed();
      topic_tlayer_inproc         |= (layer.type() == eCAL::pb::tl_inproc)         && layer.confirmed();
    }
    const size_t       connections_loc = static_cast<size_t>(sample_topic.connections_loc());
    const size_t       connections_ext = static_cast<size_t>(sample_topic.connections_ext());
    const long long    did             = sample_topic.did();
    const long long    dclock          = sample_topic.dclock();
    const long long    message_drops   = sample_topic.message_drops();
    const long         dfreq           = sample_topic.dfreq();

    // check blacklist topic filter
    {
      const std::lock_guard<std::mutex> lock(m_topic_filter_excl_mtx);
      for (const auto& it : m_topic_filter_excl)
      {
        if (std::regex_match(topic_name, std::regex(it, std::regex::icase)))
          return(false);
      }
    }

    // check whitelist topic filter
    bool is_topic_in_filter(false);
    {
      const std::lock_guard<std::mutex> lock(m_topic_filter_incl_mtx);
      is_topic_in_filter = m_topic_filter_incl.empty();
      for (const auto& it : m_topic_filter_incl)
      {
        if (std::regex_match(topic_name, std::regex(it, std::regex::icase)))
        {
          is_topic_in_filter = true;
          break;
        }
      }
    }

    if (!is_topic_in_filter) return (false);

    /////////////////////////////////
    // register in topic map
    /////////////////////////////////
    STopicMonMap* pTopicMap = GetMap(pubsub_type_);
    if (pTopicMap != nullptr)
    {
      // acquire access
      const std::lock_guard<std::mutex> lock(pTopicMap->sync);

      // common infos
      const int          host_id         = sample_topic.hid();
      const std::string& host_name       = sample_topic.hname();
      const std::string& host_group_name = sample_topic.hgname();
      const std::string& process_name    = sample_topic.pname();
      const std::string& unit_name       = sample_topic.uname();
      const std::string& topic_id        = sample_topic.tid();
      std::string        direction;
      switch (pubsub_type_)
      {
      case publisher:
        direction = "publisher";
        break;
      case subscriber:
        direction = "subscriber";
        break;
      default:
        break;
        }
      std::string topic_datatype_encoding   = sample_topic.tdatatype().encoding();
      std::string topic_datatype_name       = sample_topic.tdatatype().name();
      std::string topic_datatype_descriptor = sample_topic.tdatatype().desc();
      auto attr              = sample_topic.attr();

      // try to get topic info
      const std::string topic_name_id = topic_name + topic_id;
      Monitoring::STopicMon& TopicInfo = (*pTopicMap->map)[topic_name_id];

      // set static content
      TopicInfo.hid       = host_id;
      TopicInfo.hname     = host_name;
      TopicInfo.hgname    = host_group_name;
      TopicInfo.pid       = process_id;
      TopicInfo.pname     = process_name;
      TopicInfo.uname     = unit_name;
      TopicInfo.tname     = topic_name;
      TopicInfo.direction = direction;
      TopicInfo.tid       = topic_id;

      // update flexible content
      TopicInfo.rclock++;
      TopicInfo.tdatatype.encoding    = std::move(topic_datatype_encoding);
      TopicInfo.tdatatype.name        = std::move(topic_datatype_name);
      TopicInfo.tdatatype.descriptor  = std::move(topic_datatype_descriptor);

      TopicInfo.attr               = std::map<std::string, std::string>{attr.begin(), attr.end()};
      TopicInfo.tlayer_ecal_udp_mc = topic_tlayer_ecal_udp_mc;
      TopicInfo.tlayer_ecal_shm    = topic_tlayer_ecal_shm;
      TopicInfo.tlayer_ecal_tcp    = topic_tlayer_ecal_tcp;
      TopicInfo.tlayer_inproc      = topic_tlayer_inproc;
      TopicInfo.tsize              = static_cast<int>(topic_size);
      TopicInfo.connections_loc    = static_cast<int>(connections_loc);
      TopicInfo.connections_ext    = static_cast<int>(connections_ext);
      TopicInfo.did                = did;
      TopicInfo.dclock             = dclock;
      TopicInfo.message_drops      = message_drops;
      TopicInfo.dfreq              = dfreq;
    }

    return(true);
  }

  bool CMonitoringImpl::UnregisterTopic(const eCAL::pb::Sample& sample_, enum ePubSub pubsub_type_)
  {
    const auto& sample_topic = sample_.topic();
    const std::string& topic_name = sample_topic.tname();
    const std::string& topic_id   = sample_topic.tid();

    // unregister from topic map
    STopicMonMap* pTopicMap = GetMap(pubsub_type_);
    if (pTopicMap != nullptr)
    {
      // acquire access
      const std::lock_guard<std::mutex> lock(pTopicMap->sync);

      // remove topic info
      const std::string topic_name_id = topic_name + topic_id;
      pTopicMap->map->erase(topic_name_id);
    }

    return(true);
  }

  bool CMonitoringImpl::RegisterProcess(const eCAL::pb::Sample& sample_)
  {
    const auto& sample_process = sample_.process();
    const std::string&    host_name                    = sample_process.hname();
    const std::string&    host_group_name              = sample_process.hgname();
    const std::string&    process_name                 = sample_process.pname();
    const int             process_id                   = sample_process.pid();
    const std::string&    process_param                = sample_process.pparam();
    const std::string&    unit_name                    = sample_process.uname();
    const long long       process_memory               = sample_process.pmemory();
    const float           process_cpu                  = sample_process.pcpu();
    const float           process_usrptime             = sample_process.usrptime();
    const long long       process_datawrite            = sample_process.datawrite();
    const long long       process_dataread             = sample_process.dataread();
    const auto&           sample_process_state         = sample_process.state();
    const int             process_state_severity       = sample_process_state.severity();
    const int             process_state_severity_level = sample_process_state.severity_level();
    const std::string&    process_state_info           = sample_process_state.info();
    const int             process_tsync_state          = sample_process.tsync_state();
    const std::string&    process_tsync_mod_name       = sample_process.tsync_mod_name();
    const int             component_init_state         = sample_process.component_init_state();
    const std::string&    component_init_info          = sample_process.component_init_info();
    const std::string&    ecal_runtime_version         = sample_process.ecal_runtime_version();

    // create map key
    const std::string process_name_id = process_name + std::to_string(process_id);

    // acquire access
    const std::lock_guard<std::mutex> lock(m_process_map.sync);

    // try to get process info
    Monitoring::SProcessMon& ProcessInfo = (*m_process_map.map)[process_name_id];

    // set static content
    ProcessInfo.hname  = host_name;
    ProcessInfo.hgname = host_group_name;
    ProcessInfo.pname  = process_name;
    ProcessInfo.uname  = unit_name;
    ProcessInfo.pid    = process_id;
    ProcessInfo.pparam = process_param;

    // update flexible content
    ProcessInfo.rclock++;
    ProcessInfo.pmemory              = process_memory;
    ProcessInfo.pcpu                 = process_cpu;
    ProcessInfo.usrptime             = process_usrptime;
    ProcessInfo.datawrite            = process_datawrite;
    ProcessInfo.dataread             = process_dataread;
    ProcessInfo.state_severity       = process_state_severity;
    ProcessInfo.state_severity_level = process_state_severity_level;
    ProcessInfo.state_info           = process_state_info;
    ProcessInfo.tsync_state          = process_tsync_state;
    ProcessInfo.tsync_mod_name       = process_tsync_mod_name;
    ProcessInfo.component_init_state = component_init_state;
    ProcessInfo.component_init_info  = component_init_info;
    ProcessInfo.ecal_runtime_version = ecal_runtime_version;

    return(true);
  }

  bool CMonitoringImpl::UnregisterProcess(const pb::Sample &sample_)
  {
    const auto& sample_process = sample_.process();
    const std::string& process_name = sample_process.pname();
    const int          process_id   = sample_process.pid();

    // create map key
    const std::string process_name_id = process_name + std::to_string(process_id);

    // acquire access
    const std::lock_guard<std::mutex> lock(m_process_map.sync);

    // remove process info
    m_process_map.map->erase(process_name_id);

    return(true);
  }

  bool CMonitoringImpl::RegisterServer(const eCAL::pb::Sample& sample_)
  {
    const auto& sample_service = sample_.service();
    const std::string& host_name    = sample_service.hname();
    const std::string& service_name = sample_service.sname();
    const std::string& service_id   = sample_service.sid();
    const std::string& process_name = sample_service.pname();
    const std::string& unit_name    = sample_service.uname();
    const int          process_id   = sample_service.pid();
    const int          tcp_port     = sample_service.tcp_port();

    // create map key
    const std::string service_name_id = service_name + service_id + std::to_string(process_id);

    // acquire access
    const std::lock_guard<std::mutex> lock(m_server_map.sync);

    // try to get service info
    Monitoring::SServerMon& ServerInfo = (*m_server_map.map)[service_name_id];

    // set static content
    ServerInfo.hname    = host_name;
    ServerInfo.sname    = service_name;
    ServerInfo.sid      = service_id;
    ServerInfo.pname    = process_name;
    ServerInfo.uname    = unit_name;
    ServerInfo.pid      = process_id;
    ServerInfo.tcp_port = tcp_port;

    // update flexible content
    ServerInfo.rclock++;
    ServerInfo.methods.clear();
    for (int i = 0; i < sample_.service().methods_size(); ++i)
    {
      struct Monitoring::SMethodMon method;
      auto sample_service_methods = sample_.service().methods(i);
      method.mname      = sample_service_methods.mname();
      method.req_type   = sample_service_methods.req_type();
      method.req_desc   = sample_service_methods.req_desc();
      method.resp_type  = sample_service_methods.resp_type();
      method.resp_desc  = sample_service_methods.resp_desc();
      method.call_count = sample_service_methods.call_count();
      ServerInfo.methods.push_back(method);
    }

    return(true);
  }

  bool CMonitoringImpl::UnregisterServer(const eCAL::pb::Sample& sample_)
  {
    const auto& sample_service = sample_.service();
    const std::string& service_name = sample_service.sname();
    const std::string& service_id   = sample_service.sid();
    const int          process_id   = sample_service.pid();

    // create map key
    const std::string service_name_id = service_name + service_id + std::to_string(process_id);

    // acquire access
    const std::lock_guard<std::mutex> lock(m_server_map.sync);

    // remove service info
    m_server_map.map->erase(service_name_id);

    return(true);
  }

  bool CMonitoringImpl::RegisterClient(const eCAL::pb::Sample& sample_)
  {
    const auto& sample_client = sample_.client();
    const std::string& host_name    = sample_client.hname();
    const std::string& service_name = sample_client.sname();
    const std::string& service_id   = sample_client.sid();
    const std::string& process_name = sample_client.pname();
    const std::string& unit_name    = sample_client.uname();
    const int          process_id   = sample_client.pid();

    // create map key
    const std::string service_name_id = service_name + service_id + std::to_string(process_id);

    // acquire access
    const std::lock_guard<std::mutex> lock(m_clients_map.sync);

    // try to get service info
    Monitoring::SClientMon& ClientInfo = (*m_clients_map.map)[service_name_id];

    // set static content
    ClientInfo.hname = host_name;
    ClientInfo.sname = service_name;
    ClientInfo.sid   = service_id;
    ClientInfo.pname = process_name;
    ClientInfo.uname = unit_name;
    ClientInfo.pid   = process_id;

    // update flexible content
    ClientInfo.rclock++;

    return(true);
  }

  bool CMonitoringImpl::UnregisterClient(const eCAL::pb::Sample& sample_)
  {
    const auto& sample_client = sample_.client();
    const std::string& service_name = sample_client.sname();
    const std::string& service_id   = sample_client.sid();
    const int          process_id   = sample_client.pid();

    // create map key
    const std::string service_name_id = service_name + service_id + std::to_string(process_id);

    // acquire access
    const std::lock_guard<std::mutex> lock(m_clients_map.sync);

    // remove service info
    m_clients_map.map->erase(service_name_id);

    return(true);
  }

  void CMonitoringImpl::RegisterLogMessage(const eCAL::pb::LogMessage& log_msg_)
  {
    const std::lock_guard<std::mutex> lock(m_log_msglist_sync);
    m_log_msglist.emplace_back(log_msg_);
  }

  CMonitoringImpl::STopicMonMap* CMonitoringImpl::GetMap(enum ePubSub pubsub_type_)
  {
    STopicMonMap* pHostMap = nullptr;
    switch (pubsub_type_)
    {
    case publisher:
      pHostMap = &m_publisher_map;
      break;
    case subscriber:
      pHostMap = &m_subscriber_map;
      break;
    }
    return(pHostMap);
  }

  void CMonitoringImpl::GetMonitoringPb(eCAL::pb::Monitoring& monitoring_, unsigned int entities_)
  {
    // clear protobuf object
    monitoring_.Clear();

    if ((entities_ & Monitoring::Entity::Process) != 0u)
    {
      MonitorProcs(monitoring_);
    }

    if ((entities_ & Monitoring::Entity::Publisher) != 0u)
    {
      MonitorTopics(m_publisher_map, monitoring_, "publisher");
    }

    if ((entities_ & Monitoring::Entity::Subscriber) != 0u)
    {
      MonitorTopics(m_subscriber_map, monitoring_, "subscriber");
    }

    if ((entities_ & Monitoring::Entity::Server) != 0u)
    {
      MonitorServer(monitoring_);
    }

    if ((entities_ & Monitoring::Entity::Client) != 0u)
    {
      MonitorClients(monitoring_);
    }
  }

  void CMonitoringImpl::GetMonitoringStructs(eCAL::Monitoring::SMonitoring& monitoring_, unsigned int entities_)
  {
    if ((entities_ & Monitoring::Entity::Process) != 0u)
    {
      // clear target
      monitoring_.process.clear();

      // lock map
      const std::lock_guard<std::mutex> lock(m_process_map.sync);

      // reserve target
      monitoring_.process.reserve(m_process_map.map->size());

      // iterate map
      m_process_map.map->remove_deprecated();
      for (const auto& process : (*m_process_map.map))
      {
        monitoring_.process.emplace_back(process.second);
      }
    }

    if ((entities_ & Monitoring::Entity::Publisher) != 0u)
    {
      // clear target
      monitoring_.publisher.clear();

      // lock map
      const std::lock_guard<std::mutex> lock(m_publisher_map.sync);

      // reserve target
      monitoring_.publisher.reserve(m_publisher_map.map->size());

      // iterate map
      m_publisher_map.map->remove_deprecated();
      for (const auto& publisher : (*m_publisher_map.map))
      {
        monitoring_.publisher.emplace_back(publisher.second);
      }
    }

    if ((entities_ & Monitoring::Entity::Subscriber) != 0u)
    {
      // clear target
      monitoring_.subscriber.clear();

      // lock map
      const std::lock_guard<std::mutex> lock(m_subscriber_map.sync);

      // reserve target
      monitoring_.subscriber.reserve(m_subscriber_map.map->size());

      // iterate map
      m_subscriber_map.map->remove_deprecated();
      for (const auto& subscriber : (*m_subscriber_map.map))
      {
        monitoring_.subscriber.emplace_back(subscriber.second);
      }
    }

    if ((entities_ & Monitoring::Entity::Server) != 0u)
    {
      // clear target
      monitoring_.server.clear();

      // lock map
      const std::lock_guard<std::mutex> lock(m_server_map.sync);

      // reserve target
      monitoring_.server.reserve(m_server_map.map->size());

      // iterate map
      m_server_map.map->remove_deprecated();
      for (const auto& server : (*m_server_map.map))
      {
        monitoring_.server.emplace_back(server.second);
      }
    }

    if ((entities_ & Monitoring::Entity::Client) != 0u)
    {
      // clear target
      monitoring_.clients.clear();

      // lock map
      const std::lock_guard<std::mutex> lock(m_clients_map.sync);

      // reserve target
      monitoring_.clients.reserve(m_clients_map.map->size());

      // iterate map
      m_clients_map.map->remove_deprecated();
      for (const auto& client : (*m_clients_map.map))
      {
        monitoring_.clients.emplace_back(client.second);
      }
    }
  }

  void CMonitoringImpl::GetLogging(eCAL::pb::Logging& logging_)
  {
    // clear protobuf object
    logging_.Clear();

    // acquire access
    const std::lock_guard<std::mutex> lock(m_log_msglist_sync);

    LogMessageListT::const_iterator siter = m_log_msglist.begin();
    while (siter != m_log_msglist.end())
    {
      // add log message
      eCAL::pb::LogMessage* pMonLogMessage = logging_.add_logs();

      // copy content
      pMonLogMessage->CopyFrom(*siter);

      ++siter;
    }

    // empty message list
    m_log_msglist.clear();
  }

  int CMonitoringImpl::PubMonitoring(bool state_, std::string & name_)
  {
    // (de)activate monitor publisher
    m_pub_threadcaller->SetMonState(state_, name_);
    return 0;
  }

  int CMonitoringImpl::PubLogging(bool state_, std::string & name_)
  {
    // (de)activate logging publisher
    m_pub_threadcaller->SetLogState(state_, name_);
    return 0;
  }

  void CMonitoringImpl::MonitorProcs(eCAL::pb::Monitoring& monitoring_)
  {
    // acquire access
    const std::lock_guard<std::mutex> lock(m_process_map.sync);

    // iterate map
    m_process_map.map->remove_deprecated();
    for (const auto& process : (*m_process_map.map))
    {
      // add host
      eCAL::pb::Process* pMonProcs = monitoring_.add_processes();

      // registration clock
      pMonProcs->set_rclock(process.second.rclock);

      // host name
      pMonProcs->set_hname(process.second.hname);

      // host group name
      pMonProcs->set_hgname(process.second.hgname);

      // process name
      pMonProcs->set_pname(process.second.pname);

      // unit name
      pMonProcs->set_uname(process.second.uname);

      // process id
      pMonProcs->set_pid(process.second.pid);

      // process parameter
      pMonProcs->set_pparam(process.second.pparam);

      // process memory
      pMonProcs->set_pmemory(process.second.pmemory);

      // process cpu
      pMonProcs->set_pcpu(process.second.pcpu);

      // process user core time
      pMonProcs->set_usrptime(process.second.usrptime);

      // process data write bytes
      pMonProcs->set_datawrite(process.second.datawrite);

      // process data read bytes
      pMonProcs->set_dataread(process.second.dataread);

      // state
      auto *state = pMonProcs->mutable_state();

      // severity state
      state->set_severity(eCAL::pb::eProcessSeverity(process.second.state_severity));

      // severity level
      state->set_severity_level(eCAL::pb::eProcessSeverityLevel(process.second.state_severity_level));

      // severity info
      state->set_info(process.second.state_info);

      // time synchronization state
      pMonProcs->set_tsync_state(eCAL::pb::eTSyncState(process.second.tsync_state));

      // time synchronization module name
      pMonProcs->set_tsync_mod_name(process.second.tsync_mod_name);

      // eCAL component initialization state
      pMonProcs->set_component_init_state(process.second.component_init_state);

      // eCAL component initialization info
      pMonProcs->set_component_init_info(process.second.component_init_info);

      // eCAL component runtime version
      pMonProcs->set_ecal_runtime_version(process.second.ecal_runtime_version);
    }
  }

  void CMonitoringImpl::MonitorServer(eCAL::pb::Monitoring& monitoring_)
  {
    // acquire access
    const std::lock_guard<std::mutex> lock(m_server_map.sync);

    // iterate map
    m_server_map.map->remove_deprecated();
    for (const auto& server : (*m_server_map.map))
    {
      // add host
      eCAL::pb::Service* pMonService = monitoring_.add_services();

      // registration clock
      pMonService->set_rclock(server.second.rclock);

      // host name
      pMonService->set_hname(server.second.hname);

      // process name
      pMonService->set_pname(server.second.pname);

      // unit name
      pMonService->set_uname(server.second.uname);

      // process id
      pMonService->set_pid(server.second.pid);

      // service name
      pMonService->set_sname(server.second.sname);

      // service id
      pMonService->set_sid(server.second.sid);

      // tcp port
      pMonService->set_tcp_port(server.second.tcp_port);

      // methods
      for (const auto& method : server.second.methods)
      {
        eCAL::pb::Method* pMonMethod = pMonService->add_methods();
        pMonMethod->set_mname(method.mname);
        pMonMethod->set_req_type(method.req_type);
        pMonMethod->set_req_desc(method.req_desc);
        pMonMethod->set_resp_type(method.resp_type);
        pMonMethod->set_resp_desc(method.resp_desc);
        pMonMethod->set_call_count(method.call_count);
      }
    }
  }

  void CMonitoringImpl::MonitorClients(eCAL::pb::Monitoring& monitoring_)
  {
    // acquire access
    const std::lock_guard<std::mutex> lock(m_clients_map.sync);

    // iterate map
    m_clients_map.map->remove_deprecated();
    for (const auto& client : (*m_clients_map.map))
    {
      // add host
      eCAL::pb::Client* pMonClient = monitoring_.add_clients();

      // registration clock
      pMonClient->set_rclock(client.second.rclock);

      // host name
      pMonClient->set_hname(client.second.hname);

      // process name
      pMonClient->set_pname(client.second.pname);

      // unit name
      pMonClient->set_uname(client.second.uname);

      // process id
      pMonClient->set_pid(client.second.pid);

      // service name
      pMonClient->set_sname(client.second.sname);

      // service id
      pMonClient->set_sid(client.second.sid);
    }
  }

  void CMonitoringImpl::MonitorTopics(STopicMonMap& map_, eCAL::pb::Monitoring& monitoring_, const std::string& direction_)
  {
    // acquire access
    const std::lock_guard<std::mutex> lock(map_.sync);

    // iterate map
    map_.map->remove_deprecated();
    for (const auto& topic : (*map_.map))
    {
      // add topic
      eCAL::pb::Topic* pMonTopic = monitoring_.add_topics();

      // registration clock
      pMonTopic->set_rclock(topic.second.rclock);

      // host name
      pMonTopic->set_hname(topic.second.hname);

      // host group name
      pMonTopic->set_hgname(topic.second.hgname);

      // process id
      pMonTopic->set_pid(topic.second.pid);

      // process name
      pMonTopic->set_pname(topic.second.pname);

      // unit name
      pMonTopic->set_uname(topic.second.uname);

      // topic id
      pMonTopic->set_tid(topic.second.tid);

      // topic name
      pMonTopic->set_tname(topic.second.tname);

      // direction
      pMonTopic->set_direction(direction_);

      // remove with eCAL6
      // topic type
      pMonTopic->set_ttype(eCAL::Util::CombinedTopicEncodingAndType(topic.second.tdatatype.encoding, topic.second.tdatatype.name));

      // topic transport layers
      if (topic.second.tlayer_ecal_udp_mc)
      {
        auto *tlayer = pMonTopic->add_tlayer();
        tlayer->set_type(eCAL::pb::tl_ecal_udp_mc);
        tlayer->set_confirmed(true);
      }
      if (topic.second.tlayer_ecal_shm)
      {
        auto *tlayer = pMonTopic->add_tlayer();
        tlayer->set_type(eCAL::pb::tl_ecal_shm);
        tlayer->set_confirmed(true);
      }
      if (topic.second.tlayer_ecal_tcp)
      {
        auto *tlayer = pMonTopic->add_tlayer();
        tlayer->set_type(eCAL::pb::tl_ecal_tcp);
        tlayer->set_confirmed(true);
      }
      if (topic.second.tlayer_inproc)
      {
        auto *tlayer = pMonTopic->add_tlayer();
        tlayer->set_type(eCAL::pb::tl_inproc);
        tlayer->set_confirmed(true);
      }

      // remove with eCAL6
      // topic description
      pMonTopic->set_tdesc(topic.second.tdatatype.descriptor);

      // topic information
      {
        auto *tdatatype = pMonTopic->mutable_tdatatype();
        tdatatype->set_encoding(topic.second.tdatatype.encoding);
        tdatatype->set_name(topic.second.tdatatype.name);
        tdatatype->set_desc(topic.second.tdatatype.descriptor);
      }

      // topic attributes
      *pMonTopic->mutable_attr() = google::protobuf::Map<std::string, std::string> {topic.second.attr.begin(), topic.second.attr.end()};

      // topic size
      pMonTopic->set_tsize(topic.second.tsize);

      // local connections
      pMonTopic->set_connections_loc(topic.second.connections_loc);

      // external connections
      pMonTopic->set_connections_ext(topic.second.connections_ext);

      // data id (publisher setid)
      pMonTopic->set_did(topic.second.did);

      // data clock
      pMonTopic->set_dclock(topic.second.dclock);

      // data dropped
      pMonTopic->set_message_drops(google::protobuf::int32(topic.second.message_drops));

      // data frequency
      pMonTopic->set_dfreq(topic.second.dfreq);
    }
  }

  void CMonitoringImpl::Tokenize(const std::string& str, StrICaseSetT& tokens, const std::string& delimiters, bool trimEmpty)
  {
    std::string::size_type pos = 0;
    std::string::size_type lastPos = 0;

    for (;;)
    {
      pos = str.find_first_of(delimiters, lastPos);
      if (pos == std::string::npos)
      {
        pos = str.length();
        if (pos != lastPos || !trimEmpty)
        {
          tokens.emplace(std::string(str.data() + lastPos, pos - lastPos));
        }
        break;
      }
      else
      {
        if (pos != lastPos || !trimEmpty)
        {
          tokens.emplace(std::string(str.data() + lastPos, pos - lastPos));
        }
      }
      lastPos = pos + 1;
    }
  }
}
