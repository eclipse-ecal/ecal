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

#include "nanopb/pb_encode.h"
#include "nanopb/pb_decode.h"
#include "nanopb/ecal/core/pb/monitoring.npb.h"

#include "ecal_serialize_common.h"
#include "ecal_serialize_monitoring.h"

#include <cstddef>
#include <iostream>
#include <vector>

namespace
{
  /////////////////////////////////////////////////////////////////////////////////
  // Encode: eCAL::Monitoring::SProcessMon
  /////////////////////////////////////////////////////////////////////////////////
  void PrepareEncoding(const eCAL::Monitoring::SProcess& process_, eCAL_pb_Process& pb_process_)
  {
    ///////////////////////////////////////////////
    // process information
    ///////////////////////////////////////////////
    // registration_clock
    pb_process_.registration_clock = process_.registration_clock;
    // host_name
    eCAL::nanopb::encode_string(pb_process_.host_name, process_.host_name);
    // shm_transport_domain
    eCAL::nanopb::encode_string(pb_process_.shm_transport_domain, process_.shm_transport_domain);
    // process_id
    pb_process_.process_id = process_.process_id;
    // process_name
    eCAL::nanopb::encode_string(pb_process_.process_name, process_.process_name);
    // unit_name
    eCAL::nanopb::encode_string(pb_process_.unit_name, process_.unit_name);
    // process_parameter
    eCAL::nanopb::encode_string(pb_process_.process_parameter, process_.process_parameter);
    // state
    pb_process_.has_state = true;
    // state.severity
    pb_process_.state.severity = static_cast<eCAL_pb_eProcessSeverity>(process_.state_severity);
    // state.severity_level
    pb_process_.state.severity_level = static_cast<eCAL_pb_eProcessSeverityLevel>(process_.state_severity_level);
    // state.info
    eCAL::nanopb::encode_string(pb_process_.state.info, process_.state_info);
    // process.time_sync_state
    pb_process_.time_sync_state = static_cast<eCAL_pb_eTimeSyncState>(process_.time_sync_state);
    // time_sync_module_name
    eCAL::nanopb::encode_string(pb_process_.time_sync_module_name, process_.time_sync_module_name);
    // component_init_state
    pb_process_.component_init_state = process_.component_init_state;
    // component_init_info
    eCAL::nanopb::encode_string(pb_process_.component_init_info, process_.component_init_info);
    // ecal_runtime_version
    eCAL::nanopb::encode_string(pb_process_.ecal_runtime_version, process_.ecal_runtime_version);
    // config_file_path
    eCAL::nanopb::encode_string(pb_process_.config_file_path, process_.config_file_path);
  }

  bool encode_mon_message_processes_field(pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    auto* process_vec = static_cast<std::vector<eCAL::Monitoring::SProcess>*>(*arg);

    for (const auto& process : *process_vec)
    {
      // encode process tag
      if (!pb_encode_tag_for_field(stream, field))
      {
        return false;
      }

      // encode single process
      eCAL_pb_Process pb_process = eCAL_pb_Process_init_default;
      PrepareEncoding(process, pb_process);

      // encode submessage
      if (!pb_encode_submessage(stream, eCAL_pb_Process_fields, &pb_process))
      {
        return false;
      }
    }

    return true;
  }

  /////////////////////////////////////////////////////////////////////////////////
  // Encode: eCAL::Monitoring::STopicMon
  /////////////////////////////////////////////////////////////////////////////////
  bool encode_mon_registration_layer_field(pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    auto* layer_vec = static_cast<std::vector<eCAL::Monitoring::STransportLayer>*>(*arg);

    for (auto layer : *layer_vec)
    {
      if (!pb_encode_tag_for_field(stream, field))
      {
        return false;
      }

      eCAL_pb_TransportLayer pb_layer = eCAL_pb_TransportLayer_init_default;
      pb_layer.type = static_cast<eCAL_pb_eTransportLayerType>(layer.type);
      pb_layer.version = layer.version;
      pb_layer.active = layer.active;

      if (!pb_encode_submessage(stream, eCAL_pb_TransportLayer_fields, &pb_layer))
      {
        return false;
      }
    }

    return true;
  }

  void encode_mon_registration_layer(pb_callback_t& pb_callback, const std::vector<eCAL::Monitoring::STransportLayer>& layer_vec)
  {
    pb_callback.funcs.encode = &encode_mon_registration_layer_field; // NOLINT(*-pro-type-union-access)
    pb_callback.arg = (void*)(&layer_vec);
  }

  void PrepareEncoding(const eCAL::Monitoring::STopic& topic_, eCAL_pb_Topic& pb_topic_)
  {
    // registration_clock
    pb_topic_.registration_clock = topic_.registration_clock;
    // host_name
    eCAL::nanopb::encode_string(pb_topic_.host_name, topic_.host_name);
    // shm_transport_domain
    eCAL::nanopb::encode_string(pb_topic_.shm_transport_domain, topic_.shm_transport_domain);
    // process_id
    pb_topic_.process_id = topic_.process_id;
    // process_name
    eCAL::nanopb::encode_string(pb_topic_.process_name, topic_.process_name);
    // unit_name
    eCAL::nanopb::encode_string(pb_topic_.unit_name, topic_.unit_name);
    // topic_id
    eCAL::nanopb::encode_int_to_string(pb_topic_.topic_id, topic_.topic_id);
    // topic_name
    eCAL::nanopb::encode_string(pb_topic_.topic_name, topic_.topic_name);
    // direction
    eCAL::nanopb::encode_string(pb_topic_.direction, topic_.direction);
    // datatype_information
    pb_topic_.has_datatype_information = true;
    // datatype_information.name
    eCAL::nanopb::encode_string(pb_topic_.datatype_information.name, topic_.datatype_information.name);
    // datatype_information.encoding
    eCAL::nanopb::encode_string(pb_topic_.datatype_information.encoding, topic_.datatype_information.encoding);
    // datatype_information.descriptor
    eCAL::nanopb::encode_string(pb_topic_.datatype_information.descriptor_information, topic_.datatype_information.descriptor);
    // topic_size
    pb_topic_.topic_size = topic_.topic_size;
    // connections_local
    pb_topic_.connections_local = topic_.connections_local;
    // connections_external
    pb_topic_.connections_external = topic_.connections_external;
    // message_drops
    pb_topic_.message_drops = topic_.message_drops;
    // data_id
    pb_topic_.data_id = topic_.data_id;
    // data_clock
    pb_topic_.data_clock = topic_.data_clock;
    // data_frequency
    pb_topic_.data_frequency = topic_.data_frequency;
    // transport_layer
    encode_mon_registration_layer(pb_topic_.transport_layer, topic_.transport_layer);
  }

  bool encode_mon_message_topics_field(pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    auto* monitoring   = static_cast<eCAL::Monitoring::SMonitoring*>(*arg);

    //////////////////
    // publisher
    //////////////////
    for (const auto& topic : monitoring->publishers)
    {
      // encode topic tag
      if (!pb_encode_tag_for_field(stream, field))
      {
        return false;
      }

      // encode single topic
      eCAL_pb_Topic pb_topic = eCAL_pb_Topic_init_default;
      PrepareEncoding(topic, pb_topic);

      // encode submessage
      if (!pb_encode_submessage(stream, eCAL_pb_Topic_fields, &pb_topic))
      {
        return false;
      }
    }

    //////////////////
    // subscriber
    //////////////////
    for (const auto& topic : monitoring->subscribers)
    {
      // encode topic tag
      if (!pb_encode_tag_for_field(stream, field))
      {
        return false;
      }

      // encode single topic
      eCAL_pb_Topic pb_topic = eCAL_pb_Topic_init_default;
      PrepareEncoding(topic, pb_topic);

      // encode submessage
      if (!pb_encode_submessage(stream, eCAL_pb_Topic_fields, &pb_topic))
      {
        return false;
      }
    }

    return true;
  }

  /////////////////////////////////////////////////////////////////////////////////
  // Encode: eCAL::Monitoring::SServerMon
  /////////////////////////////////////////////////////////////////////////////////
  bool encode_mon_service_methods_field(pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    auto* method_vec = static_cast<std::vector<eCAL::Monitoring::SMethod>*>(*arg);

    for (const auto& method : *method_vec)
    {
      if (!pb_encode_tag_for_field(stream, field))
      {
        return false;
      }

      eCAL_pb_Method pb_method = eCAL_pb_Method_init_default;
      eCAL::nanopb::encode_string(pb_method.method_name, method.method_name);

      pb_method.has_request_datatype_information = true;
      eCAL::nanopb::encode_string(pb_method.request_datatype_information.name, method.request_datatype_information.name);
      eCAL::nanopb::encode_string(pb_method.request_datatype_information.encoding, method.request_datatype_information.encoding);
      eCAL::nanopb::encode_string(pb_method.request_datatype_information.descriptor_information, method.request_datatype_information.descriptor);

      pb_method.has_response_datatype_information = true;
      eCAL::nanopb::encode_string(pb_method.response_datatype_information.name, method.response_datatype_information.name);
      eCAL::nanopb::encode_string(pb_method.response_datatype_information.encoding, method.response_datatype_information.encoding);
      eCAL::nanopb::encode_string(pb_method.response_datatype_information.descriptor_information, method.response_datatype_information.descriptor);

      pb_method.call_count = method.call_count;

      if (!pb_encode_submessage(stream, eCAL_pb_Method_fields, &pb_method))
      {
        return false;
      }
    }

    return true;
  }

  void encode_mon_service_methods(pb_callback_t& pb_callback, const std::vector<eCAL::Monitoring::SMethod>& method_vec)
  {
    pb_callback.funcs.encode = &encode_mon_service_methods_field; // NOLINT(*-pro-type-union-access)
    pb_callback.arg = (void*)(&method_vec);
  }

  void PrepareEncoding(const eCAL::Monitoring::SServer& service_, eCAL_pb_Service& pb_service_)
  {
    ///////////////////////////////////////////////
    // service information
    ///////////////////////////////////////////////
    // registration_clock
    pb_service_.registration_clock = service_.registration_clock;
    // host_name
    eCAL::nanopb::encode_string(pb_service_.host_name, service_.host_name);
    // process_name
    eCAL::nanopb::encode_string(pb_service_.process_name, service_.process_name);
    // unit_name
    eCAL::nanopb::encode_string(pb_service_.unit_name, service_.unit_name);
    // process_id
    pb_service_.process_id = service_.process_id;
    // service_name
    eCAL::nanopb::encode_string(pb_service_.service_name, service_.service_name);
    // service_id
    eCAL::nanopb::encode_int_to_string(pb_service_.service_id, service_.service_id);
    // methods
    encode_mon_service_methods(pb_service_.methods, service_.methods);
    // version
    pb_service_.version = service_.version;
    // tcp_port_v0
    pb_service_.tcp_port_v0 = service_.tcp_port_v0;
    // tcp_port_v1
    pb_service_.tcp_port_v1 = service_.tcp_port_v1;
  }

  bool encode_mon_message_services_field(pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    auto* service_vec = static_cast<std::vector<eCAL::Monitoring::SServer>*>(*arg);

    for (const auto& service : *service_vec)
    {
      // encode process tag
      if (!pb_encode_tag_for_field(stream, field))
      {
        return false;
      }

      // encode single service
      eCAL_pb_Service pb_service = eCAL_pb_Service_init_default;
      PrepareEncoding(service, pb_service);

      // encode submessage
      if (!pb_encode_submessage(stream, eCAL_pb_Service_fields, &pb_service))
      {
        return false;
      }
    }

    return true;
  }

  /////////////////////////////////////////////////////////////////////////////////
  // Encode: eCAL::Monitoring::SClientMon
  /////////////////////////////////////////////////////////////////////////////////
  void PrepareEncoding(const eCAL::Monitoring::SClient& client_, eCAL_pb_Client& pb_client_)
  {
    ///////////////////////////////////////////////
    // client information
    ///////////////////////////////////////////////
    // registration_clock
    pb_client_.registration_clock = client_.registration_clock;
    // host_name
    eCAL::nanopb::encode_string(pb_client_.host_name, client_.host_name);
    // process_name
    eCAL::nanopb::encode_string(pb_client_.process_name, client_.process_name);
    // unit_name
    eCAL::nanopb::encode_string(pb_client_.unit_name, client_.unit_name);
    // process_id
    pb_client_.process_id = client_.process_id;
    // service_name
    eCAL::nanopb::encode_string(pb_client_.service_name, client_.service_name);
    // service_id
    eCAL::nanopb::encode_int_to_string(pb_client_.service_id, client_.service_id);
    // methods
    encode_mon_service_methods(pb_client_.methods, client_.methods);
    // version
    pb_client_.version = client_.version;
  }

  bool encode_mon_message_clients_field(pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    auto* client_vec = static_cast<std::vector<eCAL::Monitoring::SClient>*>(*arg);

    for (const auto& client : *client_vec)
    {
      // encode process tag
      if (!pb_encode_tag_for_field(stream, field))
      {
        return false;
      }

      // encode single client
      eCAL_pb_Client pb_client = eCAL_pb_Client_init_default;
      PrepareEncoding(client, pb_client);

      // encode submessage
      if (!pb_encode_submessage(stream, eCAL_pb_Client_fields, &pb_client))
      {
        return false;
      }
    }

    return true;
  }

  /////////////////////////////////////////////////////////////////////////////////
  // Encode: eCAL::Monitoring::Monitoring
  /////////////////////////////////////////////////////////////////////////////////
  void PrepareEncoding(const eCAL::Monitoring::SMonitoring& mon_message_, eCAL_pb_Monitoring& pb_mon_message_)
  {
    ///////////////////////////////////////////////
    // processes
    ///////////////////////////////////////////////
    pb_mon_message_.processes.funcs.encode = &encode_mon_message_processes_field; // NOLINT(*-pro-type-union-access)
    pb_mon_message_.processes.arg = (void*)(&mon_message_.processes);

    ///////////////////////////////////////////////
    // topics (publisher/subscriber)
    ///////////////////////////////////////////////
    pb_mon_message_.topics.funcs.encode = &encode_mon_message_topics_field; // NOLINT(*-pro-type-union-access)
    pb_mon_message_.topics.arg = (void*)(&mon_message_);

    ///////////////////////////////////////////////
    // services
    ///////////////////////////////////////////////
    pb_mon_message_.services.funcs.encode = &encode_mon_message_services_field; // NOLINT(*-pro-type-union-access)
    pb_mon_message_.services.arg = (void*)(&mon_message_.servers);

    ///////////////////////////////////////////////
    // clients
    ///////////////////////////////////////////////
    pb_mon_message_.clients.funcs.encode = &encode_mon_message_clients_field; // NOLINT(*-pro-type-union-access)
    pb_mon_message_.clients.arg = (void*)(&mon_message_.clients);
  }

  size_t MonitoringStruct2PbMonitoring(const eCAL::Monitoring::SMonitoring& mon_message_, eCAL_pb_Monitoring& pb_mon_message_)
  {
    ///////////////////////////////////////////////
    // prepare sample for encoding
    ///////////////////////////////////////////////
    PrepareEncoding(mon_message_, pb_mon_message_);

    ///////////////////////////////////////////////
    // evaluate byte size
    ///////////////////////////////////////////////
    pb_ostream_t pb_sizestream = { nullptr, nullptr, 0, 0, nullptr };
    pb_encode(&pb_sizestream, eCAL_pb_Monitoring_fields, &pb_mon_message_);

    // return encoding byte size
    return pb_sizestream.bytes_written;
  }

  template <typename T>
  bool MonitoringStruct2Buffer(const eCAL::Monitoring::SMonitoring& mon_message_, T& target_buffer_)
  {
    target_buffer_.clear();

    ///////////////////////////////////////////////
    // prepare sample for encoding
    ///////////////////////////////////////////////
    eCAL_pb_Monitoring pb_mon_message = eCAL_pb_Monitoring_init_default;
    size_t target_size = MonitoringStruct2PbMonitoring(mon_message_, pb_mon_message);

    ///////////////////////////////////////////////
    // encode it
    ///////////////////////////////////////////////
    target_buffer_.resize(target_size);
    pb_ostream_t pb_ostream;
    pb_ostream = pb_ostream_from_buffer((pb_byte_t*)(target_buffer_.data()), target_buffer_.size());
    if (!pb_encode(&pb_ostream, eCAL_pb_Monitoring_fields, &pb_mon_message))
    {
      std::cerr << "NanoPb eCAL::Monitoring::SMonitoring encode failed: " << pb_ostream.errmsg << '\n';
    }
    else
    {
      return true;
    }

    return false;
  }

  /////////////////////////////////////////////////////////////////////////////////
  // Decode: eCAL::Monitoring::SProcessMon
  /////////////////////////////////////////////////////////////////////////////////
  void PrepareDecoding(eCAL_pb_Process& pb_process_, eCAL::Monitoring::SProcess& process_)
  {
    // initialize
    pb_process_ = eCAL_pb_Process_init_default;

    ///////////////////////////////////////////////
    // assign decoder
    ///////////////////////////////////////////////
    // host_name
    eCAL::nanopb::decode_string(pb_process_.host_name, process_.host_name);
    // shm_transport_domain
    eCAL::nanopb::decode_string(pb_process_.shm_transport_domain, process_.shm_transport_domain);
    // process_name
    eCAL::nanopb::decode_string(pb_process_.process_name, process_.process_name);
    // unit_name
    eCAL::nanopb::decode_string(pb_process_.unit_name, process_.unit_name);
    // process_parameter
    eCAL::nanopb::decode_string(pb_process_.process_parameter, process_.process_parameter);
    // state.info
    eCAL::nanopb::decode_string(pb_process_.state.info, process_.state_info);
    // time_sync_module_name
    eCAL::nanopb::decode_string(pb_process_.time_sync_module_name, process_.time_sync_module_name);
    // component_init_info
    eCAL::nanopb::decode_string(pb_process_.component_init_info, process_.component_init_info);
    // ecal_runtime_version
    eCAL::nanopb::decode_string(pb_process_.ecal_runtime_version, process_.ecal_runtime_version);
    // ecal_config_path
    eCAL::nanopb::decode_string(pb_process_.config_file_path, process_.config_file_path);
  }

  void AssignValues(const eCAL_pb_Process& pb_process_, eCAL::Monitoring::SProcess& process_)
  {
    ///////////////////////////////////////////////
    // assign values
    ///////////////////////////////////////////////
    // registration_clock
    process_.registration_clock = pb_process_.registration_clock;
    // process_id
    process_.process_id = pb_process_.process_id;
    // state.severity
    process_.state_severity = pb_process_.state.severity;
    // state.severity_level
    process_.state_severity_level = pb_process_.state.severity_level;
    // time_sync_state
    process_.time_sync_state = pb_process_.time_sync_state;
    // component_init_state
    process_.component_init_state = pb_process_.component_init_state;
  }

  bool decode_processes_field(pb_istream_t* stream, const pb_field_iter_t* /*field*/, void** arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    eCAL_pb_Process pb_process = eCAL_pb_Process_init_default;
    eCAL::Monitoring::SProcess process{};

    // prepare sample for decoding
    PrepareDecoding(pb_process, process);

    // decode it
    if (!pb_decode(stream, eCAL_pb_Process_fields, &pb_process))
    {
      return false;
    }

    // apply sample values
    AssignValues(pb_process, process);

    // add sample to vector
    auto* processes_vec = static_cast<std::vector<eCAL::Monitoring::SProcess>*>(*arg);
    processes_vec->push_back(process);

    return true;
  }

  /////////////////////////////////////////////////////////////////////////////////
  // Decode: eCAL::Monitoring::STopicMon
  /////////////////////////////////////////////////////////////////////////////////
  bool decode_mon_registration_layer_field(pb_istream_t* stream, const pb_field_iter_t* /*field*/, void** arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    eCAL_pb_TransportLayer           pb_layer = eCAL_pb_TransportLayer_init_default;
    eCAL::Monitoring::STransportLayer layer{};

    if (!pb_decode(stream, eCAL_pb_TransportLayer_fields, &pb_layer))
    {
      return false;
    }

    // apply layer values
    layer.type = static_cast<eCAL::Monitoring::eTransportLayerType>(pb_layer.type);
    layer.version = pb_layer.version;
    layer.active  = pb_layer.active;

    // add layer
    auto* tgt_vector = static_cast<std::vector<eCAL::Monitoring::STransportLayer>*>(*arg);
    tgt_vector->push_back(layer);

    return true;
  }

  void decode_mon_registration_layer(pb_callback_t& pb_callback, std::vector<eCAL::Monitoring::STransportLayer>& layer_vec)
  {
    pb_callback.funcs.decode = &decode_mon_registration_layer_field; // NOLINT(*-pro-type-union-access)
    pb_callback.arg = &layer_vec;
  }

  void PrepareDecoding(eCAL_pb_Topic& pb_topic_, eCAL::Monitoring::STopic& topic_)
  {
    // initialize
    pb_topic_ = eCAL_pb_Topic_init_default;

    ///////////////////////////////////////////////
    // assign decoder
    ///////////////////////////////////////////////
    // host_name
    eCAL::nanopb::decode_string(pb_topic_.host_name, topic_.host_name);
    // shm_transport_domain
    eCAL::nanopb::decode_string(pb_topic_.shm_transport_domain, topic_.shm_transport_domain);
    // process_name
    eCAL::nanopb::decode_string(pb_topic_.process_name, topic_.process_name);
    // unit_name
    eCAL::nanopb::decode_string(pb_topic_.unit_name, topic_.unit_name);
    // topic_id
    eCAL::nanopb::decode_int_from_string(pb_topic_.topic_id, topic_.topic_id);
    // topic_name
    eCAL::nanopb::decode_string(pb_topic_.topic_name, topic_.topic_name);
    // direction
    eCAL::nanopb::decode_string(pb_topic_.direction, topic_.direction);
    // datatype_information.name
    eCAL::nanopb::decode_string(pb_topic_.datatype_information.name, topic_.datatype_information.name);
    // datatype_information.encoding
    eCAL::nanopb::decode_string(pb_topic_.datatype_information.encoding, topic_.datatype_information.encoding);
    // datatype_information.descriptor
    eCAL::nanopb::decode_string(pb_topic_.datatype_information.descriptor_information, topic_.datatype_information.descriptor);
    // transport_layer
    decode_mon_registration_layer(pb_topic_.transport_layer, topic_.transport_layer);
  }

  void AssignValues(const eCAL_pb_Topic& pb_topic_, eCAL::Monitoring::STopic& topic_)
  {
    ///////////////////////////////////////////////
    // assign values
    ///////////////////////////////////////////////
    // registration_clock
    topic_.registration_clock = pb_topic_.registration_clock;
    // process_id
    topic_.process_id = pb_topic_.process_id;
    // topic_size
    topic_.topic_size = pb_topic_.topic_size;
    // connections_local
    topic_.connections_local = pb_topic_.connections_local;
    // connections_external
    topic_.connections_external = pb_topic_.connections_external;
    // message_drops
    topic_.message_drops = pb_topic_.message_drops;
    // data_id
    topic_.data_id = pb_topic_.data_id;
    // data_clock
    topic_.data_clock = pb_topic_.data_clock;
    // data_frequency
    topic_.data_frequency = pb_topic_.data_frequency;
  }

  bool decode_topics_field(pb_istream_t* stream, const pb_field_iter_t* /*field*/, void** arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    eCAL_pb_Topic pb_topic = eCAL_pb_Topic_init_default;
    eCAL::Monitoring::STopic topic{};

    // prepare sample for decoding
    PrepareDecoding(pb_topic, topic);

    // decode it
    if (!pb_decode(stream, eCAL_pb_Topic_fields, &pb_topic))
    {
      return false;
    }

    // apply sample values
    AssignValues(pb_topic, topic);

    // add sample to vector
    auto* monitoring = static_cast<eCAL::Monitoring::SMonitoring*>(*arg);
    if (topic.direction == "publisher")
    {
      monitoring->publishers.push_back(topic);
    }
    if (topic.direction == "subscriber")
    {
      monitoring->subscribers.push_back(topic);
    }

    return true;
  }

  /////////////////////////////////////////////////////////////////////////////////
  // Decode: eCAL::Monitoring::SServerMon
  /////////////////////////////////////////////////////////////////////////////////
  bool decode_mon_service_methods_field(pb_istream_t* stream, const pb_field_iter_t* /*field*/, void** arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    eCAL_pb_Method pb_method = eCAL_pb_Method_init_default;
    eCAL::Monitoring::SMethod method{};

    // decode method parameter
    eCAL::nanopb::decode_string(pb_method.method_name, method.method_name);

    eCAL::nanopb::decode_string(pb_method.request_datatype_information.name, method.request_datatype_information.name);
    eCAL::nanopb::decode_string(pb_method.request_datatype_information.encoding, method.request_datatype_information.encoding);
    eCAL::nanopb::decode_string(pb_method.request_datatype_information.descriptor_information, method.request_datatype_information.descriptor);

    eCAL::nanopb::decode_string(pb_method.response_datatype_information.name, method.response_datatype_information.name);
    eCAL::nanopb::decode_string(pb_method.response_datatype_information.encoding, method.response_datatype_information.encoding);
    eCAL::nanopb::decode_string(pb_method.response_datatype_information.descriptor_information, method.response_datatype_information.descriptor);

    // decode it
    if (!pb_decode(stream, eCAL_pb_Method_fields, &pb_method))
    {
      return false;
    }

    // apply method values
    method.call_count = pb_method.call_count;

    // add method to vector
    auto* method_vec = static_cast<std::vector<eCAL::Monitoring::SMethod>*>(*arg);
    method_vec->emplace_back(method);

    return true;
  }

  void decode_mon_service_methods(pb_callback_t& pb_callback, std::vector<eCAL::Monitoring::SMethod>& method_vec)
  {
    pb_callback.funcs.decode = &decode_mon_service_methods_field; // NOLINT(*-pro-type-union-access)
    pb_callback.arg = &method_vec;
  }

  void PrepareDecoding(eCAL_pb_Service& pb_service_, eCAL::Monitoring::SServer& service_)
  {
    // initialize
    pb_service_ = eCAL_pb_Service_init_default;

    ///////////////////////////////////////////////
    // assign decoder
    ///////////////////////////////////////////////
    // host_name
    eCAL::nanopb::decode_string(pb_service_.host_name, service_.host_name);
    // process_name
    eCAL::nanopb::decode_string(pb_service_.process_name, service_.process_name);
    // unit_name
    eCAL::nanopb::decode_string(pb_service_.unit_name, service_.unit_name);
    // service_name
    eCAL::nanopb::decode_string(pb_service_.service_name, service_.service_name);
    // service_id
    eCAL::nanopb::decode_int_from_string(pb_service_.service_id, service_.service_id);
    // methods
    decode_mon_service_methods(pb_service_.methods, service_.methods);
  }

  void AssignValues(const eCAL_pb_Service& pb_service_, eCAL::Monitoring::SServer& service_)
  {
    ///////////////////////////////////////////////
    // assign values
    ///////////////////////////////////////////////
    // registration_clock
    service_.registration_clock = pb_service_.registration_clock;
    // process_id
    service_.process_id = pb_service_.process_id;
    // version
    service_.version = pb_service_.version;
    // tcp_port_v0
    service_.tcp_port_v0 = pb_service_.tcp_port_v0;
    // tcp_port_v1
    service_.tcp_port_v1 = pb_service_.tcp_port_v1;
  }

  bool decode_services_field(pb_istream_t* stream, const pb_field_iter_t* /*field*/, void** arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    eCAL_pb_Service pb_service = eCAL_pb_Service_init_default;
    eCAL::Monitoring::SServer service{};

    // prepare sample for decoding
    PrepareDecoding(pb_service, service);

    // decode it
    if (!pb_decode(stream, eCAL_pb_Service_fields, &pb_service))
    {
      return false;
    }

    // apply sample values
    AssignValues(pb_service, service);

    // add sample to vector
    auto* services_vec = static_cast<std::vector<eCAL::Monitoring::SServer>*>(*arg);
    services_vec->push_back(service);

    return true;
  }

  /////////////////////////////////////////////////////////////////////////////////
  // Decode: eCAL::Monitoring::SClientMon
  /////////////////////////////////////////////////////////////////////////////////
  void PrepareDecoding(eCAL_pb_Client& pb_client_, eCAL::Monitoring::SClient& client_)
  {
    // initialize
    pb_client_ = eCAL_pb_Client_init_default;

    ///////////////////////////////////////////////
    // assign decoder
    ///////////////////////////////////////////////
    // host_name
    eCAL::nanopb::decode_string(pb_client_.host_name, client_.host_name);
    // process_name
    eCAL::nanopb::decode_string(pb_client_.process_name, client_.process_name);
    // unit_name
    eCAL::nanopb::decode_string(pb_client_.unit_name, client_.unit_name);
    // service_name
    eCAL::nanopb::decode_string(pb_client_.service_name, client_.service_name);
    // service_id
    eCAL::nanopb::decode_int_from_string(pb_client_.service_id, client_.service_id);
    // methods
    decode_mon_service_methods(pb_client_.methods, client_.methods);
  }

  void AssignValues(const eCAL_pb_Client& pb_client_, eCAL::Monitoring::SClient& client_)
  {
    ///////////////////////////////////////////////
    // assign values
    ///////////////////////////////////////////////
    // registration_clock
    client_.registration_clock = pb_client_.registration_clock;
    // process_id
    client_.process_id = pb_client_.process_id;
    // version
    client_.version = pb_client_.version;
  }

  bool decode_clients_field(pb_istream_t* stream, const pb_field_iter_t* /*field*/, void** arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    eCAL_pb_Client pb_client = eCAL_pb_Client_init_default;
    eCAL::Monitoring::SClient client{};

    // prepare sample for decoding
    PrepareDecoding(pb_client, client);

    // decode it
    if (!pb_decode(stream, eCAL_pb_Client_fields, &pb_client))
    {
      return false;
    }

    // apply sample values
    AssignValues(pb_client, client);

    // add sample to vector
    auto* client_vec = static_cast<std::vector<eCAL::Monitoring::SClient>*>(*arg);
    client_vec->push_back(client);

    return true;
  }

  /////////////////////////////////////////////////////////////////////////////////
  // Decode: eCAL::Monitoring::Monitoring
  /////////////////////////////////////////////////////////////////////////////////
  bool Buffer2MonitoringStruct(const char* data_, size_t size_, eCAL::Monitoring::SMonitoring& mon_message_)
  {
    if (data_ == nullptr) return false;
    if (size_ == 0)       return false;

    // initialize
    eCAL_pb_Monitoring pb_mon_message = eCAL_pb_Monitoring_init_default;

    ///////////////////////////////////////////////
    // prepare processes for decoding
    ///////////////////////////////////////////////
    pb_mon_message.processes.funcs.decode = &decode_processes_field; // NOLINT(*-pro-type-union-access)
    pb_mon_message.processes.arg = &mon_message_.processes;

    ///////////////////////////////////////////////
    // prepare topics for decoding
    ///////////////////////////////////////////////
    pb_mon_message.topics.funcs.decode = &decode_topics_field; // NOLINT(*-pro-type-union-access)
    pb_mon_message.topics.arg = &mon_message_;

    ///////////////////////////////////////////////
    // prepare services for decoding
    ///////////////////////////////////////////////
    pb_mon_message.services.funcs.decode = &decode_services_field; // NOLINT(*-pro-type-union-access)
    pb_mon_message.services.arg = &mon_message_.servers;

    ///////////////////////////////////////////////
    // prepare clients for decoding
    ///////////////////////////////////////////////
    pb_mon_message.clients.funcs.decode = &decode_clients_field; // NOLINT(*-pro-type-union-access)
    pb_mon_message.clients.arg = &mon_message_.clients;

    ///////////////////////////////////////////////
    // decode it
    ///////////////////////////////////////////////
    pb_istream_t pb_istream;
    pb_istream = pb_istream_from_buffer((pb_byte_t*)data_, size_); // NOLINT(*-pro-type-cstyle-cast)
    if (!pb_decode(&pb_istream, eCAL_pb_Monitoring_fields, &pb_mon_message))
    {
      std::cerr << "NanoPb eCAL::Monitoring::SMonitoring decode failed: " << pb_istream.errmsg << '\n';
      return false;
    }

    return true;
  }
}

namespace eCAL
{
  namespace nanopb
  {
    // monitoring - serialize/deserialize
    bool SerializeToBuffer(const Monitoring::SMonitoring& source_sample_, std::vector<char>& target_buffer_)
    {
      return MonitoringStruct2Buffer(source_sample_, target_buffer_);
    }
  
    bool SerializeToBuffer(const Monitoring::SMonitoring& source_sample_, std::string& target_buffer_)
    {
      return MonitoringStruct2Buffer(source_sample_, target_buffer_);
    }
  
    bool DeserializeFromBuffer(const char* data_, size_t size_, Monitoring::SMonitoring& target_sample_)
    {
      return Buffer2MonitoringStruct(data_, size_, target_sample_);
    }
  }
}


////////////////////////////////////////////////////////

#include <ecal/core/pb/monitoring.pbftags.h>
#include <ecal/core/pb/layer.pbftags.h>
#include <ecal/core/pb/process.pbftags.h>
#include <ecal/core/pb/service.pbftags.h>
#include <ecal/core/pb/topic.pbftags.h>
#include <protozero/pbf_writer.hpp>
#include <protozero/buffer_vector.hpp>
#include <protozero/pbf_reader.hpp>
#include <protozero/ecal_helper.h>
#include <ecal_serialize_common.h>


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
