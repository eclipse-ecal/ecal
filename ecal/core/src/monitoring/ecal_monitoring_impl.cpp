/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

#include "io/udp/ecal_udp_configurations.h"
#include "ecal_monitoring_impl.h"
#include "ecal_global_accessors.h"

#include <regex>

#include "registration/ecal_registration_provider.h"
#include "registration/ecal_registration_receiver.h"

#include "serialization/ecal_serialize_monitoring.h"


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
    g_registration_receiver()->SetCustomApplySampleCallback("monitoring", [this](const auto& sample_){this->ApplySample(sample_, tl_none);});

    // setup blacklist and whitelist filter strings#
    m_topic_filter_excl_s = Config::GetMonitoringFilterExcludeList();
    m_topic_filter_incl_s = Config::GetMonitoringFilterIncludeList();

    // setup filtering on by default
    SetFilterState(true);

    m_init = true;
  }

  void CMonitoringImpl::Destroy()
  {
    // stop registration receiver utilization to enrich monitor information
    g_registration_receiver()->RemCustomApplySampleCallback("monitoring");
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

  bool CMonitoringImpl::ApplySample(const Registration::Sample& sample_, eTLayerType /*layer_*/)
  {
    switch (sample_.cmd_type)
    {
    case bct_none:
    case bct_set_sample:
      break;
    case bct_reg_process:
    {
      // register process
      RegisterProcess(sample_);
    }
    break;
    case bct_unreg_process:
    {
      // unregister process
      UnregisterProcess(sample_);
    }
    break;
    case bct_reg_service:
    {
      // register service
      RegisterServer(sample_);
    }
    break;
    case bct_unreg_service:
    {
      // unregister service
      UnregisterServer(sample_);
    }
    break;
    case bct_reg_client:
    {
      // register client
      RegisterClient(sample_);
    }
    break;
    case bct_unreg_client:
    {
      // unregister client
      UnregisterClient(sample_);
    }
    break;
    case bct_reg_publisher:
    {
      // register publisher
      RegisterTopic(sample_, CMonitoringImpl::publisher);
    }
    break;
    case bct_unreg_publisher:
    {
      // unregister publisher
      UnregisterTopic(sample_, CMonitoringImpl::publisher);
    }
    break;
    case bct_reg_subscriber:
    {
      // register subscriber
      RegisterTopic(sample_, CMonitoringImpl::subscriber);
    }
    break;
    case bct_unreg_subscriber:
    {
      // unregister subscriber
      UnregisterTopic(sample_, CMonitoringImpl::subscriber);
    }
    break;
    default:
    {
      Logging::Log(log_level_debug1, "CMonitoringImpl::ApplySample : unknown sample type");  
    }
    break;
    }

    return true;
  }

  bool CMonitoringImpl::RegisterTopic(const Registration::Sample& sample_, enum ePubSub pubsub_type_)
  {
    const auto& sample_topic = sample_.topic;
    const int          process_id = sample_topic.pid;
    const std::string& topic_name = sample_topic.tname;
    const int32_t      topic_size = sample_topic.tsize;
    bool               topic_tlayer_ecal_udp(false);
    bool               topic_tlayer_ecal_shm(false);
    bool               topic_tlayer_ecal_tcp(false);
    for (const auto& layer : sample_topic.tlayer)
    {
      topic_tlayer_ecal_udp |= (layer.type == tl_ecal_udp) && layer.active;
      topic_tlayer_ecal_shm |= (layer.type == tl_ecal_shm) && layer.active;
      topic_tlayer_ecal_tcp |= (layer.type == tl_ecal_tcp) && layer.active;
    }
    const int32_t      connections_loc = sample_topic.connections_loc;
    const int32_t      connections_ext = sample_topic.connections_ext;
    const int64_t      did             = sample_topic.did;
    const int64_t      dclock          = sample_topic.dclock;
    const int32_t      message_drops   = sample_topic.message_drops;
    const int32_t      dfreq           = sample_topic.dfreq;

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
      const std::string& host_name       = sample_topic.hname;
      const std::string& host_group_name = sample_topic.hgname;
      const std::string& process_name    = sample_topic.pname;
      const std::string& unit_name       = sample_topic.uname;
      const std::string& topic_id        = sample_topic.tid;
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
      std::string topic_datatype_encoding = sample_topic.tdatatype.encoding;
      std::string topic_datatype_name     = sample_topic.tdatatype.name;
      std::string topic_datatype_desc     = sample_topic.tdatatype.descriptor;
      auto        attr                    = sample_topic.attr;

      // try to get topic info
      const std::string topic_name_id  = topic_name + topic_id;
      Monitoring::STopicMon& TopicInfo = (*pTopicMap->map)[topic_name_id];

      // set static content
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
      TopicInfo.tdatatype.encoding   = std::move(topic_datatype_encoding);
      TopicInfo.tdatatype.name       = std::move(topic_datatype_name);
      TopicInfo.tdatatype.descriptor = std::move(topic_datatype_desc);

      // attributes
      TopicInfo.attr = std::map<std::string, std::string>{attr.begin(), attr.end()};

      // layer
      TopicInfo.tlayer.clear();
      // tlayer udp_mc
      {
        eCAL::Monitoring::TLayer tlayer;
        tlayer.type   = eCAL::Monitoring::tl_ecal_udp_mc;
        tlayer.active = topic_tlayer_ecal_udp;
        TopicInfo.tlayer.push_back(tlayer);
      }
      // tlayer shm
      {
        eCAL::Monitoring::TLayer tlayer;
        tlayer.type   = eCAL::Monitoring::tl_ecal_shm;
        tlayer.active = topic_tlayer_ecal_shm;
        TopicInfo.tlayer.push_back(tlayer);
      }
      // tlayer tcp
      {
        eCAL::Monitoring::TLayer tlayer;
        tlayer.type   = eCAL::Monitoring::tl_ecal_tcp;
        tlayer.active = topic_tlayer_ecal_tcp;
        TopicInfo.tlayer.push_back(tlayer);
      }

      TopicInfo.tsize           = static_cast<int>(topic_size);
      TopicInfo.connections_loc = static_cast<int>(connections_loc);
      TopicInfo.connections_ext = static_cast<int>(connections_ext);
      TopicInfo.did             = did;
      TopicInfo.dclock          = dclock;
      TopicInfo.message_drops   = message_drops;
      TopicInfo.dfreq           = dfreq;
    }

    return(true);
  }

  bool CMonitoringImpl::UnregisterTopic(const Registration::Sample& sample_, enum ePubSub pubsub_type_)
  {
    const auto& sample_topic = sample_.topic;
    const std::string& topic_name = sample_topic.tname;
    const std::string& topic_id   = sample_topic.tid;

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

  bool CMonitoringImpl::RegisterProcess(const Registration::Sample& sample_)
  {
    const auto& sample_process = sample_.process;
    const std::string&    host_name                    = sample_process.hname;
    const std::string&    host_group_name              = sample_process.hgname;
    const std::string&    process_name                 = sample_process.pname;
    const int             process_id                   = sample_process.pid;
    const std::string&    process_param                = sample_process.pparam;
    const std::string&    unit_name                    = sample_process.uname;
    const auto&           sample_process_state         = sample_process.state;
    const int             process_state_severity       = sample_process_state.severity;
    const int             process_state_severity_level = sample_process_state.severity_level;
    const std::string&    process_state_info           = sample_process_state.info;
    const int             process_tsync_state          = sample_process.tsync_state;
    const std::string&    process_tsync_mod_name       = sample_process.tsync_mod_name;
    const int             component_init_state         = sample_process.component_init_state;
    const std::string&    component_init_info          = sample_process.component_init_info;
    const std::string&    ecal_runtime_version         = sample_process.ecal_runtime_version;

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

  bool CMonitoringImpl::UnregisterProcess(const Registration::Sample& sample_)
  {
    const auto& sample_process = sample_.process;
    const std::string& process_name = sample_process.pname;
    const int          process_id   = sample_process.pid;

    // create map key
    const std::string process_name_id = process_name + std::to_string(process_id);

    // acquire access
    const std::lock_guard<std::mutex> lock(m_process_map.sync);

    // remove process info
    m_process_map.map->erase(process_name_id);

    return(true);
  }

  bool CMonitoringImpl::RegisterServer(const Registration::Sample& sample_)
  {
    const auto& sample_service = sample_.service;
    const std::string& host_name    = sample_service.hname;
    const std::string& service_name = sample_service.sname;
    const std::string& service_id   = sample_service.sid;
    const std::string& process_name = sample_service.pname;
    const std::string& unit_name    = sample_service.uname;
    const int32_t      process_id   = sample_service.pid;
    const uint32_t     tcp_port_v0  = sample_service.tcp_port_v0;
    const uint32_t     tcp_port_v1  = sample_service.tcp_port_v1;

    // create map key
    const std::string service_name_id = service_name + service_id + std::to_string(process_id);

    // acquire access
    const std::lock_guard<std::mutex> lock(m_server_map.sync);

    // try to get service info
    Monitoring::SServerMon& ServerInfo = (*m_server_map.map)[service_name_id];

    // set static content
    ServerInfo.hname       = host_name;
    ServerInfo.sname       = service_name;
    ServerInfo.sid         = service_id;
    ServerInfo.pname       = process_name;
    ServerInfo.uname       = unit_name;
    ServerInfo.pid         = process_id;
    ServerInfo.tcp_port_v0 = tcp_port_v0;
    ServerInfo.tcp_port_v1 = tcp_port_v1;

    // update flexible content
    ServerInfo.rclock++;
    ServerInfo.methods.clear();
    for (const auto& sample_service_method : sample_.service.methods)
    {
      struct Monitoring::SMethodMon method;
      method.mname      = sample_service_method.mname;
      method.req_type   = sample_service_method.req_type;
      method.req_desc   = sample_service_method.req_desc;
      method.resp_type  = sample_service_method.resp_type;
      method.resp_desc  = sample_service_method.resp_desc;
      method.call_count = sample_service_method.call_count;
      ServerInfo.methods.push_back(method);
    }

    return(true);
  }

  bool CMonitoringImpl::UnregisterServer(const Registration::Sample& sample_)
  {
    const auto& sample_service = sample_.service;
    const std::string& service_name = sample_service.sname;
    const std::string& service_id   = sample_service.sid;
    const int          process_id   = sample_service.pid;

    // create map key
    const std::string service_name_id = service_name + service_id + std::to_string(process_id);

    // acquire access
    const std::lock_guard<std::mutex> lock(m_server_map.sync);

    // remove service info
    m_server_map.map->erase(service_name_id);

    return(true);
  }

  bool CMonitoringImpl::RegisterClient(const Registration::Sample& sample_)
  {
    const auto& sample_client = sample_.client;
    const std::string& host_name    = sample_client.hname;
    const std::string& service_name = sample_client.sname;
    const std::string& service_id   = sample_client.sid;
    const std::string& process_name = sample_client.pname;
    const std::string& unit_name    = sample_client.uname;
    const int          process_id   = sample_client.pid;

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
    ClientInfo.methods.clear();
    for (const auto& sample_client_method : sample_.client.methods)
    {
      struct Monitoring::SMethodMon method;
      method.mname = sample_client_method.mname;
      method.req_type = sample_client_method.req_type;
      method.req_desc = sample_client_method.req_desc;
      method.resp_type = sample_client_method.resp_type;
      method.resp_desc = sample_client_method.resp_desc;
      method.call_count = sample_client_method.call_count;
      ClientInfo.methods.push_back(method);
    }

    return(true);
  }

  bool CMonitoringImpl::UnregisterClient(const Registration::Sample& sample_)
  {
    const auto& sample_client = sample_.client;
    const std::string& service_name = sample_client.sname;
    const std::string& service_id   = sample_client.sid;
    const int          process_id   = sample_client.pid;

    // create map key
    const std::string service_name_id = service_name + service_id + std::to_string(process_id);

    // acquire access
    const std::lock_guard<std::mutex> lock(m_clients_map.sync);

    // remove service info
    m_clients_map.map->erase(service_name_id);

    return(true);
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

  void CMonitoringImpl::GetMonitoring(std::string& monitoring_, unsigned int entities_)
  {
    // create monitoring struct
    Monitoring::SMonitoring monitoring;

    if ((entities_ & Monitoring::Entity::Process) != 0u)
    {
      MonitorProcs(monitoring);
    }

    if ((entities_ & Monitoring::Entity::Publisher) != 0u)
    {
      MonitorTopics(m_publisher_map, monitoring, "publisher");
    }

    if ((entities_ & Monitoring::Entity::Subscriber) != 0u)
    {
      MonitorTopics(m_subscriber_map, monitoring, "subscriber");
    }

    if ((entities_ & Monitoring::Entity::Server) != 0u)
    {
      MonitorServer(monitoring);
    }

    if ((entities_ & Monitoring::Entity::Client) != 0u)
    {
      MonitorClients(monitoring);
    }

    // serialize struct to target string
    SerializeToBuffer(monitoring, monitoring_);
  }

  void CMonitoringImpl::GetMonitoring(Monitoring::SMonitoring& monitoring_, unsigned int entities_)
  {
    // processes
    monitoring_.processes.clear();
    if ((entities_ & Monitoring::Entity::Process) != 0u)
    {
      // lock map
      const std::lock_guard<std::mutex> lock(m_process_map.sync);

      // reserve target
      monitoring_.processes.reserve(m_process_map.map->size());

      // iterate map
      m_process_map.map->erase_expired();
      for (const auto& process : (*m_process_map.map))
      {
        monitoring_.processes.emplace_back(process.second);
      }
    }

    // publisher
    monitoring_.publisher.clear();
    if ((entities_ & Monitoring::Entity::Publisher) != 0u)
    {
      // lock map
      const std::lock_guard<std::mutex> lock(m_publisher_map.sync);

      // reserve target
      monitoring_.publisher.reserve(m_publisher_map.map->size());

      // iterate map
      m_publisher_map.map->erase_expired();
      for (const auto& publisher : (*m_publisher_map.map))
      {
        monitoring_.publisher.emplace_back(publisher.second);
      }
    }

    // subscriber
    monitoring_.subscriber.clear();
    if ((entities_ & Monitoring::Entity::Subscriber) != 0u)
    {
      // lock map
      const std::lock_guard<std::mutex> lock(m_subscriber_map.sync);

      // reserve target
      monitoring_.subscriber.reserve(m_subscriber_map.map->size());

      // iterate map
      m_subscriber_map.map->erase_expired();
      for (const auto& subscriber : (*m_subscriber_map.map))
      {
        monitoring_.subscriber.emplace_back(subscriber.second);
      }
    }

    // server
    monitoring_.server.clear();
    if ((entities_ & Monitoring::Entity::Server) != 0u)
    {
      // lock map
      const std::lock_guard<std::mutex> lock(m_server_map.sync);

      // reserve target
      monitoring_.server.reserve(m_server_map.map->size());

      // iterate map
      m_server_map.map->erase_expired();
      for (const auto& server : (*m_server_map.map))
      {
        monitoring_.server.emplace_back(server.second);
      }
    }

    // clients
    monitoring_.clients.clear();
    if ((entities_ & Monitoring::Entity::Client) != 0u)
    {
      // lock map
      const std::lock_guard<std::mutex> lock(m_clients_map.sync);

      // reserve target
      monitoring_.clients.reserve(m_clients_map.map->size());

      // iterate map
      m_clients_map.map->erase_expired();
      for (const auto& client : (*m_clients_map.map))
      {
        monitoring_.clients.emplace_back(client.second);
      }
    }
  }

  void CMonitoringImpl::MonitorProcs(Monitoring::SMonitoring& monitoring_)
  {
    // acquire access
    const std::lock_guard<std::mutex> lock(m_process_map.sync);

    // iterate map
    m_process_map.map->erase_expired();
    for (const auto& process : (*m_process_map.map))
    {
      // add process
      monitoring_.processes.push_back(process.second);
    }
  }

  void CMonitoringImpl::MonitorServer(Monitoring::SMonitoring& monitoring_)
  {
    // acquire access
    const std::lock_guard<std::mutex> lock(m_server_map.sync);

    // iterate map
    m_server_map.map->erase_expired();
    for (const auto& server : (*m_server_map.map))
    {
      // add service
      monitoring_.server.push_back(server.second);
    }
  }

  void CMonitoringImpl::MonitorClients(Monitoring::SMonitoring& monitoring_)
  {
    // acquire access
    const std::lock_guard<std::mutex> lock(m_clients_map.sync);

    // iterate map
    m_clients_map.map->erase_expired();
    for (const auto& client : (*m_clients_map.map))
    {
      // add client
      monitoring_.clients.push_back(client.second);
    }
  }

  void CMonitoringImpl::MonitorTopics(STopicMonMap& map_, Monitoring::SMonitoring& monitoring_, const std::string& direction_)
  {
    // acquire access
    const std::lock_guard<std::mutex> lock(map_.sync);

    // iterate map
    map_.map->erase_expired();
    for (const auto& topic : (*map_.map))
    {
      if (direction_ == "publisher")
      {
        monitoring_.publisher.push_back(topic.second);
      }
      if (direction_ == "subscriber")
      {
        monitoring_.subscriber.push_back(topic.second);
      }
    }
  }

  void CMonitoringImpl::Tokenize(const std::string& str, StrICaseSetT& tokens, const std::string& delimiters, bool trimEmpty)
  {
    std::string::size_type pos     = 0;
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
