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
 * @file   ecal_serialize_sample_registration.cpp
 * @brief  eCAL registration sample (de)serialization
**/

#include "nanopb/pb_encode.h"
#include "nanopb/pb_decode.h"
#include "nanopb/ecal/core/pb/ecal.npb.h"

#include "ecal_serialize_common.h"
#include "ecal_serialize_sample_registration.h"

#include <cstddef>
#include <iostream>
#include <list>
#include <string>
#include <utility>
#include <vector>

namespace
{
  ///////////////////////////////////////////////
  // process information
  ///////////////////////////////////////////////
  void PrepareEncoding(const eCAL::Registration::Sample& registration_, eCAL_pb_Process& pb_process_)
  {
    const auto& registration_process_    = registration_.process;
    const auto& registration_identifier_ = registration_.identifier;
    // registration_clock
    pb_process_.registration_clock = registration_process_.registration_clock;
    // host_name
    eCAL::nanopb::encode_string(pb_process_.host_name, registration_identifier_.host_name);
    // shm_transport_domain
    eCAL::nanopb::encode_string(pb_process_.shm_transport_domain, registration_process_.shm_transport_domain);
    // process_id
    pb_process_.process_id = registration_identifier_.process_id;
    // process_name
    eCAL::nanopb::encode_string(pb_process_.process_name, registration_process_.process_name);
    // unit_name
    eCAL::nanopb::encode_string(pb_process_.unit_name, registration_process_.unit_name);
    // process_parameter
    eCAL::nanopb::encode_string(pb_process_.process_parameter, registration_process_.process_parameter);

    // state
    pb_process_.has_state = true;
    // state.severity
    pb_process_.state.severity = static_cast<eCAL_pb_eProcessSeverity>(registration_process_.state.severity);
    // state.severity_level
    pb_process_.state.severity_level = static_cast<eCAL_pb_eProcessSeverityLevel>(registration_process_.state.severity_level);
    // state.info
    eCAL::nanopb::encode_string(pb_process_.state.info, registration_process_.state.info);
    // process.time_sync_state
    pb_process_.time_sync_state = static_cast<eCAL_pb_eTimeSyncState>(registration_process_.time_sync_state);
    // time_sync_module_name
    eCAL::nanopb::encode_string(pb_process_.time_sync_module_name, registration_process_.time_sync_module_name);
    // component_init_state
    pb_process_.component_init_state = registration_process_.component_init_state;
    // component_init_info
    eCAL::nanopb::encode_string(pb_process_.component_init_info, registration_process_.component_init_info);
    // ecal_runtime_version
    eCAL::nanopb::encode_string(pb_process_.ecal_runtime_version, registration_process_.ecal_runtime_version);
    // configuration path
    eCAL::nanopb::encode_string(pb_process_.config_file_path, registration_process_.config_file_path);
  }

  ///////////////////////////////////////////////
  // service information
  ///////////////////////////////////////////////
  void PrepareEncoding(const eCAL::Registration::Sample& registration_, eCAL_pb_Service& pb_service_)
  {
    const auto& registration_service_    = registration_.service;
    const auto& registration_identifier_ = registration_.identifier;

    // registration_clock
    pb_service_.registration_clock = registration_service_.registration_clock;
    // host_name
    eCAL::nanopb::encode_string(pb_service_.host_name, registration_identifier_.host_name);
    // process_name
    eCAL::nanopb::encode_string(pb_service_.process_name, registration_service_.process_name);
    // unit_name
    eCAL::nanopb::encode_string(pb_service_.unit_name, registration_service_.unit_name);
    // process_id
    pb_service_.process_id = registration_identifier_.process_id;
    // service_name
    eCAL::nanopb::encode_string(pb_service_.service_name, registration_service_.service_name);
    // service_id
    eCAL::nanopb::encode_int_to_string(pb_service_.service_id, registration_identifier_.entity_id);
    // methods
    eCAL::nanopb::encode_service_methods(pb_service_.methods, registration_service_.methods);
    // version
    pb_service_.version = registration_service_.version;
    // tcp_port_v0
    pb_service_.tcp_port_v0 = registration_service_.tcp_port_v0;
    // tcp_port_v1
    pb_service_.tcp_port_v1 = registration_service_.tcp_port_v1;
  }

  ///////////////////////////////////////////////
  // client information
  ///////////////////////////////////////////////
  void PrepareEncoding(const eCAL::Registration::Sample& registration_, eCAL_pb_Client& pb_client_)
  {
    const auto& registration_client_ = registration_.client;
    const auto& registration_producer_ = registration_.identifier;

    // registration_clock
    pb_client_.registration_clock = registration_client_.registration_clock;
    // host_name
    eCAL::nanopb::encode_string(pb_client_.host_name, registration_producer_.host_name);
    // process_name
    eCAL::nanopb::encode_string(pb_client_.process_name, registration_client_.process_name);
    // unit_name
    eCAL::nanopb::encode_string(pb_client_.unit_name, registration_client_.unit_name);
    // process_id
    pb_client_.process_id = registration_producer_.process_id;
    // service_name
    eCAL::nanopb::encode_string(pb_client_.service_name, registration_client_.service_name);
    // service_id
    eCAL::nanopb::encode_int_to_string(pb_client_.service_id, registration_producer_.entity_id);
    // methods
    eCAL::nanopb::encode_service_methods(pb_client_.methods, registration_client_.methods);
    // version
    pb_client_.version = registration_client_.version;
  }

  ///////////////////////////////////////////////
  // topic information
  ///////////////////////////////////////////////
  void PrepareEncoding(const eCAL::Registration::Sample& registration_sample_, eCAL_pb_Topic& pb_topic_)
  {
    const auto& registration_identifier_ = registration_sample_.identifier;
    const auto& registration_topic_      = registration_sample_.topic;

    // registration_clock
    pb_topic_.registration_clock = registration_topic_.registration_clock;
    // host_name
    eCAL::nanopb::encode_string(pb_topic_.host_name, registration_identifier_.host_name);
    // shm_transport_domain
    eCAL::nanopb::encode_string(pb_topic_.shm_transport_domain, registration_topic_.shm_transport_domain);
    // process_id
    pb_topic_.process_id = registration_identifier_.process_id;
    // process_name
    eCAL::nanopb::encode_string(pb_topic_.process_name, registration_topic_.process_name);
    // unit_name
    eCAL::nanopb::encode_string(pb_topic_.unit_name, registration_topic_.unit_name);
    // topic_id
    eCAL::nanopb::encode_int_to_string(pb_topic_.topic_id, registration_identifier_.entity_id);
    // topic_name
    eCAL::nanopb::encode_string(pb_topic_.topic_name, registration_topic_.topic_name);
    // direction
    eCAL::nanopb::encode_string(pb_topic_.direction, registration_topic_.direction);
    // datatype_information
    pb_topic_.has_datatype_information = true;
    // datatype_information.name
    eCAL::nanopb::encode_string(pb_topic_.datatype_information.name, registration_topic_.datatype_information.name);
    // datatype_information.encoding
    eCAL::nanopb::encode_string(pb_topic_.datatype_information.encoding, registration_topic_.datatype_information.encoding);
    // datatype_information.descriptor
    eCAL::nanopb::encode_string(pb_topic_.datatype_information.descriptor_information, registration_topic_.datatype_information.descriptor);
    // topic_size
    pb_topic_.topic_size = registration_topic_.topic_size;
    // connections_local
    pb_topic_.connections_local = registration_topic_.connections_local;
    // connections_external
    pb_topic_.connections_external = registration_topic_.connections_external;
    // message_drops
    pb_topic_.message_drops = registration_topic_.message_drops;
    // data_id
    pb_topic_.data_id = registration_topic_.data_id;
    // data_clock
    pb_topic_.data_clock = registration_topic_.data_clock;
    // data_frequency
    pb_topic_.data_frequency = registration_topic_.data_frequency;
    // transport_layer
    eCAL::nanopb::encode_registration_layer(pb_topic_.transport_layer, registration_topic_.transport_layer);
  }

  /////////////////////////////////////////////////////////////////////////////////
  // eCAL::Registration::Sample
  /////////////////////////////////////////////////////////////////////////////////
  void PrepareEncoding(const eCAL::Registration::Sample& registration_, eCAL_pb_Sample& pb_sample_)
  {
    // command type
    pb_sample_.cmd_type = static_cast<eCAL_pb_eCmdType>(registration_.cmd_type);

    ///////////////////////////////////////////////
    // host information
    ///////////////////////////////////////////////
    pb_sample_.has_host = true;

    // host_name
    eCAL::nanopb::encode_string(pb_sample_.host.name, registration_.host.name);

    pb_sample_.has_process = false;
    pb_sample_.has_service = false;
    pb_sample_.has_client = false;
    pb_sample_.has_topic = false;

    switch (registration_.cmd_type)
    {
    case eCAL::bct_reg_process:
    case eCAL::bct_unreg_process:
      pb_sample_.has_process = true;
      PrepareEncoding(registration_, pb_sample_.process);
      break;

    case eCAL::bct_reg_service:
    case eCAL::bct_unreg_service:
      pb_sample_.has_service = true;
      PrepareEncoding(registration_, pb_sample_.service);
      break;

    case eCAL::bct_reg_client:
    case eCAL::bct_unreg_client:
      pb_sample_.has_client = true;
      PrepareEncoding(registration_, pb_sample_.client);
      break;

    case eCAL::bct_reg_publisher:
    case eCAL::bct_unreg_publisher:
    case eCAL::bct_reg_subscriber:
    case eCAL::bct_unreg_subscriber:
      pb_sample_.has_topic = true;
      PrepareEncoding(registration_, pb_sample_.topic);
      break;

    default:
      break;
    }
  }

  size_t RegistrationStruct2PbSample(const eCAL::Registration::Sample& registration_, eCAL_pb_Sample& pb_sample_)
  {
    ///////////////////////////////////////////////
    // prepare sample for encoding
    ///////////////////////////////////////////////
    PrepareEncoding(registration_, pb_sample_);

    ///////////////////////////////////////////////
    // evaluate byte size
    ///////////////////////////////////////////////
    pb_ostream_t pb_sizestream = { nullptr, nullptr, 0, 0, nullptr};
    pb_encode(&pb_sizestream, eCAL_pb_Sample_fields, &pb_sample_);

    // return encoding byte size
    return pb_sizestream.bytes_written;
  }

  template <typename T>
  bool RegistrationStruct2Buffer(const eCAL::Registration::Sample& registration_, T& target_buffer_)
  {
    target_buffer_.clear();

    ///////////////////////////////////////////////
    // prepare sample for encoding
    ///////////////////////////////////////////////
    eCAL_pb_Sample pb_sample = eCAL_pb_Sample_init_default;
    size_t target_size = RegistrationStruct2PbSample(registration_, pb_sample);

    ///////////////////////////////////////////////
    // encode it
    ///////////////////////////////////////////////
    target_buffer_.resize(target_size);
    pb_ostream_t pb_ostream;
    pb_ostream = pb_ostream_from_buffer((pb_byte_t*)(target_buffer_.data()), target_buffer_.size());
    if (!pb_encode(&pb_ostream, eCAL_pb_Sample_fields, &pb_sample))
    {
      std::cerr << "NanoPb eCAL::Registration::Sample encode failed: " << pb_ostream.errmsg << '\n';
    }
    else
    {
      return true;
    }

    return false;
  }

  void PrepareDecoding(eCAL_pb_Sample& pb_sample_, eCAL::Registration::Sample& registration_)
  {
    // initialize
    pb_sample_ = eCAL_pb_Sample_init_default;

    ///////////////////////////////////////////////
    // assign decoder
    ///////////////////////////////////////////////

    ///////////////////////////////////////////////
    // host information
    ///////////////////////////////////////////////
    // host_name
    eCAL::nanopb::decode_string(pb_sample_.host.name, registration_.host.name);

    ///////////////////////////////////////////////
    // process information
    ///////////////////////////////////////////////
    // host_name
    eCAL::nanopb::decode_string(pb_sample_.process.host_name, registration_.identifier.host_name);
    // shm_transport_domain
    eCAL::nanopb::decode_string(pb_sample_.process.shm_transport_domain, registration_.process.shm_transport_domain);
    // process_name
    eCAL::nanopb::decode_string(pb_sample_.process.process_name, registration_.process.process_name);
    // unit_name
    eCAL::nanopb::decode_string(pb_sample_.process.unit_name, registration_.process.unit_name);
    // process_parameter
    eCAL::nanopb::decode_string(pb_sample_.process.process_parameter, registration_.process.process_parameter);
    // state.info
    eCAL::nanopb::decode_string(pb_sample_.process.state.info, registration_.process.state.info);
    // time_sync_module_name
    eCAL::nanopb::decode_string(pb_sample_.process.time_sync_module_name, registration_.process.time_sync_module_name);
    // component_init_info
    eCAL::nanopb::decode_string(pb_sample_.process.component_init_info, registration_.process.component_init_info);
    // ecal_runtime_version
    eCAL::nanopb::decode_string(pb_sample_.process.ecal_runtime_version, registration_.process.ecal_runtime_version);
    // config_file_path
    eCAL::nanopb::decode_string(pb_sample_.process.config_file_path, registration_.process.config_file_path);

    ///////////////////////////////////////////////
    // service information
    ///////////////////////////////////////////////
    // host_name
    eCAL::nanopb::decode_string(pb_sample_.service.host_name, registration_.identifier.host_name);
    // process_name
    eCAL::nanopb::decode_string(pb_sample_.service.process_name, registration_.service.process_name);
    // unit_name
    eCAL::nanopb::decode_string(pb_sample_.service.unit_name, registration_.service.unit_name);
    // service_name
    eCAL::nanopb::decode_string(pb_sample_.service.service_name, registration_.service.service_name);
    // service_id
    eCAL::nanopb::decode_int_from_string(pb_sample_.service.service_id, registration_.identifier.entity_id);
    // methods
    eCAL::nanopb::decode_service_methods(pb_sample_.service.methods, registration_.service.methods);

    ///////////////////////////////////////////////
    // client information
    ///////////////////////////////////////////////
    // host_name
    eCAL::nanopb::decode_string(pb_sample_.client.host_name, registration_.identifier.host_name);
    // process_name
    eCAL::nanopb::decode_string(pb_sample_.client.process_name, registration_.client.process_name);
    // unit_name
    eCAL::nanopb::decode_string(pb_sample_.client.unit_name, registration_.client.unit_name);
    // service_name
    eCAL::nanopb::decode_string(pb_sample_.client.service_name, registration_.client.service_name);
    // service_id
    eCAL::nanopb::decode_int_from_string(pb_sample_.client.service_id, registration_.identifier.entity_id);
    // methods
    eCAL::nanopb::decode_service_methods(pb_sample_.client.methods, registration_.client.methods);

    ///////////////////////////////////////////////
    // topic information
    ///////////////////////////////////////////////
    // host_name
    eCAL::nanopb::decode_string(pb_sample_.topic.host_name, registration_.identifier.host_name);
    // shm_transport_domain
    eCAL::nanopb::decode_string(pb_sample_.topic.shm_transport_domain, registration_.topic.shm_transport_domain);
    // process_name
    eCAL::nanopb::decode_string(pb_sample_.topic.process_name, registration_.topic.process_name);
    // unit_name
    eCAL::nanopb::decode_string(pb_sample_.topic.unit_name, registration_.topic.unit_name);
    // topic_id
    eCAL::nanopb::decode_int_from_string(pb_sample_.topic.topic_id, registration_.identifier.entity_id);
    // topic_name
    eCAL::nanopb::decode_string(pb_sample_.topic.topic_name, registration_.topic.topic_name);
    // direction
    eCAL::nanopb::decode_string(pb_sample_.topic.direction, registration_.topic.direction);
    // datatype_information.name
    eCAL::nanopb::decode_string(pb_sample_.topic.datatype_information.name, registration_.topic.datatype_information.name);
    // datatype_information.encoding
    eCAL::nanopb::decode_string(pb_sample_.topic.datatype_information.encoding, registration_.topic.datatype_information.encoding);
    // datatype_information.descriptor
    eCAL::nanopb::decode_string(pb_sample_.topic.datatype_information.descriptor_information, registration_.topic.datatype_information.descriptor);
    // transport_layer
    eCAL::nanopb::decode_registration_layer(pb_sample_.topic.transport_layer, registration_.topic.transport_layer);
  }

  void AssignValues(const eCAL_pb_Sample& pb_sample_, eCAL::Registration::Sample& registration_)
  {
    ///////////////////////////////////////////////
    // assign values
    ///////////////////////////////////////////////
    // command type
    registration_.cmd_type = static_cast<eCAL::eCmdType>(pb_sample_.cmd_type);
    switch (registration_.cmd_type)
    {
    case eCAL::bct_reg_process:
    case eCAL::bct_unreg_process:
      // registration_clock
      registration_.process.registration_clock = pb_sample_.process.registration_clock;
      // process_id
      registration_.identifier.process_id = pb_sample_.process.process_id;
      // topic_id -> we need to use the PID here, because we don't have a designated field for it
      registration_.identifier.entity_id = registration_.identifier.process_id;
      // state.severity
      registration_.process.state.severity = static_cast<eCAL::Registration::eProcessSeverity>(pb_sample_.process.state.severity);
      // state.severity_level
      registration_.process.state.severity_level = static_cast<eCAL::Registration::eProcessSeverityLevel>(pb_sample_.process.state.severity_level);
      // time_sync_state
      registration_.process.time_sync_state = static_cast<eCAL::Registration::eTimeSyncState>(pb_sample_.process.time_sync_state);
      // component_init_state
      registration_.process.component_init_state = pb_sample_.process.component_init_state;
      break;
    case eCAL::bct_reg_service:
    case eCAL::bct_unreg_service:
      // registration_clock
      registration_.service.registration_clock = pb_sample_.service.registration_clock;
      // process_id
      registration_.identifier.process_id = pb_sample_.service.process_id;
      // version
      registration_.service.version = pb_sample_.service.version;
      // tcp_port_v0
      registration_.service.tcp_port_v0 = pb_sample_.service.tcp_port_v0;
      // tcp_port_v1
      registration_.service.tcp_port_v1 = pb_sample_.service.tcp_port_v1;
      break;
    case eCAL::bct_reg_client:
    case eCAL::bct_unreg_client:
      // registration_clock
      registration_.client.registration_clock = pb_sample_.client.registration_clock;
      // process_id
      registration_.identifier.process_id = pb_sample_.client.process_id;
      // version
      registration_.client.version = pb_sample_.client.version;
      break;
    case eCAL::bct_reg_publisher:
    case eCAL::bct_unreg_publisher:
    case eCAL::bct_reg_subscriber:
    case eCAL::bct_unreg_subscriber:
      // registration_clock
      registration_.topic.registration_clock = pb_sample_.topic.registration_clock;
      // process_id
      registration_.identifier.process_id = pb_sample_.topic.process_id;
      // topic_size
      registration_.topic.topic_size = pb_sample_.topic.topic_size;
      // connections_local
      registration_.topic.connections_local = pb_sample_.topic.connections_local;
      // connections_external
      registration_.topic.connections_external = pb_sample_.topic.connections_external;
      // message_drops
      registration_.topic.message_drops = pb_sample_.topic.message_drops;
      // data_id
      registration_.topic.data_id = pb_sample_.topic.data_id;
      // data_clock
      registration_.topic.data_clock = pb_sample_.topic.data_clock;
      // data_frequency
      registration_.topic.data_frequency = pb_sample_.topic.data_frequency;
      break;
    default:
    break;
    }
  }

  bool Buffer2RegistrationStruct(const char* data_, size_t size_, eCAL::Registration::Sample& registration_)
  {
    if (data_ == nullptr) return false;
    if (size_ == 0)       return false;

    // initialize
    eCAL_pb_Sample pb_sample = eCAL_pb_Sample_init_default;

    ///////////////////////////////////////////////
    // prepare sample for decoding
    ///////////////////////////////////////////////
    PrepareDecoding(pb_sample, registration_);

    ///////////////////////////////////////////////
    // decode it
    ///////////////////////////////////////////////
    pb_istream_t pb_istream;
    pb_istream = pb_istream_from_buffer((pb_byte_t*)data_, size_); // NOLINT(*-pro-type-cstyle-cast)
    if (!pb_decode(&pb_istream, eCAL_pb_Sample_fields, &pb_sample))
    {
      std::cerr << "NanoPb eCAL::Registration::Sample decode failed: " << pb_istream.errmsg << '\n';
      return false;
    }

    ///////////////////////////////////////////////
    // assign sample values
    ///////////////////////////////////////////////
    AssignValues(pb_sample, registration_);

    return true;
  }

  /////////////////////////////////////////////////////////////////////////////////
  // eCAL::Registration::SampleList
  /////////////////////////////////////////////////////////////////////////////////
  bool encode_sample_list_field(pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    auto* sample_list = static_cast<eCAL::Registration::SampleList*>(*arg);

    for (const auto& sample : *sample_list)
    {
      // encode sample tag
      if (!pb_encode_tag_for_field(stream, field))
      {
        return false;
      }

      // encode single sample
      eCAL_pb_Sample pb_sample = eCAL_pb_Sample_init_default;
      PrepareEncoding(sample, pb_sample);

      // encode submessage
      if (!pb_encode_submessage(stream, eCAL_pb_Sample_fields, &pb_sample))
      {
        return false;
      }
    }

    return true;
  }

  size_t RegistrationListStruct2PbSample(const eCAL::Registration::SampleList& registration_list_, eCAL_pb_SampleList& pb_sample_list_)
  {
    ///////////////////////////////////////////////
    // prepare sample for encoding
    ///////////////////////////////////////////////
    pb_sample_list_.samples.funcs.encode = &encode_sample_list_field; // NOLINT(*-pro-type-union-access)
    pb_sample_list_.samples.arg = (void*)(&registration_list_);

    ///////////////////////////////////////////////
    // evaluate byte size
    ///////////////////////////////////////////////
    pb_ostream_t pb_sizestream = { nullptr, nullptr, 0, 0, nullptr};
    pb_encode(&pb_sizestream, eCAL_pb_SampleList_fields, &pb_sample_list_);

    // return encoding byte size
    return pb_sizestream.bytes_written;
  }

  template <typename T>
  bool RegistrationListStruct2Buffer(const eCAL::Registration::SampleList& registration_list_, T& target_buffer_)
  {
    target_buffer_.clear();

    ///////////////////////////////////////////////
    // prepare sample for encoding
    ///////////////////////////////////////////////
    eCAL_pb_SampleList pb_sample_list = eCAL_pb_SampleList_init_default;
    size_t target_size = RegistrationListStruct2PbSample(registration_list_, pb_sample_list);

    ///////////////////////////////////////////////
    // encode it
    ///////////////////////////////////////////////
    target_buffer_.resize(target_size);
    pb_ostream_t pb_ostream;
    pb_ostream = pb_ostream_from_buffer((pb_byte_t*)(target_buffer_.data()), target_buffer_.size());
    if (!pb_encode(&pb_ostream, eCAL_pb_SampleList_fields, &pb_sample_list))
    {
      std::cerr << "NanoPb eCAL::Registration::SampleList encode failed: " << pb_ostream.errmsg << '\n';
    }
    else
    {
      return true;
    }

    return false;
  }

  bool decode_sample_list_field(pb_istream_t* stream, const pb_field_iter_t* /*field*/, void** arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    eCAL_pb_Sample pb_sample = eCAL_pb_Sample_init_default;

    // add sample to list
    auto* sample_list = static_cast<eCAL::Registration::SampleList*>(*arg);
    // Create a new element directly at the end of the vector
    auto& sample = sample_list->push_back();

    // prepare sample for decoding
    PrepareDecoding(pb_sample, sample);

    // decode it
    if (!pb_decode(stream, eCAL_pb_Sample_fields, &pb_sample))
    {
      return false;
    }

    // apply sample values
    AssignValues(pb_sample, sample);

    return true;
  }

  bool Buffer2RegistrationListStruct(const char* data_, size_t size_, eCAL::Registration::SampleList& registration_list_)
  {
    if (data_ == nullptr) return false;
    if (size_ == 0)       return false;

    // initialize
    eCAL_pb_SampleList pb_sample_list = eCAL_pb_SampleList_init_default;

    ///////////////////////////////////////////////
    // prepare sample for decoding
    ///////////////////////////////////////////////
    pb_sample_list.samples.funcs.decode = &decode_sample_list_field; // NOLINT(*-pro-type-union-access)
    pb_sample_list.samples.arg = &registration_list_;

    ///////////////////////////////////////////////
    // decode it
    ///////////////////////////////////////////////
    pb_istream_t pb_istream;
    pb_istream = pb_istream_from_buffer((pb_byte_t*)data_, size_); // NOLINT(*-pro-type-cstyle-cast)
    if (!pb_decode(&pb_istream, eCAL_pb_SampleList_fields, &pb_sample_list))
    {
      std::cerr << "NanoPb eCAL::Registration::Sample decode failed: " << pb_istream.errmsg << '\n';
      return false;
    }

    return true;
  }
}

namespace eCAL
{
  bool SerializeToBuffer(const Registration::Sample& source_sample_, std::vector<char>& target_buffer_)
  {
    return RegistrationStruct2Buffer(source_sample_, target_buffer_);
  }

  bool SerializeToBuffer(const Registration::Sample& source_sample_, std::string& target_buffer_)
  {
    return RegistrationStruct2Buffer(source_sample_, target_buffer_);
  }

  bool DeserializeFromBuffer(const char* data_, size_t size_, Registration::Sample& target_sample_)
  {
    return Buffer2RegistrationStruct(data_, size_, target_sample_);
  }

  bool SerializeToBuffer(const Registration::SampleList& source_sample_list_, std::vector<char>& target_buffer_)
  {
    return RegistrationListStruct2Buffer(source_sample_list_, target_buffer_);
  }

  bool SerializeToBuffer(const Registration::SampleList& source_sample_list_, std::string& target_buffer_)
  {
    return RegistrationListStruct2Buffer(source_sample_list_, target_buffer_);
  }

  bool DeserializeFromBuffer(const char* data_, size_t size_, Registration::SampleList& target_sample_list_)
  {
    return Buffer2RegistrationListStruct(data_, size_, target_sample_list_);
  }
}
