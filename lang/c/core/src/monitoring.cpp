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
 * @file   monitoring.cpp
 * @brief  eCAL monitoring c interface
**/

#include <ecal/ecal.h>
#include <ecal_c/monitoring.h>
#include <ecal_c/types/monitoring.h>

#include "common.h"

#include <map>
#include <cassert>
#include <numeric>

namespace
{
  unsigned int Convert_Entities(unsigned int entities_c_)
  {
    unsigned int entities{ 0 };
    static const std::map<unsigned int, unsigned int> entity_map
    {
        {eCAL_Monitoring_Entity_None, eCAL::Monitoring::Entity::None},
        {eCAL_Monitoring_Entity_Publisher, eCAL::Monitoring::Entity::Publisher},
        {eCAL_Monitoring_Entity_Subscriber, eCAL::Monitoring::Entity::Subscriber},
        {eCAL_Monitoring_Entity_Server, eCAL::Monitoring::Entity::Server},
        {eCAL_Monitoring_Entity_Client, eCAL::Monitoring::Entity::Client},
        {eCAL_Monitoring_Entity_Process, eCAL::Monitoring::Entity::Process},
        {eCAL_Monitoring_Entity_Host, eCAL::Monitoring::Entity::Host},
    };

    decltype(entities_c_) bit_mask = 1 << 0;
    for (std::size_t i = 0; i < sizeof(decltype(entities_c_)) * 8; ++i)
    {
      entities |= entity_map.at(bit_mask & entities_c_);
      bit_mask <<= 1;
    }

    return entities;
  }

  size_t ExtSize_Monitoring_SProcessMon(const eCAL::Monitoring::SProcessMon& process_)
  {
    return ExtSize_String(process_.component_init_info) +
      ExtSize_String(process_.config_file_path) +
      ExtSize_String(process_.ecal_runtime_version) +
      ExtSize_String(process_.host_name) +
      ExtSize_String(process_.process_name) +
      ExtSize_String(process_.process_parameter) +
      ExtSize_String(process_.shm_transport_domain) +
      ExtSize_String(process_.state_info) +
      ExtSize_String(process_.time_sync_module_name) +
      ExtSize_String(process_.unit_name);
  }

  size_t ExtSize_Monitoring_SProcessMonArray(const std::vector<eCAL::Monitoring::SProcessMon>& processes_)
  {
    return aligned_size(sizeof(struct eCAL_Monitoring_SProcessMon) * processes_.size());
  }

  size_t ExtSize_Monitoring_STransportLayerArray(const std::vector<eCAL::Monitoring::STransportLayer>& transport_layers_)
  {
    return aligned_size(sizeof(struct eCAL_Monitoring_STransportLayer) * transport_layers_.size());
  }

  size_t ExtSize_Monitoring_STopicMon(const eCAL::Monitoring::STopicMon& topic_)
  {
    return ExtSize_SDataTypeInformation(topic_.datatype_information) +
      ExtSize_String(topic_.direction) +
      ExtSize_String(topic_.host_name) +
      ExtSize_String(topic_.process_name) +
      ExtSize_String(topic_.shm_transport_domain) +
      ExtSize_String(topic_.topic_name) +
      ExtSize_String(topic_.unit_name) +
      ExtSize_Monitoring_STransportLayerArray(topic_.transport_layer);
  }

  size_t ExtSize_Monitoring_STopicMonArray(const std::vector<eCAL::Monitoring::STopicMon>& topics_)
  {
    return aligned_size(sizeof(struct eCAL_Monitoring_STopicMon) * topics_.size());
  }

  size_t ExtSize_Monitoring_SServerMonArray(const std::vector<eCAL::Monitoring::SServerMon>& servers_)
  {
    return aligned_size(sizeof(struct eCAL_Monitoring_SServerMon) * servers_.size());
  }

  size_t ExtSize_Monitoring_SMethodMon(const eCAL::Monitoring::SMethodMon& method_)
  {
    return ExtSize_String(method_.method_name) +
      ExtSize_SDataTypeInformation(method_.request_datatype_information) +
      ExtSize_SDataTypeInformation(method_.response_datatype_information);
  }

  size_t ExtSize_Monitoring_SMethodMonArray(const std::vector<eCAL::Monitoring::SMethodMon>& methods_)
  {
    return aligned_size(sizeof(struct eCAL_Monitoring_SMethodMon) * methods_.size());
  }

  size_t ExtSize_Monitoring_SServerMon(const eCAL::Monitoring::SServerMon& server_)
  {
    return ExtSize_String(server_.host_name) +
      ExtSize_String(server_.process_name) +
      ExtSize_String(server_.service_name) +
      ExtSize_String(server_.unit_name) +
      ExtSize_Monitoring_SMethodMonArray(server_.methods) +
      std::accumulate(server_.methods.begin(), server_.methods.end(), size_t{ 0 },
        [](auto size_, const auto& method_) {
          return size_ + ExtSize_Monitoring_SMethodMon(method_);
        });
  }

  size_t ExtSize_Monitoring_SClientMonArray(const std::vector<eCAL::Monitoring::SClientMon>& clients_)
  {
    return aligned_size(sizeof(struct eCAL_Monitoring_SClientMon) * clients_.size());
  }

  size_t ExtSize_Monitoring_SClientMon(const eCAL::Monitoring::SClientMon& client_)
  {
     return ExtSize_String(client_.host_name) +
       ExtSize_String(client_.process_name) +
       ExtSize_String(client_.service_name) +
       ExtSize_String(client_.unit_name) +
       ExtSize_Monitoring_SMethodMonArray(client_.methods) +
       std::accumulate(client_.methods.begin(), client_.methods.end(), size_t{ 0 },
         [](auto size_, const auto& method_) {
           return size_ + ExtSize_Monitoring_SMethodMon(method_);
         });
  }

  size_t ExtSize_Monitoring_SMonitoring(const eCAL::Monitoring::SMonitoring& monitoring_)
  {
    return ExtSize_Monitoring_SProcessMonArray(monitoring_.processes)
      + std::accumulate(monitoring_.processes.begin(), monitoring_.processes.end(), size_t{ 0 },
        [](auto size_, const auto& process_) {
          return size_ + ExtSize_Monitoring_SProcessMon(process_);
        })
      + ExtSize_Monitoring_STopicMonArray(monitoring_.publisher)
      + std::accumulate(monitoring_.publisher.begin(), monitoring_.publisher.end(), size_t{ 0 },
        [](auto size_, const auto& publisher_) {
          return size_ + ExtSize_Monitoring_STopicMon(publisher_);
        })
      + ExtSize_Monitoring_STopicMonArray(monitoring_.subscriber)
      + std::accumulate(monitoring_.subscriber.begin(), monitoring_.subscriber.end(), size_t{ 0 },
        [](auto size_, const auto& subscriber_) {
          return size_ + ExtSize_Monitoring_STopicMon(subscriber_);
        })
      + ExtSize_Monitoring_SServerMonArray(monitoring_.server)
      + std::accumulate(monitoring_.server.begin(), monitoring_.server.end(), size_t{ 0 },
        [](auto size_, const auto& server_) {
          return size_ + ExtSize_Monitoring_SServerMon(server_);
        })
      + ExtSize_Monitoring_SClientMonArray(monitoring_.clients);
  }

  void Assign_Monitoring_SProcessMon(struct eCAL_Monitoring_SProcessMon* process_c_, const eCAL::Monitoring::SProcessMon& process_, char** offset_)
  {
    process_c_->component_init_info = Convert_String(process_.component_init_info, offset_);
    process_c_->component_init_state = process_.component_init_state;
    process_c_->config_file_path = Convert_String(process_.config_file_path, offset_);
    process_c_->ecal_runtime_version = Convert_String(process_.ecal_runtime_version, offset_);
    process_c_->host_name = Convert_String(process_.host_name, offset_);
    process_c_->process_id = process_.process_id;
    process_c_->process_name = Convert_String(process_.process_name, offset_);
    process_c_->process_parameter = Convert_String(process_.process_parameter, offset_);
    process_c_->registration_clock = process_.registration_clock;
    process_c_->shm_transport_domain = Convert_String(process_.shm_transport_domain, offset_);
    process_c_->state_info = Convert_String(process_.state_info, offset_);
    process_c_->state_severity = process_.state_severity;
    process_c_->state_severity_level = process_.state_severity_level;
    process_c_->time_sync_module_name = Convert_String(process_.time_sync_module_name, offset_);
    process_c_->time_sync_state = process_.time_sync_state;
    process_c_->unit_name = Convert_String(process_.unit_name, offset_);
  }

  void Assign_Monitoring_STransportLayer(struct eCAL_Monitoring_STransportLayer* transport_layer_c_, const eCAL::Monitoring::STransportLayer& transport_layer_)
  {
    static const std::map<eCAL::Monitoring::eTransportLayerType, eCAL_Monitoring_eTransportLayerType> transport_layer_type_map
    {
      {eCAL::Monitoring::eTransportLayerType::none, eCAL_Monitoring_eTransportLayerType_none},
      {eCAL::Monitoring::eTransportLayerType::udp_mc, eCAL_Monitoring_eTransportLayerType_udp_mc},
      {eCAL::Monitoring::eTransportLayerType::shm, eCAL_Monitoring_eTransportLayerType_shm},
      {eCAL::Monitoring::eTransportLayerType::tcp, eCAL_Monitoring_eTransportLayerType_tcp}
    };

    transport_layer_c_->active = transport_layer_.active;
    transport_layer_c_->type = transport_layer_type_map.at(transport_layer_.type);
    transport_layer_c_->version = transport_layer_.version;
  }

  void Assign_Monitoring_STransportLayerArray(struct eCAL_Monitoring_STransportLayer** transport_layers_c_, const std::vector<eCAL::Monitoring::STransportLayer>& transport_layers_, char** offset_)
  {
    *transport_layers_c_ = reinterpret_cast<eCAL_Monitoring_STransportLayer*>(*offset_);
    *offset_ += ExtSize_Monitoring_STransportLayerArray(transport_layers_);
    for (size_t i = 0; i < transport_layers_.size(); ++i)
    {
      Assign_Monitoring_STransportLayer(&((*transport_layers_c_)[i]), transport_layers_.at(i));
    }
  }

  void Assign_Monitoring_STopicMon(struct eCAL_Monitoring_STopicMon* topic_c_, const eCAL::Monitoring::STopicMon& topic_, char** offset_)
  {
    topic_c_->registration_clock = topic_.registration_clock;
    topic_c_->host_name = Convert_String(topic_.host_name, offset_);
    topic_c_->shm_transport_domain = Convert_String(topic_.shm_transport_domain, offset_);
    topic_c_->process_id = topic_.process_id;
    topic_c_->process_name = Convert_String(topic_.process_name, offset_);
    topic_c_->unit_name = Convert_String(topic_.unit_name, offset_);
    topic_c_->topic_id = topic_.topic_id;
    topic_c_->topic_name = Convert_String(topic_.topic_name, offset_);
    topic_c_->direction = Convert_String(topic_.direction, offset_);
    Assign_SDataTypeInformation(&topic_c_->datatype_information, topic_.datatype_information, offset_);
    Assign_Monitoring_STransportLayerArray(&(topic_c_->transport_layer), topic_.transport_layer, offset_);
    topic_c_->transport_layer_length = topic_.transport_layer.size();
    topic_c_->topic_size = topic_.topic_size;
    topic_c_->connections_local = topic_.connections_local;
    topic_c_->connections_external = topic_.connections_external;
    topic_c_->message_drops = topic_.message_drops;
    topic_c_->data_id = topic_.data_id;
    topic_c_->data_clock = topic_.data_clock;
    topic_c_->data_frequency = topic_.data_frequency;
  }

  void Assign_Monitoring_SMethodMon(struct eCAL_Monitoring_SMethodMon* method_c_, const eCAL::Monitoring::SMethodMon& method_, char** offset_)
  {
    method_c_->call_count = method_.call_count;
    method_c_->method_name = Convert_String(method_.method_name, offset_);
    Assign_SDataTypeInformation(&(method_c_->request_datatype_information), method_.request_datatype_information, offset_);
    Assign_SDataTypeInformation(&(method_c_->response_datatype_information), method_.response_datatype_information, offset_);
  }

  void Assign_Monitoring_SMethodMonArray(struct eCAL_Monitoring_SMethodMon** methods_c_, const std::vector<eCAL::Monitoring::SMethodMon>& methods_, char** offset_)
  {
    *methods_c_ = reinterpret_cast<eCAL_Monitoring_SMethodMon*>(*offset_);
    *offset_ += ExtSize_Monitoring_SMethodMonArray(methods_);
    for (size_t i = 0; i < methods_.size(); ++i)
    {
      Assign_Monitoring_SMethodMon(&((*methods_c_)[i]), methods_.at(i), offset_);
    }
  }

  void Assign_Monitoring_SServerMon(struct eCAL_Monitoring_SServerMon* server_c_, const eCAL::Monitoring::SServerMon& server_, char** offset_)
  {
    server_c_->host_name = Convert_String(server_.host_name, offset_);
    Assign_Monitoring_SMethodMonArray(&(server_c_->methods), server_.methods, offset_);
    server_c_->methods_length = server_.methods.size();
    server_c_->process_id = server_.process_id;
    server_c_->process_name = Convert_String(server_.process_name, offset_);
    server_c_->registration_clock = server_.registration_clock;
    server_c_->service_id = server_.service_id;
    server_c_->service_name = Convert_String(server_.service_name, offset_);
    server_c_->tcp_port_v0 = server_.tcp_port_v0;
    server_c_->tcp_port_v1 = server_.tcp_port_v1;
    server_c_->unit_name = Convert_String(server_.unit_name, offset_);
    server_c_->version = server_.version;
  }

  void Assign_Monitoring_SClientMon(struct eCAL_Monitoring_SClientMon* client_c_, const eCAL::Monitoring::SClientMon& client_, char** offset_)
  {
    client_c_->host_name = Convert_String(client_.host_name, offset_);
    Assign_Monitoring_SMethodMonArray(&(client_c_->methods), client_.methods, offset_);
    client_c_->methods_length = client_.methods.size();
    client_c_->process_id = client_.process_id;
    client_c_->process_name = Convert_String(client_.process_name, offset_);
    client_c_->registration_clock = client_.registration_clock;
    client_c_->service_id = client_.service_id;
    client_c_->service_name = Convert_String(client_.service_name, offset_);
    client_c_->unit_name = Convert_String(client_.unit_name, offset_);
    client_c_->version = client_.version;
  }

  void Assign_Monitoring_SProcessMonArray(struct eCAL_Monitoring_SProcessMon** processes_c_, const std::vector<eCAL::Monitoring::SProcessMon>& processes_, char** offset_)
  {
    *processes_c_ = reinterpret_cast<eCAL_Monitoring_SProcessMon*>(*offset_);
    *offset_ += ExtSize_Monitoring_SProcessMonArray(processes_);
    for (size_t i = 0; i < processes_.size(); ++i)
    {
      Assign_Monitoring_SProcessMon(&((*processes_c_)[i]), processes_.at(i), offset_);
    }
  }

  void Assign_Monitoring_STopicMonArray(struct eCAL_Monitoring_STopicMon** topics_c_, const std::vector<eCAL::Monitoring::STopicMon>& topics_, char** offset_)
  {
    *topics_c_ = reinterpret_cast<eCAL_Monitoring_STopicMon*>(*offset_);
    *offset_ += ExtSize_Monitoring_STopicMonArray(topics_);
    for (size_t i = 0; i < topics_.size(); ++i)
    {
      Assign_Monitoring_STopicMon(&((*topics_c_)[i]), topics_.at(i), offset_);
    }
  }

  void Assign_Monitoring_SServerMonArray(struct eCAL_Monitoring_SServerMon** servers_c_, const std::vector<eCAL::Monitoring::SServerMon>& servers_, char** offset_)
  {
    *servers_c_ = reinterpret_cast<eCAL_Monitoring_SServerMon*>(*offset_);
    *offset_ += ExtSize_Monitoring_SServerMonArray(servers_);
    for (size_t i = 0; i < servers_.size(); ++i)
    {
      Assign_Monitoring_SServerMon(&((*servers_c_)[i]), servers_.at(i), offset_);
    }
  }

  void Assign_Monitoring_SClientMonArray(struct eCAL_Monitoring_SClientMon** clients_c_, const std::vector<eCAL::Monitoring::SClientMon>& clients_, char** offset_)
  {
    *clients_c_ = reinterpret_cast<eCAL_Monitoring_SClientMon*>(*offset_);
    *offset_ += ExtSize_Monitoring_SClientMonArray(clients_);
    for (size_t i = 0; i < clients_.size(); ++i)
    {
      Assign_Monitoring_SClientMon(&((*clients_c_)[i]), clients_.at(i), offset_);
    }
  }

  void Assign_Monitoring_SMonitoring(struct eCAL_Monitoring_SMonitoring* monitoring_c_, const eCAL::Monitoring::SMonitoring& monitoring_, char** offset_)
  {
    monitoring_c_->processes_length = monitoring_.processes.size();
    Assign_Monitoring_SProcessMonArray(&(monitoring_c_->processes), monitoring_.processes, offset_);
    
    monitoring_c_->publisher_length = monitoring_.publisher.size();
    Assign_Monitoring_STopicMonArray(&(monitoring_c_->publisher), monitoring_.publisher, offset_);

    monitoring_c_->subscriber_length = monitoring_.subscriber.size();
    Assign_Monitoring_STopicMonArray(&(monitoring_c_->subscriber), monitoring_.subscriber, offset_);

    monitoring_c_->server_length = monitoring_.server.size();
    Assign_Monitoring_SServerMonArray(&(monitoring_c_->server), monitoring_.server, offset_);

    monitoring_c_->clients_length = monitoring_.clients.size();
    Assign_Monitoring_SClientMonArray(&(monitoring_c_->clients), monitoring_.clients, offset_);
  }
}

#if ECAL_CORE_MONITORING
extern "C"
{
  ECALC_API int eCAL_Monitoring_GetMonitoringBuffer(void** monitoring_buffer_, size_t* monitoring_buffer_length_, const unsigned int* entities_)
  {
    assert(monitoring_buffer_ != NULL && monitoring_buffer_length_ != NULL);
    assert(*monitoring_buffer_ == NULL && *monitoring_buffer_length_ == 0);

    std::string buffer;
    if (eCAL::Monitoring::GetMonitoring(buffer, entities_ != NULL ? Convert_Entities(*entities_) : eCAL::Monitoring::Entity::All))
    {
      *monitoring_buffer_ = std::malloc(buffer.size());
      if (*monitoring_buffer_ != NULL)
      {
        std::memcpy(*monitoring_buffer_, buffer.data(), buffer.size());
        *monitoring_buffer_length_ = buffer.size();
      }
    }
    return !static_cast<int>(*monitoring_buffer_ != NULL);
  }

  ECALC_API int eCAL_Monitoring_GetMonitoring(struct eCAL_Monitoring_SMonitoring** monitoring_, const unsigned int* entities_)
  {
    assert(monitoring_ != NULL);
    assert(*monitoring_ == NULL);

    eCAL::Monitoring::SMonitoring monitoring;
    if (eCAL::Monitoring::GetMonitoring(monitoring, entities_ != NULL ? Convert_Entities(*entities_) : eCAL::Monitoring::Entity::All))
    {
      const auto base_size = aligned_size(sizeof(struct eCAL_Monitoring_SMonitoring));
      const auto extented_size = ExtSize_Monitoring_SMonitoring(monitoring);
      *monitoring_ = reinterpret_cast<struct eCAL_Monitoring_SMonitoring*>(std::malloc(base_size + extented_size));
      if (*monitoring_ != NULL)
      {
        auto* offset = reinterpret_cast<char*>(*monitoring_) + base_size;
        Assign_Monitoring_SMonitoring(*monitoring_, monitoring, &offset);
      }
    }

    return !static_cast<int>(*monitoring_ != NULL);
  }
}
#endif // ECAL_CORE_MONITORING
