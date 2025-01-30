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

#include <ecal/ecal.h>
#include <ecal/config.h>

#include "io/udp/ecal_udp_configurations.h"
#include "ecal_monitoring_impl.h"
#include "ecal_global_accessors.h"

#include "registration/ecal_registration_provider.h"
#include "registration/ecal_registration_receiver.h"

#include "serialization/ecal_serialize_monitoring.h"


namespace eCAL
{
  ////////////////////////////////////////
  // Monitoring Implementation
  ////////////////////////////////////////
  CMonitoringImpl::CMonitoringImpl() :
    m_init(false)
  {
  }

  void CMonitoringImpl::Create()
  {
    if (m_init) return;

    // utilize registration receiver to enrich monitor information
    g_registration_receiver()->SetCustomApplySampleCallback("monitoring", [this](const auto& sample_){this->ApplySample(sample_, tl_none);});
    m_init = true;
  }

  void CMonitoringImpl::Destroy()
  {
    // stop registration receiver utilization to enrich monitor information
    g_registration_receiver()->RemCustomApplySampleCallback("monitoring");
    m_init = false;
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
      Logging::Log(Logging::log_level_debug1, "CMonitoringImpl::ApplySample : unknown sample type");
    }
    break;
    }

    return true;
  }

  bool CMonitoringImpl::RegisterTopic(const Registration::Sample& sample_, enum ePubSub pubsub_type_)
  {
    const auto& sample_topic = sample_.topic;
    const int          process_id = sample_.identifier.process_id;
    const std::string& topic_name = sample_topic.topic_name;
    const int32_t      topic_size = sample_topic.topic_size;
    bool               topic_tlayer_ecal_udp(false);
    bool               topic_tlayer_ecal_shm(false);
    bool               topic_tlayer_ecal_tcp(false);
    for (const auto& layer : sample_topic.transport_layer)
    {
      topic_tlayer_ecal_udp |= (layer.type == tl_ecal_udp) && layer.active;
      topic_tlayer_ecal_shm |= (layer.type == tl_ecal_shm) && layer.active;
      topic_tlayer_ecal_tcp |= (layer.type == tl_ecal_tcp) && layer.active;
    }
    const int32_t      connections_local = sample_topic.connections_local;
    const int32_t      connections_external = sample_topic.connections_external;
    const int64_t      data_id = sample_topic.data_id;
    const int64_t      data_clock = sample_topic.data_clock;
    const int32_t      message_drops = sample_topic.message_drops;
    const int32_t      data_frequency = sample_topic.data_frequency;

    /////////////////////////////////
    // register in topic map
    /////////////////////////////////
    STopicMonMap* pTopicMap = GetMap(pubsub_type_);
    if (pTopicMap != nullptr)
    {
      // acquire access
      const std::lock_guard<std::mutex> lock(pTopicMap->sync);

      // common infos
      const std::string& host_name            = sample_.identifier.host_name;
      const std::string& shm_transport_domain = sample_topic.shm_transport_domain;
      const std::string& process_name         = sample_topic.process_name;
      const std::string& unit_name            = sample_topic.unit_name;
      const auto&        topic_id             = sample_.identifier.entity_id;
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
      std::string topic_datatype_encoding = sample_topic.datatype_information.encoding;
      std::string topic_datatype_name     = sample_topic.datatype_information.name;
      std::string topic_datatype_desc     = sample_topic.datatype_information.descriptor;

      // try to get topic info
      const auto& topic_map_key  = topic_id;
      Monitoring::STopicMon& TopicInfo = (*pTopicMap->map)[topic_map_key];

      // set static content
      TopicInfo.host_name            = host_name;
      TopicInfo.shm_transport_domain = shm_transport_domain;
      TopicInfo.process_id           = process_id;
      TopicInfo.process_name         = process_name;
      TopicInfo.unit_name            = unit_name;
      TopicInfo.topic_name           = topic_name;
      TopicInfo.direction            = direction;
      TopicInfo.topic_id             = topic_id;

      // update flexible content
      TopicInfo.registration_clock++;
      TopicInfo.datatype_information.encoding   = std::move(topic_datatype_encoding);
      TopicInfo.datatype_information.name       = std::move(topic_datatype_name);
      TopicInfo.datatype_information.descriptor = std::move(topic_datatype_desc);

      // layer
      TopicInfo.transport_layer.clear();
      // transport_layer udp_mc
      {
        eCAL::Monitoring::STransportLayer transport_layer;
        transport_layer.type   = eCAL::Monitoring::eTransportLayerType::udp_mc;
        transport_layer.active = topic_tlayer_ecal_udp;
        TopicInfo.transport_layer.push_back(transport_layer);
      }
      // transport_layer shm
      {
        eCAL::Monitoring::STransportLayer transport_layer;
        transport_layer.type   = eCAL::Monitoring::eTransportLayerType::shm;
        transport_layer.active = topic_tlayer_ecal_shm;
        TopicInfo.transport_layer.push_back(transport_layer);
      }
      // transport_layer tcp
      {
        eCAL::Monitoring::STransportLayer transport_layer;
        transport_layer.type   = eCAL::Monitoring::eTransportLayerType::tcp;
        transport_layer.active = topic_tlayer_ecal_tcp;
        TopicInfo.transport_layer.push_back(transport_layer);
      }

      TopicInfo.topic_size           = static_cast<int>(topic_size);
      TopicInfo.connections_local    = static_cast<int>(connections_local);
      TopicInfo.connections_external = static_cast<int>(connections_external);
      TopicInfo.data_id              = data_id;
      TopicInfo.data_clock           = data_clock;
      TopicInfo.message_drops        = message_drops;
      TopicInfo.data_frequency       = data_frequency;
    }

    return(true);
  }

  bool CMonitoringImpl::UnregisterTopic(const Registration::Sample& sample_, enum ePubSub pubsub_type_)
  {
    const auto& topic_map_key = sample_.identifier.entity_id;

    // unregister from topic map
    STopicMonMap* pTopicMap = GetMap(pubsub_type_);
    if (pTopicMap != nullptr)
    {
      // acquire access
      const std::lock_guard<std::mutex> lock(pTopicMap->sync);

      // remove topic info
      pTopicMap->map->erase(topic_map_key);
    }

    return(true);
  }

  bool CMonitoringImpl::RegisterProcess(const Registration::Sample& sample_)
  {
    const auto& sample_process = sample_.process;
    const std::string&    host_name                    = sample_.identifier.host_name;
    const std::string&    shm_transport_domain         = sample_process.shm_transport_domain;
    const std::string&    process_name                 = sample_process.process_name;
    const int             process_id                   = sample_.identifier.process_id;
    const std::string&    process_param                = sample_process.process_parameter;
    const std::string&    unit_name                    = sample_process.unit_name;
    const auto&           sample_process_state         = sample_process.state;
    const int             process_state_severity       = sample_process_state.severity;
    const int             process_state_severity_level = sample_process_state.severity_level;
    const std::string&    process_state_info           = sample_process_state.info;
    const int             process_time_sync_state      = sample_process.time_sync_state;
    const std::string&    process_tsync_mod_name       = sample_process.time_sync_module_name;
    const int             component_init_state         = sample_process.component_init_state;
    const std::string&    component_init_info          = sample_process.component_init_info;
    const std::string&    ecal_runtime_version         = sample_process.ecal_runtime_version;
    const std::string&    config_file_path             = sample_process.config_file_path;

    // create map key
    const auto& process_map_key = sample_.identifier.entity_id;

    // acquire access
    const std::lock_guard<std::mutex> lock(m_process_map.sync);

    // try to get process info
    Monitoring::SProcessMon& ProcessInfo = (*m_process_map.map)[process_map_key];

    // set static content
    ProcessInfo.host_name            = host_name;
    ProcessInfo.shm_transport_domain = shm_transport_domain;
    ProcessInfo.process_name         = process_name;
    ProcessInfo.unit_name            = unit_name;
    ProcessInfo.process_id           = process_id;
    ProcessInfo.process_parameter               = process_param;

    // update flexible content
    ProcessInfo.registration_clock++;
    ProcessInfo.state_severity        = process_state_severity;
    ProcessInfo.state_severity_level  = process_state_severity_level;
    ProcessInfo.state_info            = process_state_info;
    ProcessInfo.time_sync_state       = process_time_sync_state;
    ProcessInfo.time_sync_module_name = process_tsync_mod_name;
    ProcessInfo.component_init_state  = component_init_state;
    ProcessInfo.component_init_info   = component_init_info;
    ProcessInfo.ecal_runtime_version  = ecal_runtime_version;
    ProcessInfo.config_file_path      = config_file_path;

    return(true);
  }

  bool CMonitoringImpl::UnregisterProcess(const Registration::Sample& sample_)
  {
    const auto& process_map_key = sample_.identifier.entity_id;

    // acquire access
    const std::lock_guard<std::mutex> lock(m_process_map.sync);

    // remove process info
    m_process_map.map->erase(process_map_key);

    return(true);
  }

  bool CMonitoringImpl::RegisterServer(const Registration::Sample& sample_)
  {
    const auto& sample_identifier = sample_.identifier;
    const auto&        service_id = sample_.identifier.entity_id;
    const int32_t      process_id = sample_identifier.process_id;
    const std::string& host_name  = sample_identifier.host_name;

    const auto& sample_service = sample_.service;
    const std::string& service_name = sample_service.service_name;
    const std::string& process_name = sample_service.process_name;
    const std::string& unit_name    = sample_service.unit_name;

    const uint32_t     tcp_port_v0  = sample_service.tcp_port_v0;
    const uint32_t     tcp_port_v1  = sample_service.tcp_port_v1;

    // create map key
    const auto& service_map_key = service_id;

    // acquire access
    const std::lock_guard<std::mutex> lock(m_server_map.sync);

    // try to get service info
    Monitoring::SServerMon& ServerInfo = (*m_server_map.map)[service_map_key];

    // set static content
    ServerInfo.host_name    = host_name;
    ServerInfo.service_name        = service_name;
    ServerInfo.service_id          = service_id;
    ServerInfo.process_name = process_name;
    ServerInfo.unit_name    = unit_name;
    ServerInfo.process_id   = process_id;
    ServerInfo.tcp_port_v0  = tcp_port_v0;
    ServerInfo.tcp_port_v1  = tcp_port_v1;

    // update flexible content
    ServerInfo.registration_clock++;
    ServerInfo.methods.clear();
    for (const auto& sample_service_method : sample_.service.methods)
    {
      struct Monitoring::SMethodMon method;
      method.method_name   = sample_service_method.method_name;

      method.request_datatype_information  = sample_service_method.request_datatype_information;
      method.response_datatype_information = sample_service_method.response_datatype_information;

      method.call_count    = sample_service_method.call_count;
      ServerInfo.methods.push_back(method);
    }

    return(true);
  }

  bool CMonitoringImpl::UnregisterServer(const Registration::Sample& sample_)
  {
    // create map key
    const auto& service_map_key = sample_.identifier.entity_id;

    // acquire access
    const std::lock_guard<std::mutex> lock(m_server_map.sync);

    // remove service info
    m_server_map.map->erase(service_map_key);

    return(true);
  }

  bool CMonitoringImpl::RegisterClient(const Registration::Sample& sample_)
  {
    const auto& sample_identifier = sample_.identifier;
    const auto&        service_id = sample_identifier.entity_id;
    const int32_t      process_id = sample_identifier.process_id;
    const std::string& host_name  = sample_identifier.host_name;

    const auto& sample_client = sample_.client;
    const std::string& service_name = sample_client.service_name;
    const std::string& process_name = sample_client.process_name;
    const std::string& unit_name    = sample_client.unit_name;

    // create map key
    const auto& client_map_key = service_id;

    // acquire access
    const std::lock_guard<std::mutex> lock(m_clients_map.sync);

    // try to get service info
    Monitoring::SClientMon& ClientInfo = (*m_clients_map.map)[client_map_key];

    // set static content
    ClientInfo.host_name    = host_name;
    ClientInfo.service_name        = service_name;
    ClientInfo.service_id          = service_id;
    ClientInfo.process_name = process_name;
    ClientInfo.unit_name    = unit_name;
    ClientInfo.process_id   = process_id;

    // update flexible content
    ClientInfo.registration_clock++;
    ClientInfo.methods.clear();
    for (const auto& sample_client_method : sample_.client.methods)
    {
      struct Monitoring::SMethodMon method;
      method.method_name = sample_client_method.method_name;
      method.request_datatype_information = sample_client_method.request_datatype_information;
      method.response_datatype_information = sample_client_method.response_datatype_information;
      method.call_count = sample_client_method.call_count;
      ClientInfo.methods.push_back(method);
    }

    return(true);
  }

  bool CMonitoringImpl::UnregisterClient(const Registration::Sample& sample_)
  {
    // create map key
    const auto& client_map_key = sample_.identifier.entity_id;

    // acquire access
    const std::lock_guard<std::mutex> lock(m_clients_map.sync);

    // remove service info
    m_clients_map.map->erase(client_map_key);

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
}
