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
 * @file   ecal_serialize_monitoring.cpp
 * @brief  eCAL monitoring (de)serialization
**/

#include "nanopb/pb_encode.h"
#include "nanopb/pb_decode.h"
#include "nanopb/monitoring.pb.h"

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
  void PrepareEncoding(const eCAL::Monitoring::SProcessMon& process_, eCAL_pb_Process& pb_process_)
  {
    ///////////////////////////////////////////////
    // process information
    ///////////////////////////////////////////////
    // rclock
    pb_process_.rclock = process_.rclock;
    // hname
    eCAL::nanopb::encode_string(pb_process_.hname, process_.hname);
    // hgname
    eCAL::nanopb::encode_string(pb_process_.hgname, process_.hgname);
    // pid
    pb_process_.pid = process_.pid;
    // pname
    eCAL::nanopb::encode_string(pb_process_.pname, process_.pname);
    // uname
    eCAL::nanopb::encode_string(pb_process_.uname, process_.uname);
    // pparam
    eCAL::nanopb::encode_string(pb_process_.pparam, process_.pparam);
    // state
    pb_process_.has_state = true;
    // state.severity
    pb_process_.state.severity = static_cast<eCAL_pb_eProcessSeverity>(process_.state_severity);
    // state.severity_level
    pb_process_.state.severity_level = static_cast<eCAL_pb_eProcessSeverityLevel>(process_.state_severity_level);
    // state.info
    eCAL::nanopb::encode_string(pb_process_.state.info, process_.state_info);
    // process.tsync_state
    pb_process_.tsync_state = static_cast<eCAL_pb_eTSyncState>(process_.tsync_state);
    // tsync_mod_name
    eCAL::nanopb::encode_string(pb_process_.tsync_mod_name, process_.tsync_mod_name);
    // component_init_state
    pb_process_.component_init_state = process_.component_init_state;
    // component_init_info
    eCAL::nanopb::encode_string(pb_process_.component_init_info, process_.component_init_info);
    // ecal_runtime_version
    eCAL::nanopb::encode_string(pb_process_.ecal_runtime_version, process_.ecal_runtime_version);
  }

  bool encode_mon_message_processes_field(pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    auto* process_vec = static_cast<std::vector<eCAL::Monitoring::SProcessMon>*>(*arg);

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

    auto* layer_vec = static_cast<std::vector<eCAL::Monitoring::TLayer>*>(*arg);

    for (auto layer : *layer_vec)
    {
      if (!pb_encode_tag_for_field(stream, field))
      {
        return false;
      }

      eCAL_pb_TLayer pb_layer = eCAL_pb_TLayer_init_default;
      pb_layer.type = static_cast<eCAL_pb_eTLayerType>(layer.type);
      pb_layer.version = layer.version;
      pb_layer.active = layer.active;

      if (!pb_encode_submessage(stream, eCAL_pb_TLayer_fields, &pb_layer))
      {
        return false;
      }
    }

    return true;
  }

  void encode_mon_registration_layer(pb_callback_t& pb_callback, const std::vector<eCAL::Monitoring::TLayer>& layer_vec)
  {
    pb_callback.funcs.encode = &encode_mon_registration_layer_field; // NOLINT(*-pro-type-union-access)
    pb_callback.arg = (void*)(&layer_vec);
  }

  void PrepareEncoding(const eCAL::Monitoring::STopicMon& topic_, eCAL_pb_Topic& pb_topic_)
  {
    // rclock
    pb_topic_.rclock = topic_.rclock;
    // hname
    eCAL::nanopb::encode_string(pb_topic_.hname, topic_.hname);
    // hgname
    eCAL::nanopb::encode_string(pb_topic_.hgname, topic_.hgname);
    // pid
    pb_topic_.pid = topic_.pid;
    // pname
    eCAL::nanopb::encode_string(pb_topic_.pname, topic_.pname);
    // uname
    eCAL::nanopb::encode_string(pb_topic_.uname, topic_.uname);
    // tid
    eCAL::nanopb::encode_string(pb_topic_.tid, topic_.tid);
    // tname
    eCAL::nanopb::encode_string(pb_topic_.tname, topic_.tname);
    // direction
    eCAL::nanopb::encode_string(pb_topic_.direction, topic_.direction);
    // tdatatype
    pb_topic_.has_tdatatype = true;
    // tdatatype.name
    eCAL::nanopb::encode_string(pb_topic_.tdatatype.name, topic_.tdatatype.name);
    // tdatatype.encoding
    eCAL::nanopb::encode_string(pb_topic_.tdatatype.encoding, topic_.tdatatype.encoding);
    // tdatatype.desc
    eCAL::nanopb::encode_string(pb_topic_.tdatatype.desc, topic_.tdatatype.descriptor);
    // tsize
    pb_topic_.tsize = topic_.tsize;
    // connections_loc
    pb_topic_.connections_loc = topic_.connections_loc;
    // connections_ext
    pb_topic_.connections_ext = topic_.connections_ext;
    // message_drops
    pb_topic_.message_drops = topic_.message_drops;
    // did
    pb_topic_.did = topic_.did;
    // dclock
    pb_topic_.dclock = topic_.dclock;
    // dfreq
    pb_topic_.dfreq = topic_.dfreq;
    // tlayer
    encode_mon_registration_layer(pb_topic_.tlayer, topic_.tlayer);
    // attr
    eCAL::nanopb::encode_map(pb_topic_.attr, topic_.attr);
  }

  bool encode_mon_message_topics_field(pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    auto* monitoring   = static_cast<eCAL::Monitoring::SMonitoring*>(*arg);

    //////////////////
    // publisher
    //////////////////
    for (const auto& topic : monitoring->publisher)
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
    for (const auto& topic : monitoring->subscriber)
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

    auto* method_vec = static_cast<std::vector<eCAL::Monitoring::SMethodMon>*>(*arg);

    for (const auto& method : *method_vec)
    {
      if (!pb_encode_tag_for_field(stream, field))
      {
        return false;
      }

      eCAL_pb_Method pb_method = eCAL_pb_Method_init_default;
      eCAL::nanopb::encode_string(pb_method.mname, method.mname);
      eCAL::nanopb::encode_string(pb_method.req_type, method.req_type);
      eCAL::nanopb::encode_string(pb_method.resp_type, method.resp_type);
      eCAL::nanopb::encode_string(pb_method.req_desc, method.req_desc);
      eCAL::nanopb::encode_string(pb_method.resp_desc, method.resp_desc);
      pb_method.call_count = method.call_count;

      if (!pb_encode_submessage(stream, eCAL_pb_Method_fields, &pb_method))
      {
        return false;
      }
    }

    return true;
  }

  void encode_mon_service_methods(pb_callback_t& pb_callback, const std::vector<eCAL::Monitoring::SMethodMon>& method_vec)
  {
    pb_callback.funcs.encode = &encode_mon_service_methods_field; // NOLINT(*-pro-type-union-access)
    pb_callback.arg = (void*)(&method_vec);
  }

  void PrepareEncoding(const eCAL::Monitoring::SServerMon& service_, eCAL_pb_Service& pb_service_)
  {
    ///////////////////////////////////////////////
    // service information
    ///////////////////////////////////////////////
    // rclock
    pb_service_.rclock = service_.rclock;
    // hname
    eCAL::nanopb::encode_string(pb_service_.hname, service_.hname);
    // pname
    eCAL::nanopb::encode_string(pb_service_.pname, service_.pname);
    // uname
    eCAL::nanopb::encode_string(pb_service_.uname, service_.uname);
    // pid
    pb_service_.pid = service_.pid;
    // sname
    eCAL::nanopb::encode_string(pb_service_.sname, service_.sname);
    // sid
    eCAL::nanopb::encode_string(pb_service_.sid, service_.sid);
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

    auto* service_vec = static_cast<std::vector<eCAL::Monitoring::SServerMon>*>(*arg);

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
  void PrepareEncoding(const eCAL::Monitoring::SClientMon& client_, eCAL_pb_Client& pb_client_)
  {
    ///////////////////////////////////////////////
    // client information
    ///////////////////////////////////////////////
    // rclock
    pb_client_.rclock = client_.rclock;
    // hname
    eCAL::nanopb::encode_string(pb_client_.hname, client_.hname);
    // pname
    eCAL::nanopb::encode_string(pb_client_.pname, client_.pname);
    // uname
    eCAL::nanopb::encode_string(pb_client_.uname, client_.uname);
    // pid
    pb_client_.pid = client_.pid;
    // sname
    eCAL::nanopb::encode_string(pb_client_.sname, client_.sname);
    // sid
    eCAL::nanopb::encode_string(pb_client_.sid, client_.sid);
    // methods
    encode_mon_service_methods(pb_client_.methods, client_.methods);
    // version
    pb_client_.version = client_.version;
  }

  bool encode_mon_message_clients_field(pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    auto* client_vec = static_cast<std::vector<eCAL::Monitoring::SClientMon>*>(*arg);

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
    pb_mon_message_.services.arg = (void*)(&mon_message_.server);

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
  void PrepareDecoding(eCAL_pb_Process& pb_process_, eCAL::Monitoring::SProcessMon& process_)
  {
    // initialize
    pb_process_ = eCAL_pb_Process_init_default;

    ///////////////////////////////////////////////
    // assign decoder
    ///////////////////////////////////////////////
    // hname
    eCAL::nanopb::decode_string(pb_process_.hname, process_.hname);
    // hgname
    eCAL::nanopb::decode_string(pb_process_.hgname, process_.hgname);
    // pname
    eCAL::nanopb::decode_string(pb_process_.pname, process_.pname);
    // uname
    eCAL::nanopb::decode_string(pb_process_.uname, process_.uname);
    // pparam
    eCAL::nanopb::decode_string(pb_process_.pparam, process_.pparam);
    // state.info
    eCAL::nanopb::decode_string(pb_process_.state.info, process_.state_info);
    // tsync_mod_name
    eCAL::nanopb::decode_string(pb_process_.tsync_mod_name, process_.tsync_mod_name);
    // component_init_info
    eCAL::nanopb::decode_string(pb_process_.component_init_info, process_.component_init_info);
    // ecal_runtime_version
    eCAL::nanopb::decode_string(pb_process_.ecal_runtime_version, process_.ecal_runtime_version);
  }

  void AssignValues(const eCAL_pb_Process& pb_process_, eCAL::Monitoring::SProcessMon& process_)
  {
    ///////////////////////////////////////////////
    // assign values
    ///////////////////////////////////////////////
    // rclock
    process_.rclock = pb_process_.rclock;
    // pid
    process_.pid = pb_process_.pid;
    // state.severity
    process_.state_severity = pb_process_.state.severity;
    // state.severity_level
    process_.state_severity_level = pb_process_.state.severity_level;
    // tsync_state
    process_.tsync_state = pb_process_.tsync_state;
    // component_init_state
    process_.component_init_state = pb_process_.component_init_state;
  }

  bool decode_processes_field(pb_istream_t* stream, const pb_field_iter_t* /*field*/, void** arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    eCAL_pb_Process pb_process = eCAL_pb_Process_init_default;
    eCAL::Monitoring::SProcessMon process{};

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
    auto* processes_vec = static_cast<std::vector<eCAL::Monitoring::SProcessMon>*>(*arg);
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

    eCAL_pb_TLayer           pb_layer = eCAL_pb_TLayer_init_default;
    eCAL::Monitoring::TLayer layer{};

    if (!pb_decode(stream, eCAL_pb_TLayer_fields, &pb_layer))
    {
      return false;
    }

    // apply layer values
    layer.type = static_cast<eCAL::Monitoring::eTLayerType>(pb_layer.type);
    layer.version = pb_layer.version;
    layer.active  = pb_layer.active;

    // add layer
    auto* tgt_vector = static_cast<std::vector<eCAL::Monitoring::TLayer>*>(*arg);
    tgt_vector->push_back(layer);

    return true;
  }

  void decode_mon_registration_layer(pb_callback_t& pb_callback, std::vector<eCAL::Monitoring::TLayer>& layer_vec)
  {
    pb_callback.funcs.decode = &decode_mon_registration_layer_field; // NOLINT(*-pro-type-union-access)
    pb_callback.arg = &layer_vec;
  }

  void PrepareDecoding(eCAL_pb_Topic& pb_topic_, eCAL::Monitoring::STopicMon& topic_)
  {
    // initialize
    pb_topic_ = eCAL_pb_Topic_init_default;

    ///////////////////////////////////////////////
    // assign decoder
    ///////////////////////////////////////////////
    // hname
    eCAL::nanopb::decode_string(pb_topic_.hname, topic_.hname);
    // hgname
    eCAL::nanopb::decode_string(pb_topic_.hgname, topic_.hgname);
    // pname
    eCAL::nanopb::decode_string(pb_topic_.pname, topic_.pname);
    // uname
    eCAL::nanopb::decode_string(pb_topic_.uname, topic_.uname);
    // tid
    eCAL::nanopb::decode_string(pb_topic_.tid, topic_.tid);
    // tname
    eCAL::nanopb::decode_string(pb_topic_.tname, topic_.tname);
    // direction
    eCAL::nanopb::decode_string(pb_topic_.direction, topic_.direction);
    // tdatatype.name
    eCAL::nanopb::decode_string(pb_topic_.tdatatype.name, topic_.tdatatype.name);
    // tdatatype.encoding
    eCAL::nanopb::decode_string(pb_topic_.tdatatype.encoding, topic_.tdatatype.encoding);
    // tdatatype.desc
    eCAL::nanopb::decode_string(pb_topic_.tdatatype.desc, topic_.tdatatype.descriptor);
    // tlayer
    decode_mon_registration_layer(pb_topic_.tlayer, topic_.tlayer);
    // attr
    eCAL::nanopb::decode_map(pb_topic_.attr, topic_.attr);
  }

  void AssignValues(const eCAL_pb_Topic& pb_topic_, eCAL::Monitoring::STopicMon& topic_)
  {
    ///////////////////////////////////////////////
    // assign values
    ///////////////////////////////////////////////
    // rclock
    topic_.rclock = pb_topic_.rclock;
    // pid
    topic_.pid = pb_topic_.pid;
    // tsize
    topic_.tsize = pb_topic_.tsize;
    // connections_loc
    topic_.connections_loc = pb_topic_.connections_loc;
    // connections_ext
    topic_.connections_ext = pb_topic_.connections_ext;
    // message_drops
    topic_.message_drops = pb_topic_.message_drops;
    // did
    topic_.did = pb_topic_.did;
    // dclock
    topic_.dclock = pb_topic_.dclock;
    // dfreq
    topic_.dfreq = pb_topic_.dfreq;
  }

  bool decode_topics_field(pb_istream_t* stream, const pb_field_iter_t* /*field*/, void** arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    eCAL_pb_Topic pb_topic = eCAL_pb_Topic_init_default;
    eCAL::Monitoring::STopicMon topic{};

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
      monitoring->publisher.push_back(topic);
    }
    if (topic.direction == "subscriber")
    {
      monitoring->subscriber.push_back(topic);
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
    eCAL::Monitoring::SMethodMon method{};

    // decode method parameter
    eCAL::nanopb::decode_string(pb_method.mname, method.mname);
    eCAL::nanopb::decode_string(pb_method.req_type, method.req_type);
    eCAL::nanopb::decode_string(pb_method.req_desc, method.req_desc);
    eCAL::nanopb::decode_string(pb_method.resp_type, method.resp_type);
    eCAL::nanopb::decode_string(pb_method.resp_desc, method.resp_desc);

    // decode it
    if (!pb_decode(stream, eCAL_pb_Method_fields, &pb_method))
    {
      return false;
    }

    // apply method values
    method.call_count = pb_method.call_count;

    // add method to vector
    auto* method_vec = static_cast<std::vector<eCAL::Monitoring::SMethodMon>*>(*arg);
    method_vec->emplace_back(method);

    return true;
  }

  void decode_mon_service_methods(pb_callback_t& pb_callback, std::vector<eCAL::Monitoring::SMethodMon>& method_vec)
  {
    pb_callback.funcs.decode = &decode_mon_service_methods_field; // NOLINT(*-pro-type-union-access)
    pb_callback.arg = &method_vec;
  }

  void PrepareDecoding(eCAL_pb_Service& pb_service_, eCAL::Monitoring::SServerMon& service_)
  {
    // initialize
    pb_service_ = eCAL_pb_Service_init_default;

    ///////////////////////////////////////////////
    // assign decoder
    ///////////////////////////////////////////////
    // hname
    eCAL::nanopb::decode_string(pb_service_.hname, service_.hname);
    // pname
    eCAL::nanopb::decode_string(pb_service_.pname, service_.pname);
    // uname
    eCAL::nanopb::decode_string(pb_service_.uname, service_.uname);
    // sname
    eCAL::nanopb::decode_string(pb_service_.sname, service_.sname);
    // sid
    eCAL::nanopb::decode_string(pb_service_.sid, service_.sid);
    // methods
    decode_mon_service_methods(pb_service_.methods, service_.methods);
  }

  void AssignValues(const eCAL_pb_Service& pb_service_, eCAL::Monitoring::SServerMon& service_)
  {
    ///////////////////////////////////////////////
    // assign values
    ///////////////////////////////////////////////
    // rclock
    service_.rclock = pb_service_.rclock;
    // pid
    service_.pid = pb_service_.pid;
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
    eCAL::Monitoring::SServerMon service{};

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
    auto* services_vec = static_cast<std::vector<eCAL::Monitoring::SServerMon>*>(*arg);
    services_vec->push_back(service);

    return true;
  }

  /////////////////////////////////////////////////////////////////////////////////
  // Decode: eCAL::Monitoring::SClientMon
  /////////////////////////////////////////////////////////////////////////////////
  void PrepareDecoding(eCAL_pb_Client& pb_client_, eCAL::Monitoring::SClientMon& client_)
  {
    // initialize
    pb_client_ = eCAL_pb_Client_init_default;

    ///////////////////////////////////////////////
    // assign decoder
    ///////////////////////////////////////////////
    // hname
    eCAL::nanopb::decode_string(pb_client_.hname, client_.hname);
    // pname
    eCAL::nanopb::decode_string(pb_client_.pname, client_.pname);
    // uname
    eCAL::nanopb::decode_string(pb_client_.uname, client_.uname);
    // sname
    eCAL::nanopb::decode_string(pb_client_.sname, client_.sname);
    // sid
    eCAL::nanopb::decode_string(pb_client_.sid, client_.sid);
    // methods
    decode_mon_service_methods(pb_client_.methods, client_.methods);
  }

  void AssignValues(const eCAL_pb_Client& pb_client_, eCAL::Monitoring::SClientMon& client_)
  {
    ///////////////////////////////////////////////
    // assign values
    ///////////////////////////////////////////////
    // rclock
    client_.rclock = pb_client_.rclock;
    // pid
    client_.pid = pb_client_.pid;
    // version
    client_.version = pb_client_.version;
  }

  bool decode_clients_field(pb_istream_t* stream, const pb_field_iter_t* /*field*/, void** arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    eCAL_pb_Client pb_client = eCAL_pb_Client_init_default;
    eCAL::Monitoring::SClientMon client{};

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
    auto* client_vec = static_cast<std::vector<eCAL::Monitoring::SClientMon>*>(*arg);
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
    pb_mon_message.services.arg = &mon_message_.server;

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
