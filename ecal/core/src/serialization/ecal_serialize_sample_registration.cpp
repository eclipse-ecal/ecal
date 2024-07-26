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
#include "nanopb/ecal.pb.h"

#include "ecal_serialize_common.h"
#include "ecal_serialize_sample_registration.h"

#include <cstddef>
#include <iostream>
#include <list>
#include <string>
#include <vector>

namespace
{
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

    ///////////////////////////////////////////////
    // process information
    ///////////////////////////////////////////////
    pb_sample_.has_process = true;

    // rclock
    pb_sample_.process.rclock = registration_.process.rclock;
    // hname
    eCAL::nanopb::encode_string(pb_sample_.process.hname, registration_.process.hname);
    // hgname
    eCAL::nanopb::encode_string(pb_sample_.process.hgname, registration_.process.hgname);
    // pid
    pb_sample_.process.pid = registration_.process.pid;
    // pname
    eCAL::nanopb::encode_string(pb_sample_.process.pname, registration_.process.pname);
    // uname
    eCAL::nanopb::encode_string(pb_sample_.process.uname, registration_.process.uname);
    // pparam
    eCAL::nanopb::encode_string(pb_sample_.process.pparam, registration_.process.pparam);
    // state
    pb_sample_.process.has_state = true;
    // state.severity
    pb_sample_.process.state.severity = static_cast<eCAL_pb_eProcessSeverity>(registration_.process.state.severity);
    // state.severity_level
    pb_sample_.process.state.severity_level = static_cast<eCAL_pb_eProcessSeverityLevel>(registration_.process.state.severity_level);
    // state.info
    eCAL::nanopb::encode_string(pb_sample_.process.state.info, registration_.process.state.info);
    // process.tsync_state
    pb_sample_.process.tsync_state = static_cast<eCAL_pb_eTSyncState>(registration_.process.tsync_state);
    // tsync_mod_name
    eCAL::nanopb::encode_string(pb_sample_.process.tsync_mod_name, registration_.process.tsync_mod_name);
    // component_init_state
    pb_sample_.process.component_init_state = registration_.process.component_init_state;
    // component_init_info
    eCAL::nanopb::encode_string(pb_sample_.process.component_init_info, registration_.process.component_init_info);
    // ecal_runtime_version
    eCAL::nanopb::encode_string(pb_sample_.process.ecal_runtime_version, registration_.process.ecal_runtime_version);

    ///////////////////////////////////////////////
    // service information
    ///////////////////////////////////////////////
    pb_sample_.has_service = true;

    // rclock
    pb_sample_.service.rclock = registration_.service.rclock;
    // hname
    eCAL::nanopb::encode_string(pb_sample_.service.hname, registration_.service.hname);
    // pname
    eCAL::nanopb::encode_string(pb_sample_.service.pname, registration_.service.pname);
    // uname
    eCAL::nanopb::encode_string(pb_sample_.service.uname, registration_.service.uname);
    // pid
    pb_sample_.service.pid = registration_.service.pid;
    // sname
    eCAL::nanopb::encode_string(pb_sample_.service.sname, registration_.service.sname);
    // sid
    eCAL::nanopb::encode_string(pb_sample_.service.sid, registration_.service.sid);
    // methods
    eCAL::nanopb::encode_service_methods(pb_sample_.service.methods, registration_.service.methods);
    // version
    pb_sample_.service.version = registration_.service.version;
    // tcp_port_v0
    pb_sample_.service.tcp_port_v0 = registration_.service.tcp_port_v0;
    // tcp_port_v1
    pb_sample_.service.tcp_port_v1 = registration_.service.tcp_port_v1;

    ///////////////////////////////////////////////
    // client information
    ///////////////////////////////////////////////
    pb_sample_.has_client = true;

    // rclock
    pb_sample_.client.rclock = registration_.client.rclock;
    // hname
    eCAL::nanopb::encode_string(pb_sample_.client.hname, registration_.client.hname);
    // pname
    eCAL::nanopb::encode_string(pb_sample_.client.pname, registration_.client.pname);
    // uname
    eCAL::nanopb::encode_string(pb_sample_.client.uname, registration_.client.uname);
    // pid
    pb_sample_.client.pid = registration_.client.pid;
    // sname
    eCAL::nanopb::encode_string(pb_sample_.client.sname, registration_.client.sname);
    // sid
    eCAL::nanopb::encode_string(pb_sample_.client.sid, registration_.client.sid);
    // methods
    eCAL::nanopb::encode_service_methods(pb_sample_.client.methods, registration_.client.methods);
    // version
    pb_sample_.client.version = registration_.client.version;

    ///////////////////////////////////////////////
    // topic information
    ///////////////////////////////////////////////
    pb_sample_.has_topic = true;

    // rclock
    pb_sample_.topic.rclock = registration_.topic.rclock;
    // hname
    eCAL::nanopb::encode_string(pb_sample_.topic.hname, registration_.topic.hname);
    // hgname
    eCAL::nanopb::encode_string(pb_sample_.topic.hgname, registration_.topic.hgname);
    // pid
    pb_sample_.topic.pid = registration_.topic.pid;
    // pname
    eCAL::nanopb::encode_string(pb_sample_.topic.pname, registration_.topic.pname);
    // uname
    eCAL::nanopb::encode_string(pb_sample_.topic.uname, registration_.topic.uname);
    // tid
    eCAL::nanopb::encode_string(pb_sample_.topic.tid, registration_.topic.tid);
    // tname
    eCAL::nanopb::encode_string(pb_sample_.topic.tname, registration_.topic.tname);
    // direction
    eCAL::nanopb::encode_string(pb_sample_.topic.direction, registration_.topic.direction);
    // tdatatype
    pb_sample_.topic.has_tdatatype = true;
    // tdatatype.name
    eCAL::nanopb::encode_string(pb_sample_.topic.tdatatype.name, registration_.topic.tdatatype.name);
    // tdatatype.encoding
    eCAL::nanopb::encode_string(pb_sample_.topic.tdatatype.encoding, registration_.topic.tdatatype.encoding);
    // tdatatype.desc
    eCAL::nanopb::encode_string(pb_sample_.topic.tdatatype.desc, registration_.topic.tdatatype.descriptor);
    // tsize
    pb_sample_.topic.tsize = registration_.topic.tsize;
    // connections_loc
    pb_sample_.topic.connections_loc = registration_.topic.connections_loc;
    // connections_ext
    pb_sample_.topic.connections_ext = registration_.topic.connections_ext;
    // message_drops
    pb_sample_.topic.message_drops = registration_.topic.message_drops;
    // did
    pb_sample_.topic.did = registration_.topic.did;
    // dclock
    pb_sample_.topic.dclock = registration_.topic.dclock;
    // dfreq
    pb_sample_.topic.dfreq = registration_.topic.dfreq;
    // tlayer
    eCAL::nanopb::encode_registration_layer(pb_sample_.topic.tlayer, registration_.topic.tlayer);
    // attr
    eCAL::nanopb::encode_map(pb_sample_.topic.attr, registration_.topic.attr);
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
    eCAL::nanopb::decode_string(pb_sample_.process.hname, registration_.process.hname);
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
    eCAL::nanopb::decode_string(pb_sample_.service.hname, registration_.service.hname);
    // pname
    eCAL::nanopb::decode_string(pb_sample_.service.pname, registration_.service.pname);
    // uname
    eCAL::nanopb::decode_string(pb_sample_.service.uname, registration_.service.uname);
    // sname
    eCAL::nanopb::decode_string(pb_sample_.service.sname, registration_.service.sname);
    // sid
    eCAL::nanopb::decode_string(pb_sample_.service.sid, registration_.service.sid);
    // methods
    eCAL::nanopb::decode_service_methods(pb_sample_.service.methods, registration_.service.methods);

    ///////////////////////////////////////////////
    // client information
    ///////////////////////////////////////////////
    // hname
    eCAL::nanopb::decode_string(pb_sample_.client.hname, registration_.client.hname);
    // pname
    eCAL::nanopb::decode_string(pb_sample_.client.pname, registration_.client.pname);
    // uname
    eCAL::nanopb::decode_string(pb_sample_.client.uname, registration_.client.uname);
    // sname
    eCAL::nanopb::decode_string(pb_sample_.client.sname, registration_.client.sname);
    // sid
    eCAL::nanopb::decode_string(pb_sample_.client.sid, registration_.client.sid);
    // methods
    eCAL::nanopb::decode_service_methods(pb_sample_.client.methods, registration_.client.methods);

    ///////////////////////////////////////////////
    // topic information
    ///////////////////////////////////////////////
    // hname
    eCAL::nanopb::decode_string(pb_sample_.topic.hname, registration_.topic.hname);
    // hgname
    eCAL::nanopb::decode_string(pb_sample_.topic.hgname, registration_.topic.hgname);
    // pname
    eCAL::nanopb::decode_string(pb_sample_.topic.pname, registration_.topic.pname);
    // uname
    eCAL::nanopb::decode_string(pb_sample_.topic.uname, registration_.topic.uname);
    // tid
    eCAL::nanopb::decode_string(pb_sample_.topic.tid, registration_.topic.tid);
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

    ///////////////////////////////////////////////
    // process information
    ///////////////////////////////////////////////
    // rclock
    registration_.process.rclock = pb_sample_.process.rclock;
    // pid
    registration_.process.pid = pb_sample_.process.pid;
    // state.severity
    registration_.process.state.severity = static_cast<eCAL::Registration::eProcessSeverity>(pb_sample_.process.state.severity);
    // state.severity_level
    registration_.process.state.severity_level = static_cast<eCAL::Registration::eProcessSeverityLevel>(pb_sample_.process.state.severity_level);
    // tsync_state
    registration_.process.tsync_state = static_cast<eCAL::Registration::eTSyncState>(pb_sample_.process.tsync_state);
    // component_init_state
    registration_.process.component_init_state = pb_sample_.process.component_init_state;

    ///////////////////////////////////////////////
    // service information
    ///////////////////////////////////////////////
    // rclock
    registration_.service.rclock = pb_sample_.service.rclock;
    // pid
    registration_.service.pid = pb_sample_.service.pid;
    // version
    registration_.service.version = pb_sample_.service.version;
    // tcp_port_v0
    registration_.service.tcp_port_v0 = pb_sample_.service.tcp_port_v0;
    // tcp_port_v1
    registration_.service.tcp_port_v1 = pb_sample_.service.tcp_port_v1;

    ///////////////////////////////////////////////
    // client information
    ///////////////////////////////////////////////
    // rclock
    registration_.client.rclock = pb_sample_.client.rclock;
    // pid
    registration_.client.pid = pb_sample_.client.pid;
    // version
    registration_.client.version = pb_sample_.client.version;

    ///////////////////////////////////////////////
    // topic information
    ///////////////////////////////////////////////
    // rclock
    registration_.topic.rclock = pb_sample_.topic.rclock;
    // pid
    registration_.topic.pid = pb_sample_.topic.pid;
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

    auto* sample_list = static_cast<std::list<eCAL::Registration::Sample>*>(*arg);

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
    pb_sample_list_.samples.arg = (void*)(&registration_list_.samples);

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
    eCAL::Registration::Sample sample{};

    // prepare sample for decoding
    PrepareDecoding(pb_sample, sample);

    // decode it
    if (!pb_decode(stream, eCAL_pb_Sample_fields, &pb_sample))
    {
      return false;
    }

    // apply sample values
    AssignValues(pb_sample, sample);

    // add sample to list
    auto* sample_list = static_cast<std::list<eCAL::Registration::Sample>*>(*arg);
    sample_list->push_back(sample);

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
    pb_sample_list.samples.arg = &registration_list_.samples;

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
