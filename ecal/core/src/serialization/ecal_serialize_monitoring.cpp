/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 * Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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
 * @file   ecal_serialize_monitoring.cpp
 * @brief  eCAL monitoring (de)serialization
**/

#include "ecal_serialize_common.h"
#include "ecal_serialize_monitoring.h"

#include <cstddef>
#include <iostream>
#include <vector>

#include <ecal/core/pb/monitoring.pbftags.h>
#include <ecal/core/pb/layer.pbftags.h>
#include <ecal/core/pb/process.pbftags.h>
#include <ecal/core/pb/service.pbftags.h>
#include <ecal/core/pb/topic.pbftags.h>
#include <protozero/pbf_writer.hpp>
#include <protozero/buffer_vector.hpp>
#include <protozero/pbf_reader.hpp>
#include <protozero/ecal_helper.h>


namespace{

  void DeserializeProcessState(protozero::pbf_reader& reader, eCAL::Monitoring::SProcess& target_sample_)
  {
    while (reader.next())
    {
      switch (reader.tag())
      {
      case +eCAL::pb::ProcessState::optional_enum_severity:
        target_sample_.state_severity = reader.get_int32();
        break;
      case +eCAL::pb::ProcessState::optional_enum_severity_level:
        target_sample_.state_severity_level = reader.get_int32();
        break;
      case +eCAL::pb::ProcessState::optional_string_info:
        AssignString(reader, target_sample_.state_info);
        break;
      }
    }
  }

  template <typename Writer>
  void SerializeProcess(Writer& writer_, const eCAL::Monitoring::SProcess& source_sample_)
  {
    writer_.add_int32(+eCAL::pb::Process::optional_int32_registration_clock, source_sample_.registration_clock);
    writer_.add_string(+eCAL::pb::Process::optional_string_host_name, source_sample_.host_name);
    writer_.add_string(+eCAL::pb::Process::optional_string_shm_transport_domain, source_sample_.shm_transport_domain);
    writer_.add_int32(+eCAL::pb::Process::optional_int32_process_id, source_sample_.process_id);
    writer_.add_string(+eCAL::pb::Process::optional_string_process_name, source_sample_.process_name);
    writer_.add_string(+eCAL::pb::Process::optional_string_unit_name, source_sample_.unit_name);
    writer_.add_string(+eCAL::pb::Process::optional_string_process_parameter, source_sample_.process_parameter);
    {
      Writer state_writer{ writer_, +eCAL::pb::Process::optional_message_state };
      state_writer.add_int32(+eCAL::pb::ProcessState::optional_enum_severity, source_sample_.state_severity);
      state_writer.add_int32(+eCAL::pb::ProcessState::optional_enum_severity_level, source_sample_.state_severity_level);
      state_writer.add_string(+eCAL::pb::ProcessState::optional_string_info, source_sample_.state_info);
    }
    writer_.add_enum(+eCAL::pb::Process::optional_enum_time_sync_state, static_cast<int>(source_sample_.time_sync_state));
    writer_.add_string(+eCAL::pb::Process::optional_string_time_sync_module_name, source_sample_.time_sync_module_name);
    writer_.add_int32(+eCAL::pb::Process::optional_int32_component_init_state, source_sample_.component_init_state);
    writer_.add_string(+eCAL::pb::Process::optional_string_component_init_info, source_sample_.component_init_info);
    writer_.add_string(+eCAL::pb::Process::optional_string_ecal_runtime_version, source_sample_.ecal_runtime_version);
    writer_.add_string(+eCAL::pb::Process::optional_string_config_file_path, source_sample_.config_file_path);
  }

  void DeserializeProcess(protozero::pbf_reader& reader, eCAL::Monitoring::SProcess& target_sample_)
  {
    while (reader.next())
    {
      switch (reader.tag())
      {
      case +eCAL::pb::Process::optional_int32_registration_clock:
        target_sample_.registration_clock = reader.get_int32();
        break;
      case +eCAL::pb::Process::optional_string_host_name:
        AssignString(reader, target_sample_.host_name);
        break;
      case +eCAL::pb::Process::optional_string_shm_transport_domain:
        AssignString(reader, target_sample_.shm_transport_domain);
        break;
      case +eCAL::pb::Process::optional_int32_process_id:
        target_sample_.process_id = reader.get_int32();
        break;
      case +eCAL::pb::Process::optional_string_process_name:
        AssignString(reader, target_sample_.process_name);
        break;
      case +eCAL::pb::Process::optional_string_unit_name:
        AssignString(reader, target_sample_.unit_name);
        break;
      case +eCAL::pb::Process::optional_string_process_parameter:
        AssignString(reader, target_sample_.process_parameter);
        break;
      case +eCAL::pb::Process::optional_message_state:
      {
        AssignMessage(reader, target_sample_, DeserializeProcessState);
        break;
      }
      case +eCAL::pb::Process::optional_enum_time_sync_state:
        target_sample_.time_sync_state = reader.get_enum();
        break;
      case +eCAL::pb::Process::optional_string_time_sync_module_name:
        AssignString(reader, target_sample_.time_sync_module_name);
        break;
      case +eCAL::pb::Process::optional_int32_component_init_state:
        target_sample_.component_init_state = reader.get_int32();
        break;
      case +eCAL::pb::Process::optional_string_component_init_info:
        AssignString(reader, target_sample_.component_init_info);
        break;
      case +eCAL::pb::Process::optional_string_ecal_runtime_version:
        AssignString(reader, target_sample_.ecal_runtime_version);
        break;
      case +eCAL::pb::Process::optional_string_config_file_path:
        AssignString(reader, target_sample_.config_file_path);
        break;
      default:
        reader.skip();
      }
    }
  }

  template <typename Writer>
  void SerializeTransportLayer(Writer& writer_, const eCAL::Monitoring::STransportLayer& source_sample_)
  {
    writer_.add_enum(+eCAL::pb::TransportLayer::optional_enum_type, static_cast<int>(source_sample_.type));
    writer_.add_int32(+eCAL::pb::TransportLayer::optional_int32_version, source_sample_.version);
    writer_.add_bool(+eCAL::pb::TransportLayer::optional_bool_active, source_sample_.active);
  } 

  void DeserializeTransportLayer(protozero::pbf_reader& reader_, eCAL::Monitoring::STransportLayer& target_sample_)
  {
    while (reader_.next())
    {
      switch (reader_.tag())
      {
      case +eCAL::pb::TransportLayer::optional_enum_type:
        target_sample_.type = static_cast<eCAL::Monitoring::eTransportLayerType>(reader_.get_enum());
        break;
      case +eCAL::pb::TransportLayer::optional_int32_version:
        target_sample_.version = reader_.get_int32();
        break;
      case +eCAL::pb::TransportLayer::optional_bool_active:
        target_sample_.active = reader_.get_bool();
        break;
      default:
        reader_.skip();
      }
    }
  }

  template <typename Writer>
  void SerializeStatistics(Writer& writer_, const eCAL::Monitoring::SStatistics& source_sample_)
  {
    writer_.add_uint64(+eCAL::pb::Statistics::optional_uint64_count, source_sample_.count);
    writer_.add_double(+eCAL::pb::Statistics::optional_double_min, source_sample_.min);
    writer_.add_double(+eCAL::pb::Statistics::optional_double_max, source_sample_.max);
    writer_.add_double(+eCAL::pb::Statistics::optional_double_mean, source_sample_.mean);
    writer_.add_double(+eCAL::pb::Statistics::optional_double_variance, source_sample_.variance);
  }

  void DeserializeStatistics(protozero::pbf_reader& reader_, eCAL::Monitoring::SStatistics& target_sample_)
  {
    while (reader_.next())
    {
      switch (reader_.tag())
      {
      case +eCAL::pb::Statistics::optional_uint64_count:
        target_sample_.count = reader_.get_int64();
        break;
      case +eCAL::pb::Statistics::optional_double_min:
        target_sample_.min = reader_.get_double();
        break;
      case +eCAL::pb::Statistics::optional_double_max:
        target_sample_.max = reader_.get_double();
        break;
      case +eCAL::pb::Statistics::optional_double_mean:
        target_sample_.mean = reader_.get_double();
        break;
      case +eCAL::pb::Statistics::optional_double_variance:
        target_sample_.variance = reader_.get_double();
        break;
      default:
        reader_.skip();
      }
    }
  }

  template <typename Writer>
  void SerializeTopic(Writer& writer_, const eCAL::Monitoring::STopic& source_sample_)
  {
    writer_.add_int32(+eCAL::pb::Topic::optional_int32_registration_clock, source_sample_.registration_clock);
    writer_.add_string(+eCAL::pb::Topic::optional_string_host_name, source_sample_.host_name);
    writer_.add_string(+eCAL::pb::Topic::optional_string_shm_transport_domain, source_sample_.shm_transport_domain);
    writer_.add_int32(+eCAL::pb::Topic::optional_int32_process_id, source_sample_.process_id);
    writer_.add_string(+eCAL::pb::Topic::optional_string_process_name, source_sample_.process_name);
    writer_.add_string(+eCAL::pb::Topic::optional_string_unit_name, source_sample_.unit_name);
    writer_.add_string(+eCAL::pb::Topic::optional_string_topic_id, std::to_string(source_sample_.topic_id));
    writer_.add_string(+eCAL::pb::Topic::optional_string_topic_name, source_sample_.topic_name);
    writer_.add_string(+eCAL::pb::Topic::optional_string_direction, source_sample_.direction);
    {
      Writer datatype_info_writer{ writer_, +eCAL::pb::Topic::optional_message_datatype_information };
      SerializeDataTypeInformation(datatype_info_writer, source_sample_.datatype_information);
    }
    for (const auto& layer : source_sample_.transport_layer)
    {
      Writer layer_writer{ writer_, +eCAL::pb::Topic::repeated_message_transport_layer };
      SerializeTransportLayer(layer_writer, layer);
    }
    writer_.add_int32(+eCAL::pb::Topic::optional_int32_topic_size, source_sample_.topic_size);
    writer_.add_int32(+eCAL::pb::Topic::optional_int32_connections_local, source_sample_.connections_local);
    writer_.add_int32(+eCAL::pb::Topic::optional_int32_connections_external, source_sample_.connections_external);
    writer_.add_int32(+eCAL::pb::Topic::optional_int32_message_drops, source_sample_.message_drops);
    writer_.add_int64(+eCAL::pb::Topic::optional_int64_data_id, source_sample_.data_id);
    writer_.add_int64(+eCAL::pb::Topic::optional_int64_data_clock, source_sample_.data_clock);
    writer_.add_int32(+eCAL::pb::Topic::optional_int32_data_frequency, source_sample_.data_frequency);
    {
      Writer latency_writer{ writer_, +eCAL::pb::Topic::optional_message_latency_us };
      SerializeStatistics(latency_writer, source_sample_.latency_us);
    }
  }

  void DeserializeTopic(protozero::pbf_reader& reader_, eCAL::Monitoring::STopic& target_sample_)
  {
    while (reader_.next())
    {
      switch (reader_.tag())
      {
      case +eCAL::pb::Topic::optional_int32_registration_clock:
        target_sample_.registration_clock = reader_.get_int32();
        break;
      case +eCAL::pb::Topic::optional_string_host_name:
        target_sample_.host_name = reader_.get_string();
        break;
      case +eCAL::pb::Topic::optional_string_shm_transport_domain:
        target_sample_.shm_transport_domain = reader_.get_string();
        break;
      case +eCAL::pb::Topic::optional_int32_process_id:
        target_sample_.process_id = reader_.get_int32();
        break;
      case +eCAL::pb::Topic::optional_string_process_name:
        target_sample_.process_name = reader_.get_string();
        break;
      case +eCAL::pb::Topic::optional_string_unit_name:
        target_sample_.unit_name = reader_.get_string();
        break;
      case +eCAL::pb::Topic::optional_string_topic_id:
        target_sample_.topic_id = std::stoull(reader_.get_string());
        break;
      case +eCAL::pb::Topic::optional_string_topic_name:
        target_sample_.topic_name =  reader_.get_string();
        break;
      case +eCAL::pb::Topic::optional_string_direction:
        target_sample_.direction = reader_.get_string();
        break;
      case +eCAL::pb::Topic::optional_message_datatype_information:
        AssignMessage(reader_, target_sample_.datatype_information, eCAL::protozero::DeserializeDataTypeInformation);
        break;
      case +eCAL::pb::Topic::repeated_message_transport_layer:
        AddRepeatedMessage(reader_, target_sample_.transport_layer, DeserializeTransportLayer);
        break;
      case +eCAL::pb::Topic::optional_int32_topic_size:
        target_sample_.topic_size = reader_.get_int32();
        break;
      case +eCAL::pb::Topic::optional_int32_connections_local:
        target_sample_.connections_local = reader_.get_int32();
        break;
      case +eCAL::pb::Topic::optional_int32_connections_external:
        target_sample_.connections_external = reader_.get_int32();
        break;  
      case +eCAL::pb::Topic::optional_int32_message_drops:
        target_sample_.message_drops = reader_.get_int32();
        break;
      case +eCAL::pb::Topic::optional_int64_data_id:
        target_sample_.data_id = reader_.get_int64();
        break;
      case +eCAL::pb::Topic::optional_int64_data_clock:
        target_sample_.data_clock = reader_.get_int64();
        break;
      case +eCAL::pb::Topic::optional_int32_data_frequency:
        target_sample_.data_frequency = reader_.get_int32();  
        break;
      case +eCAL::pb::Topic::optional_message_latency_us:
        AssignMessage(reader_, target_sample_.latency_us, DeserializeStatistics);
        break;
      default:
        reader_.skip();
      }
    }
  }


  template <typename Writer>
  void SerializeMethod(Writer& writer_, const eCAL::Monitoring::SMethod& method_)
  {
    writer_.add_string(+eCAL::pb::Method::optional_string_method_name, method_.method_name);
    {
      Writer req_dt_info_writer{ writer_, +eCAL::pb::Method::optional_message_request_datatype_information };
      SerializeDataTypeInformation(req_dt_info_writer, method_.request_datatype_information);
    }
    {
      Writer resp_dt_info_writer{ writer_, +eCAL::pb::Method::optional_message_response_datatype_information };
      SerializeDataTypeInformation(resp_dt_info_writer, method_.response_datatype_information);
    }
    writer_.add_int64(+eCAL::pb::Method::optional_int64_call_count, method_.call_count);
  }

  void DeserializeMethod(protozero::pbf_reader& reader_, eCAL::Monitoring::SMethod& method_)
  {
    while (reader_.next())
    {
      switch (reader_.tag())
      {
      case +eCAL::pb::Method::optional_string_method_name:
        method_.method_name = reader_.get_string();
        break;
      case +eCAL::pb::Method::optional_message_request_datatype_information:
        AssignMessage(reader_, method_.request_datatype_information, eCAL::protozero::DeserializeDataTypeInformation);
        break;
      case +eCAL::pb::Method::optional_message_response_datatype_information:
        AssignMessage(reader_, method_.response_datatype_information, eCAL::protozero::DeserializeDataTypeInformation);
        break;
      case +eCAL::pb::Method::optional_int64_call_count:
        method_.call_count = reader_.get_int64();
        break;
      default:
        reader_.skip();
      }
    }
  } 

  template <typename Writer>
  void SerializeServer(Writer& writer_, const eCAL::Monitoring::SServer& source_sample_)
  {
    writer_.add_int32(+eCAL::pb::Service::optional_int32_registration_clock, source_sample_.registration_clock);
    writer_.add_string(+eCAL::pb::Service::optional_string_host_name, source_sample_.host_name);
    writer_.add_string(+eCAL::pb::Service::optional_string_process_name, source_sample_.process_name);
    writer_.add_string(+eCAL::pb::Service::optional_string_unit_name, source_sample_.unit_name);
    writer_.add_int32(+eCAL::pb::Service::optional_int32_process_id, source_sample_.process_id);
    writer_.add_string(+eCAL::pb::Service::optional_string_service_name, source_sample_.service_name);
    writer_.add_string(+eCAL::pb::Service::optional_string_service_id, std::to_string(source_sample_.service_id));
    
    writer_.add_uint32(+eCAL::pb::Service::optional_uint32_version, source_sample_.version);
    writer_.add_uint32(+eCAL::pb::Service::optional_uint32_tcp_port_v0, source_sample_.tcp_port_v0);
    writer_.add_uint32(+eCAL::pb::Service::optional_uint32_tcp_port_v1, source_sample_.tcp_port_v1);

    for (const auto& method : source_sample_.methods)
    {
      Writer method_writer{ writer_, +eCAL::pb::Service::repeated_message_methods };
      SerializeMethod(method_writer, method);
    }
  }

  void DeserializeServer(protozero::pbf_reader& reader_, eCAL::Monitoring::SServer& target_sample_)
  {
    while (reader_.next())
    {
      switch (reader_.tag())
      {
      case +eCAL::pb::Service::optional_int32_registration_clock:
        target_sample_.registration_clock = reader_.get_int32();
        break;
      case +eCAL::pb::Service::optional_string_host_name:
        target_sample_.host_name = reader_.get_string();
        break;
      case +eCAL::pb::Service::optional_string_process_name:
        target_sample_.process_name = reader_.get_string();
        break;
      case +eCAL::pb::Service::optional_string_unit_name:
        target_sample_.unit_name = reader_.get_string();
        break;
      case +eCAL::pb::Service::optional_int32_process_id:
        target_sample_.process_id = reader_.get_int32();
        break;
      case +eCAL::pb::Service::optional_string_service_name:
        target_sample_.service_name = reader_.get_string();
        break;
      case +eCAL::pb::Service::optional_string_service_id:
        target_sample_.service_id = std::stoull(reader_.get_string());
        break;
      case +eCAL::pb::Service::optional_uint32_version:
        target_sample_.version = reader_.get_uint32();
        break;
      case +eCAL::pb::Service::optional_uint32_tcp_port_v0:
        target_sample_.tcp_port_v0 = reader_.get_uint32();
        break;
      case +eCAL::pb::Service::optional_uint32_tcp_port_v1:
        target_sample_.tcp_port_v1 = reader_.get_uint32();
        break;
      case +eCAL::pb::Service::repeated_message_methods:
        AddRepeatedMessage(reader_, target_sample_.methods, DeserializeMethod);
        break;
      default:
        reader_.skip();
      }
    }
  }

  template <typename Writer>
  void SerializeClient(Writer& writer_, const eCAL::Monitoring::SClient& source_sample_)
  {
    writer_.add_int32(+eCAL::pb::Client::optional_int32_registration_clock, source_sample_.registration_clock);
    writer_.add_string(+eCAL::pb::Client::optional_string_host_name, source_sample_.host_name);
    writer_.add_string(+eCAL::pb::Client::optional_string_process_name, source_sample_.process_name);
    writer_.add_string(+eCAL::pb::Client::optional_string_unit_name, source_sample_.unit_name);
    writer_.add_int32(+eCAL::pb::Client::optional_int32_process_id, source_sample_.process_id);
    writer_.add_string(+eCAL::pb::Client::optional_string_service_name, source_sample_.service_name);
    writer_.add_string(+eCAL::pb::Client::optional_string_service_id, std::to_string(source_sample_.service_id));
    
    for (const auto& method : source_sample_.methods)
    {
      Writer method_writer{ writer_, +eCAL::pb::Client::repeated_message_methods };
      SerializeMethod(method_writer, method);
    }

    writer_.add_uint32(+eCAL::pb::Client::optional_uint32_version, source_sample_.version);
  }

  void DeserializeClient(protozero::pbf_reader& reader_, eCAL::Monitoring::SClient& target_sample_)
  {
    while (reader_.next())
    {
      switch (reader_.tag())
      {
      case +eCAL::pb::Client::optional_int32_registration_clock:
        target_sample_.registration_clock = reader_.get_int32();
        break;
      case +eCAL::pb::Client::optional_string_host_name:
        target_sample_.host_name = reader_.get_string();
        break;
      case +eCAL::pb::Client::optional_string_process_name:
        target_sample_.process_name = reader_.get_string();
        break;
      case +eCAL::pb::Client::optional_string_unit_name:
        target_sample_.unit_name = reader_.get_string();
        break;
      case +eCAL::pb::Client::optional_int32_process_id:
        target_sample_.process_id = reader_.get_int32();
        break;
      case +eCAL::pb::Client::optional_string_service_name:
        target_sample_.service_name = reader_.get_string();
        break;
      case +eCAL::pb::Client::optional_string_service_id:
        target_sample_.service_id = std::stoull(reader_.get_string());
        break;
      case +eCAL::pb::Client::repeated_message_methods:
        AddRepeatedMessage(reader_, target_sample_.methods, DeserializeMethod);
        break;
      case +eCAL::pb::Client::optional_uint32_version:
        target_sample_.version = reader_.get_uint32();
        break;
      default:
        reader_.skip();
      }
    }
  }

  template <typename Writer>
  void SerializeMonitoring(Writer& writer_, const eCAL::Monitoring::SMonitoring& source_sample_)
  {
    for (const auto& process : source_sample_.processes)
    {
      Writer process_writer{ writer_, +eCAL::pb::Monitoring::repeated_message_processes };
      SerializeProcess(process_writer, process);
    }
    for (const auto& publisher : source_sample_.publishers)
    {
      Writer topic_writer{ writer_, +eCAL::pb::Monitoring::repeated_message_topics };
      SerializeTopic(topic_writer, publisher);
    }
    for (const auto& subscriber : source_sample_.subscribers)
    {
      Writer topic_writer{ writer_, +eCAL::pb::Monitoring::repeated_message_topics };
      SerializeTopic(topic_writer, subscriber);
    }
    for (const auto& server : source_sample_.servers)
    {
      Writer server_writer{ writer_, +eCAL::pb::Monitoring::repeated_message_services };
      SerializeServer(server_writer, server);
    }
    for (const auto& client : source_sample_.clients)
    {
      Writer client_writer{ writer_, +eCAL::pb::Monitoring::repeated_message_clients };
      SerializeClient(client_writer, client);
    }
  }

  void DeserializeMonitoring(::protozero::pbf_reader& reader_, eCAL::Monitoring::SMonitoring& target_sample_)
  {
    while (reader_.next())
    {
      switch (reader_.tag())
      {
      case +eCAL::pb::Monitoring::repeated_message_processes:
        AddRepeatedMessage(reader_, target_sample_.processes, DeserializeProcess);
        break;
      case +eCAL::pb::Monitoring::repeated_message_topics:
      {
        eCAL::Monitoring::STopic topic;
        auto topic_reader  = reader_.get_message();
        DeserializeTopic(topic_reader, topic);
        if (topic.direction == "publisher")
        {
          target_sample_.publishers.emplace_back(std::move(topic));
        }
        else
        {
          target_sample_.subscribers.emplace_back(std::move(topic));
        }
        break;
      }
      case +eCAL::pb::Monitoring::repeated_message_services:
        AddRepeatedMessage(reader_, target_sample_.servers, DeserializeServer);
        break;
      case +eCAL::pb::Monitoring::repeated_message_clients:
        AddRepeatedMessage(reader_, target_sample_.clients, DeserializeClient);
        break;
      default:
        reader_.skip();
      }
    }
  }
}

namespace eCAL
{
  namespace protozero
  {
    // monitoring - serialize/deserialize
    bool SerializeToBuffer(const Monitoring::SMonitoring& monitoring_, std::vector<char>& target_buffer_)
    {
      target_buffer_.clear();
      ::protozero::basic_pbf_writer<std::vector<char>> writer{ target_buffer_ };
      SerializeMonitoring(writer, monitoring_);
      return true;
    }

    bool SerializeToBuffer(const Monitoring::SMonitoring& monitoring_, std::string& target_buffer_)
    {
      target_buffer_.clear();
      ::protozero::pbf_writer writer{ target_buffer_ };
      SerializeMonitoring(writer, monitoring_);
      return true;
    }
  
    bool DeserializeFromBuffer(const char* data_, size_t size_, Monitoring::SMonitoring& monitoring_)
    {
      try
      {
        ::protozero::pbf_reader message{ data_, size_ };
        DeserializeMonitoring(message, monitoring_);
        return true;
      }
      catch (const std::exception& exception)
      {
        LogDeserializationException(exception, "eCAL::Monitoring::SMonitoring");
        return false;
      }
    }
  }
}
