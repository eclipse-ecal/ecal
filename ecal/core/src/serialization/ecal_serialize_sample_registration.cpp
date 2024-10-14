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
    // rclock
    pb_process_.rclock = registration_process_.rclock;
    // hname
    eCAL::nanopb::encode_string(pb_process_.hname, registration_identifier_.host_name);
    // hgname
    eCAL::nanopb::encode_string(pb_process_.hgname, registration_process_.hgname);
    // pid
    pb_process_.pid = registration_identifier_.process_id;
    // pname
    eCAL::nanopb::encode_string(pb_process_.pname, registration_process_.pname);
    // uname
    eCAL::nanopb::encode_string(pb_process_.uname, registration_process_.uname);
    // pparam
    eCAL::nanopb::encode_string(pb_process_.pparam, registration_process_.pparam);

    // state
    pb_process_.has_state = true;
    // state.severity
    pb_process_.state.severity = static_cast<eCAL_pb_eProcessSeverity>(registration_process_.state.severity);
    // state.severity_level
    pb_process_.state.severity_level = static_cast<eCAL_pb_eProcessSeverityLevel>(registration_process_.state.severity_level);
    // state.info
    eCAL::nanopb::encode_string(pb_process_.state.info, registration_process_.state.info);
    // process.tsync_state
    pb_process_.tsync_state = static_cast<eCAL_pb_eTSyncState>(registration_process_.tsync_state);
    // tsync_mod_name
    eCAL::nanopb::encode_string(pb_process_.tsync_mod_name, registration_process_.tsync_mod_name);
    // component_init_state
    pb_process_.component_init_state = registration_process_.component_init_state;
    // component_init_info
    eCAL::nanopb::encode_string(pb_process_.component_init_info, registration_process_.component_init_info);
    // ecal_runtime_version
    eCAL::nanopb::encode_string(pb_process_.ecal_runtime_version, registration_process_.ecal_runtime_version);
  }

  ///////////////////////////////////////////////
  // service information
  ///////////////////////////////////////////////
  void PrepareEncoding(const eCAL::Registration::Sample& registration_, eCAL_pb_Service& pb_service_)
  {
    const auto& registration_service_    = registration_.service;
    const auto& registration_identifier_ = registration_.identifier;

    // rclock
    pb_service_.rclock = registration_service_.rclock;
    // hname
    eCAL::nanopb::encode_string(pb_service_.hname, registration_identifier_.host_name);
    // pname
    eCAL::nanopb::encode_string(pb_service_.pname, registration_service_.pname);
    // uname
    eCAL::nanopb::encode_string(pb_service_.uname, registration_service_.uname);
    // pid
    pb_service_.pid = registration_identifier_.process_id;
    // sname
    eCAL::nanopb::encode_string(pb_service_.sname, registration_service_.sname);
    // sid
    eCAL::nanopb::encode_string(pb_service_.sid, registration_identifier_.entity_id);
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

    // rclock
    pb_client_.rclock = registration_client_.rclock;
    // hname
    eCAL::nanopb::encode_string(pb_client_.hname, registration_producer_.host_name);
    // pname
    eCAL::nanopb::encode_string(pb_client_.pname, registration_client_.pname);
    // uname
    eCAL::nanopb::encode_string(pb_client_.uname, registration_client_.uname);
    // pid
    pb_client_.pid = registration_producer_.process_id;
    // sname
    eCAL::nanopb::encode_string(pb_client_.sname, registration_client_.sname);
    // sid
    eCAL::nanopb::encode_string(pb_client_.sid, registration_producer_.entity_id);
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

    // rclock
    pb_topic_.rclock = registration_topic_.rclock;
    // hname
    eCAL::nanopb::encode_string(pb_topic_.hname, registration_identifier_.host_name);
    // hgname
    eCAL::nanopb::encode_string(pb_topic_.hgname, registration_topic_.hgname);
    // pid
    pb_topic_.pid = registration_identifier_.process_id;
    // pname
    eCAL::nanopb::encode_string(pb_topic_.pname, registration_topic_.pname);
    // uname
    eCAL::nanopb::encode_string(pb_topic_.uname, registration_topic_.uname);
    // tid
    eCAL::nanopb::encode_string(pb_topic_.tid, registration_identifier_.entity_id);
    // tname
    eCAL::nanopb::encode_string(pb_topic_.tname, registration_topic_.tname);
    // direction
    eCAL::nanopb::encode_string(pb_topic_.direction, registration_topic_.direction);
    // tdatatype
    pb_topic_.has_tdatatype = true;
    // tdatatype.name
    eCAL::nanopb::encode_string(pb_topic_.tdatatype.name, registration_topic_.tdatatype.name);
    // tdatatype.encoding
    eCAL::nanopb::encode_string(pb_topic_.tdatatype.encoding, registration_topic_.tdatatype.encoding);
    // tdatatype.desc
    eCAL::nanopb::encode_string(pb_topic_.tdatatype.desc, registration_topic_.tdatatype.descriptor);
    // tsize
    pb_topic_.tsize = registration_topic_.tsize;
    // connections_loc
    pb_topic_.connections_loc = registration_topic_.connections_loc;
    // connections_ext
    pb_topic_.connections_ext = registration_topic_.connections_ext;
    // message_drops
    pb_topic_.message_drops = registration_topic_.message_drops;
    // did
    pb_topic_.did = registration_topic_.did;
    // dclock
    pb_topic_.dclock = registration_topic_.dclock;
    // dfreq
    pb_topic_.dfreq = registration_topic_.dfreq;
    // tlayer
    eCAL::nanopb::encode_registration_layer(pb_topic_.tlayer, registration_topic_.tlayer);
    // attr
    eCAL::nanopb::encode_map(pb_topic_.attr, registration_topic_.attr);
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

    // hname
    eCAL::nanopb::encode_string(pb_sample_.host.hname, registration_.host.hname);

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
    // hname
    eCAL::nanopb::decode_string(pb_sample_.host.hname, registration_.host.hname);

    ///////////////////////////////////////////////
    // process information
    ///////////////////////////////////////////////
    // hname
    eCAL::nanopb::decode_string(pb_sample_.process.hname, registration_.identifier.host_name);
    // hgname
    eCAL::nanopb::decode_string(pb_sample_.process.hgname, registration_.process.hgname);
    // pname
    eCAL::nanopb::decode_string(pb_sample_.process.pname, registration_.process.pname);
    // uname
    eCAL::nanopb::decode_string(pb_sample_.process.uname, registration_.process.uname);
    // pparam
    eCAL::nanopb::decode_string(pb_sample_.process.pparam, registration_.process.pparam);
    // state.info
    eCAL::nanopb::decode_string(pb_sample_.process.state.info, registration_.process.state.info);
    // tsync_mod_name
    eCAL::nanopb::decode_string(pb_sample_.process.tsync_mod_name, registration_.process.tsync_mod_name);
    // component_init_info
    eCAL::nanopb::decode_string(pb_sample_.process.component_init_info, registration_.process.component_init_info);
    // ecal_runtime_version
    eCAL::nanopb::decode_string(pb_sample_.process.ecal_runtime_version, registration_.process.ecal_runtime_version);

    ///////////////////////////////////////////////
    // service information
    ///////////////////////////////////////////////
    // hname
    eCAL::nanopb::decode_string(pb_sample_.service.hname, registration_.identifier.host_name);
    // pname
    eCAL::nanopb::decode_string(pb_sample_.service.pname, registration_.service.pname);
    // uname
    eCAL::nanopb::decode_string(pb_sample_.service.uname, registration_.service.uname);
    // sname
    eCAL::nanopb::decode_string(pb_sample_.service.sname, registration_.service.sname);
    // sid
    eCAL::nanopb::decode_string(pb_sample_.service.sid, registration_.identifier.entity_id);
    // methods
    eCAL::nanopb::decode_service_methods(pb_sample_.service.methods, registration_.service.methods);

    ///////////////////////////////////////////////
    // client information
    ///////////////////////////////////////////////
    // hname
    eCAL::nanopb::decode_string(pb_sample_.client.hname, registration_.identifier.host_name);
    // pname
    eCAL::nanopb::decode_string(pb_sample_.client.pname, registration_.client.pname);
    // uname
    eCAL::nanopb::decode_string(pb_sample_.client.uname, registration_.client.uname);
    // sname
    eCAL::nanopb::decode_string(pb_sample_.client.sname, registration_.client.sname);
    // sid
    eCAL::nanopb::decode_string(pb_sample_.client.sid, registration_.identifier.entity_id);
    // methods
    eCAL::nanopb::decode_service_methods(pb_sample_.client.methods, registration_.client.methods);

    ///////////////////////////////////////////////
    // topic information
    ///////////////////////////////////////////////
    // hname
    eCAL::nanopb::decode_string(pb_sample_.topic.hname, registration_.identifier.host_name);
    // hgname
    eCAL::nanopb::decode_string(pb_sample_.topic.hgname, registration_.topic.hgname);
    // pname
    eCAL::nanopb::decode_string(pb_sample_.topic.pname, registration_.topic.pname);
    // uname
    eCAL::nanopb::decode_string(pb_sample_.topic.uname, registration_.topic.uname);
    // tid
    eCAL::nanopb::decode_string(pb_sample_.topic.tid, registration_.identifier.entity_id);
    // tname
    eCAL::nanopb::decode_string(pb_sample_.topic.tname, registration_.topic.tname);
    // direction
    eCAL::nanopb::decode_string(pb_sample_.topic.direction, registration_.topic.direction);
    // tdatatype.name
    eCAL::nanopb::decode_string(pb_sample_.topic.tdatatype.name, registration_.topic.tdatatype.name);
    // tdatatype.encoding
    eCAL::nanopb::decode_string(pb_sample_.topic.tdatatype.encoding, registration_.topic.tdatatype.encoding);
    // tdatatype.desc
    eCAL::nanopb::decode_string(pb_sample_.topic.tdatatype.desc, registration_.topic.tdatatype.descriptor);
    // tlayer
    eCAL::nanopb::decode_registration_layer(pb_sample_.topic.tlayer, registration_.topic.tlayer);
    // attr
    eCAL::nanopb::decode_map(pb_sample_.topic.attr, registration_.topic.attr);
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
      // rclock
      registration_.process.rclock = pb_sample_.process.rclock;
      // pid
      registration_.identifier.process_id = pb_sample_.process.pid;
      // tid -> we need to use the PID here, because we don't have a designated field for it
      registration_.identifier.entity_id = std::to_string(registration_.identifier.process_id);
      // state.severity
      registration_.process.state.severity = static_cast<eCAL::Registration::eProcessSeverity>(pb_sample_.process.state.severity);
      // state.severity_level
      registration_.process.state.severity_level = static_cast<eCAL::Registration::eProcessSeverityLevel>(pb_sample_.process.state.severity_level);
      // tsync_state
      registration_.process.tsync_state = static_cast<eCAL::Registration::eTSyncState>(pb_sample_.process.tsync_state);
      // component_init_state
      registration_.process.component_init_state = pb_sample_.process.component_init_state;
      break;
    case eCAL::bct_reg_service:
    case eCAL::bct_unreg_service:
      // rclock
      registration_.service.rclock = pb_sample_.service.rclock;
      // pid
      registration_.identifier.process_id = pb_sample_.service.pid;
      // version
      registration_.service.version = pb_sample_.service.version;
      // tcp_port_v0
      registration_.service.tcp_port_v0 = pb_sample_.service.tcp_port_v0;
      // tcp_port_v1
      registration_.service.tcp_port_v1 = pb_sample_.service.tcp_port_v1;
      break;
    case eCAL::bct_reg_client:
    case eCAL::bct_unreg_client:
      // rclock
      registration_.client.rclock = pb_sample_.client.rclock;
      // pid
      registration_.identifier.process_id = pb_sample_.client.pid;
      // version
      registration_.client.version = pb_sample_.client.version;
      break;
    case eCAL::bct_reg_publisher:
    case eCAL::bct_unreg_publisher:
    case eCAL::bct_reg_subscriber:
    case eCAL::bct_unreg_subscriber:
      // rclock
      registration_.topic.rclock = pb_sample_.topic.rclock;
      // pid
      registration_.identifier.process_id = pb_sample_.topic.pid;
      // tsize
      registration_.topic.tsize = pb_sample_.topic.tsize;
      // connections_loc
      registration_.topic.connections_loc = pb_sample_.topic.connections_loc;
      // connections_ext
      registration_.topic.connections_ext = pb_sample_.topic.connections_ext;
      // message_drops
      registration_.topic.message_drops = pb_sample_.topic.message_drops;
      // did
      registration_.topic.did = pb_sample_.topic.did;
      // dclock
      registration_.topic.dclock = pb_sample_.topic.dclock;
      // dfreq
      registration_.topic.dfreq = pb_sample_.topic.dfreq;
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
