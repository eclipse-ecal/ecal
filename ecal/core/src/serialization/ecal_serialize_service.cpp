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
 * @file   ecal_serialize_service.cpp
 * @brief  eCAL service (de)serialization
**/

#include "nanopb/pb_encode.h"
#include "nanopb/pb_decode.h"
#include "nanopb/ecal/core/pb/ecal.npb.h"

#include "ecal_serialize_common.h"
#include "ecal_serialize_service.h"

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

namespace
{
  size_t RequestStruct2PbRequest(const eCAL::Service::Request& request_, eCAL_pb_Request& pb_request_)
  {
    pb_request_ = eCAL_pb_Request_init_default;

    ///////////////////////////////////////////////
    // header information
    ///////////////////////////////////////////////
    pb_request_.has_header = true;

    // host_name
    eCAL::nanopb::encode_string(pb_request_.header.host_name, request_.header.host_name);
    // service_name
    eCAL::nanopb::encode_string(pb_request_.header.service_name, request_.header.service_name);
    // service_id
    eCAL::nanopb::encode_string(pb_request_.header.service_id, request_.header.service_id);
    // method_name
    eCAL::nanopb::encode_string(pb_request_.header.method_name, request_.header.method_name);
    // error
    eCAL::nanopb::encode_string(pb_request_.header.error, request_.header.error);
    // id
    pb_request_.header.id = request_.header.id;
    // state
    pb_request_.header.state = static_cast<eCAL_pb_ServiceHeader_eCallState>(request_.header.state);

    ///////////////////////////////////////////////
    // request
    ///////////////////////////////////////////////
    eCAL::nanopb::encode_string(pb_request_.request, request_.request);

    ///////////////////////////////////////////////
    // evaluate byte size
    ///////////////////////////////////////////////
    pb_ostream_t pb_sizestream = { nullptr, nullptr, 0, 0, nullptr};
    pb_encode(&pb_sizestream, eCAL_pb_Request_fields, &pb_request_);

    // return encoding byte size
    return pb_sizestream.bytes_written;
  }

  template <typename T>
  bool RequestStruct2Buffer(const eCAL::Service::Request& request_, T& target_buffer_)
  {
    target_buffer_.clear();

    ///////////////////////////////////////////////
    // prepare request for encoding
    ///////////////////////////////////////////////
    eCAL_pb_Request pb_request = eCAL_pb_Request_init_default;
    size_t target_size = RequestStruct2PbRequest(request_, pb_request);

    ///////////////////////////////////////////////
    // encode it
    ///////////////////////////////////////////////
    target_buffer_.resize(target_size);
    pb_ostream_t pb_ostream;
    pb_ostream = pb_ostream_from_buffer((pb_byte_t*)(target_buffer_.data()), target_buffer_.size());
    if (!pb_encode(&pb_ostream, eCAL_pb_Request_fields, &pb_request))
    {
      std::cerr << "NanoPb eCAL::Service::Request encode failed: " << pb_ostream.errmsg << '\n';
    }
    else
    {
      return true;
    }

    return false;
  }

  bool Buffer2RequestStruct(const char* data_, size_t size_, eCAL::Service::Request& request_)
  {
    if (data_ == nullptr) return false;
    if (size_ == 0)       return false;

    // initialize
    eCAL_pb_Request pb_request = eCAL_pb_Request_init_default;

    ///////////////////////////////////////////////
    // assign decoder
    ///////////////////////////////////////////////

    ///////////////////////////////////////////////
    // header information
    ///////////////////////////////////////////////
    // host_name
    eCAL::nanopb::decode_string(pb_request.header.host_name, request_.header.host_name);
    // service_name
    eCAL::nanopb::decode_string(pb_request.header.service_name, request_.header.service_name);
    // service_id
    eCAL::nanopb::decode_string(pb_request.header.service_id, request_.header.service_id);
    // method_name
    eCAL::nanopb::decode_string(pb_request.header.method_name, request_.header.method_name);
    // error
    eCAL::nanopb::decode_string(pb_request.header.error, request_.header.error);

    ///////////////////////////////////////////////
    // request
    ///////////////////////////////////////////////
    // request
    eCAL::nanopb::decode_string(pb_request.request, request_.request);

    ///////////////////////////////////////////////
    // decode it
    ///////////////////////////////////////////////
    pb_istream_t pb_istream;
    pb_istream = pb_istream_from_buffer((pb_byte_t*)data_, size_); // NOLINT(*-pro-type-cstyle-cast)
    if (!pb_decode(&pb_istream, eCAL_pb_Request_fields, &pb_request))
    {
      std::cerr << "NanoPb eCAL::Service::Request decode failed: " << pb_istream.errmsg << '\n';
      return false;
    }

    ///////////////////////////////////////////////
    // assign values
    ///////////////////////////////////////////////

    ///////////////////////////////////////////////
    // header information
    ///////////////////////////////////////////////
    // id
    request_.header.id = pb_request.header.id;
    // state
    request_.header.state = static_cast<eCAL::Service::eMethodCallState>(pb_request.header.state);

    return true;
  }

  size_t ResponseStruct2PbResponse(const eCAL::Service::Response& response_, eCAL_pb_Response& pb_response_)
  {
    pb_response_ = eCAL_pb_Response_init_default;

    ///////////////////////////////////////////////
    // header information
    ///////////////////////////////////////////////
    pb_response_.has_header = true;

    // host_name
    eCAL::nanopb::encode_string(pb_response_.header.host_name, response_.header.host_name);
    // service_name
    eCAL::nanopb::encode_string(pb_response_.header.service_name, response_.header.service_name);
    // service_id
    eCAL::nanopb::encode_string(pb_response_.header.service_id, response_.header.service_id);
    // method_name
    eCAL::nanopb::encode_string(pb_response_.header.method_name, response_.header.method_name);
    // error
    eCAL::nanopb::encode_string(pb_response_.header.error, response_.header.error);
    // id
    pb_response_.header.id = response_.header.id;
    // state
    pb_response_.header.state = static_cast<eCAL_pb_ServiceHeader_eCallState>(response_.header.state);

    ///////////////////////////////////////////////
    // response
    ///////////////////////////////////////////////
    eCAL::nanopb::encode_string(pb_response_.response, response_.response);
    pb_response_.ret_state = response_.ret_state;

    ///////////////////////////////////////////////
    // evaluate byte size
    ///////////////////////////////////////////////
    pb_ostream_t pb_sizestream = { nullptr, nullptr, 0, 0, nullptr};
    pb_encode(&pb_sizestream, eCAL_pb_Response_fields, &pb_response_);

    // return encoding byte size
    return pb_sizestream.bytes_written;
  }

  template <typename T>
  bool ResponseStruct2Buffer(const eCAL::Service::Response& response_, T& target_buffer_)
  {
    target_buffer_.clear();

    ///////////////////////////////////////////////
    // prepare response for encoding
    ///////////////////////////////////////////////
    eCAL_pb_Response pb_response = eCAL_pb_Response_init_default;
    size_t target_size = ResponseStruct2PbResponse(response_, pb_response);

    ///////////////////////////////////////////////
    // encode it
    ///////////////////////////////////////////////
    target_buffer_.resize(target_size);
    pb_ostream_t pb_ostream;
    pb_ostream = pb_ostream_from_buffer((pb_byte_t*)(target_buffer_.data()), target_buffer_.size());
    if (!pb_encode(&pb_ostream, eCAL_pb_Response_fields, &pb_response))
    {
      std::cerr << "NanoPb eCAL::Service::Response encode failed: " << pb_ostream.errmsg << '\n';
    }
    else
    {
      return true;
    }

    return false;
  }

  bool Buffer2ResponseStruct(const char* data_, size_t size_, eCAL::Service::Response& response_)
  {
    if (data_ == nullptr) return false;
    if (size_ == 0)       return false;

    // initialize
    eCAL_pb_Response pb_response = eCAL_pb_Response_init_default;

    ///////////////////////////////////////////////
    // assign decoder
    ///////////////////////////////////////////////

    ///////////////////////////////////////////////
    // header information
    ///////////////////////////////////////////////
    // host_name
    eCAL::nanopb::decode_string(pb_response.header.host_name, response_.header.host_name);
    // service_name
    eCAL::nanopb::decode_string(pb_response.header.service_name, response_.header.service_name);
    // service_id
    eCAL::nanopb::decode_string(pb_response.header.service_id, response_.header.service_id);
    // method_name
    eCAL::nanopb::decode_string(pb_response.header.method_name, response_.header.method_name);
    // error
    eCAL::nanopb::decode_string(pb_response.header.error, response_.header.error);

    ///////////////////////////////////////////////
    // response
    ///////////////////////////////////////////////
    // response
    eCAL::nanopb::decode_string(pb_response.response, response_.response);

    ///////////////////////////////////////////////
    // decode it
    ///////////////////////////////////////////////
    pb_istream_t pb_istream;
    pb_istream = pb_istream_from_buffer((pb_byte_t*)data_, size_); // NOLINT(*-pro-type-cstyle-cast)
    if (!pb_decode(&pb_istream, eCAL_pb_Response_fields, &pb_response))
    {
      std::cerr << "NanoPb eCAL::Service::Response decode failed: " << pb_istream.errmsg << '\n';
      return false;
    }

    ///////////////////////////////////////////////
    // assign values
    ///////////////////////////////////////////////

    ///////////////////////////////////////////////
    // header information
    ///////////////////////////////////////////////
    // id
    response_.header.id = pb_response.header.id;
    // state
    response_.header.state = static_cast<eCAL::Service::eMethodCallState>(pb_response.header.state);

    ///////////////////////////////////////////////
    // response
    ///////////////////////////////////////////////
    response_.ret_state = pb_response.ret_state;

    return true;
  }
}

namespace eCAL
{
  namespace nanopb
  {
    // service request - serialize/deserialize
    bool SerializeToBuffer(const Service::Request& source_sample_, std::vector<char>& target_buffer_)
    {
      return RequestStruct2Buffer(source_sample_, target_buffer_);
    }

    bool SerializeToBuffer(const Service::Request& source_sample_, std::string& target_buffer_)
    {
      return RequestStruct2Buffer(source_sample_, target_buffer_);
    }

    bool DeserializeFromBuffer(const char* data_, size_t size_, Service::Request& target_sample_)
    {
      return Buffer2RequestStruct(data_, size_, target_sample_);
    }

    // service response - serialize/deserialize
    bool SerializeToBuffer(const Service::Response& source_sample_, std::vector<char>& target_buffer_)
    {
      return ResponseStruct2Buffer(source_sample_, target_buffer_);
    }

    bool SerializeToBuffer(const Service::Response& source_sample_, std::string& target_buffer_)
    {
      return ResponseStruct2Buffer(source_sample_, target_buffer_);
    }

    bool DeserializeFromBuffer(const char* data_, size_t size_, Service::Response& target_sample_)
    {
      return Buffer2ResponseStruct(data_, size_, target_sample_);
    }
  }
}

/// <summary>
/// PROTOZERO IMPLEMENTATION STARTS HERE!
/// </summary>
/// 

#include <ecal/core/pb/service.pbftags.h>
#include <protozero/pbf_writer.hpp>
#include <protozero/buffer_vector.hpp>
#include <protozero/pbf_reader.hpp>
#include <protozero/ecal_helper.h>

namespace {
  template <typename Writer>
  void SerializeServiceHeader(Writer& writer, const eCAL::Service::ServiceHeader& service_header)
  {
    writer.add_string(+eCAL::pb::ServiceHeader::optional_string_host_name, service_header.host_name);
    writer.add_string(+eCAL::pb::ServiceHeader::optional_string_service_name, service_header.service_name);
    writer.add_string(+eCAL::pb::ServiceHeader::optional_string_service_id, service_header.service_id);
    writer.add_string(+eCAL::pb::ServiceHeader::optional_string_method_name, service_header.method_name);
    writer.add_string(+eCAL::pb::ServiceHeader::optional_string_error, service_header.error);
    writer.add_int32(+eCAL::pb::ServiceHeader::optional_int32_id, service_header.id);
    writer.add_enum(+eCAL::pb::ServiceHeader::optional_enum_state, service_header.state); //TODO check enums!
  }
  
  void DeserializeServiceHeader(::protozero::pbf_reader& reader, eCAL::Service::ServiceHeader& service_header)
  {
    while (reader.next())
    {
      switch (reader.tag())
      {
      case +eCAL::pb::ServiceHeader::optional_string_host_name:
        AssignString(reader, service_header.host_name);
        break;
      case +eCAL::pb::ServiceHeader::optional_string_service_name:
        AssignString(reader, service_header.service_name);
        break;
      case +eCAL::pb::ServiceHeader::optional_string_service_id:
        AssignString(reader, service_header.service_id);
        break;
      case +eCAL::pb::ServiceHeader::optional_string_method_name:
        AssignString(reader, service_header.method_name);
        break;
      case +eCAL::pb::ServiceHeader::optional_string_error:
        AssignString(reader, service_header.error);
        break;
      case +eCAL::pb::ServiceHeader::optional_int32_id:
        service_header.id = reader.get_int32();
        break;
      case +eCAL::pb::ServiceHeader::optional_enum_state:
        service_header.state = (eCAL::Service::eMethodCallState)reader.get_enum(); // TODO check enums!
        break;
      default:
        reader.skip();
      }
    }
  }

  template <typename Writer>
  void SerializeServiceRequest(Writer& writer, const eCAL::Service::Request& service_request)
  {
    {
      Writer header_writer{ writer, +eCAL::pb::Request::optional_message_header };
      SerializeServiceHeader(header_writer, service_request.header);
    }
    writer.add_bytes(+eCAL::pb::Request::optional_bytes_request, service_request.request);
  }

  void DeserializeServiceRequest(::protozero::pbf_reader& reader, eCAL::Service::Request& service_request)
  {
    while (reader.next())
    {
      switch (reader.tag())
      {
      case +eCAL::pb::Request::optional_message_header:
        AssignMessage(reader, service_request.header, DeserializeServiceHeader);
        break;
      case +eCAL::pb::Request::optional_bytes_request:
        AssignBytes(reader, service_request.request);
        break;
      default:
        reader.skip();
      }
    }
  }

  template <typename Writer>
  void SerializeServiceResponse(Writer& writer, const eCAL::Service::Response& service_response)
  {
    {
      Writer header_writer{ writer, +eCAL::pb::Response::optional_message_header };
      SerializeServiceHeader(header_writer, service_response.header);
    }
    writer.add_bytes(+eCAL::pb::Response::optional_bytes_response, service_response.response);
    writer.add_int64(+eCAL::pb::Response::optional_int64_ret_state, service_response.ret_state);
  }

  void DeserializeServiceResponse(::protozero::pbf_reader& reader, eCAL::Service::Response& service_response)
  {
    while (reader.next())
    {
      switch (reader.tag())
      {
      case +eCAL::pb::Response::optional_message_header:
        AssignMessage(reader, service_response.header, DeserializeServiceHeader);
        break;
      case +eCAL::pb::Response::optional_bytes_response:
        AssignBytes(reader, service_response.response);
        break;
      case +eCAL::pb::Response::optional_int64_ret_state:
        service_response.ret_state = reader.get_int64();
        break;
      default:
        reader.skip();
      }
    }
  }
}

namespace eCAL
{
  namespace protozero
  {
    // service request - serialize/deserialize
    bool SerializeToBuffer(const eCAL::Service::Request& source_sample_, std::vector<char>& target_buffer_)
    {
      target_buffer_.clear();
      ::protozero::basic_pbf_writer<std::vector<char>> request_writer{ target_buffer_ };
      SerializeServiceRequest(request_writer, source_sample_);
      return true;
    }

    bool SerializeToBuffer(const eCAL::Service::Request& source_sample_, std::string& target_buffer_)
    {
      target_buffer_.clear();
      ::protozero::pbf_writer request_writer{ target_buffer_ };
      SerializeServiceRequest(request_writer, source_sample_);
      return true;
    }

    bool DeserializeFromBuffer(const char* data_, size_t size_, Service::Request& target_sample_)
    {
      try
      {
        target_sample_.clear();
        ::protozero::pbf_reader message{ data_, size_ };
        DeserializeServiceRequest(message, target_sample_);
        return true;
      }
      catch (const std::exception& exception)
      {
        LogDeserializationException(exception, "eCAL::Service::Request");
        return false;
      }
    }

    // service response - serialize/deserialize
    bool SerializeToBuffer(const Service::Response& source_sample_, std::vector<char>& target_buffer_)
    {
      target_buffer_.clear();
      ::protozero::basic_pbf_writer<std::vector<char>> response_writer{ target_buffer_ };
      SerializeServiceResponse(response_writer, source_sample_);
      return true;
    }

    bool SerializeToBuffer(const Service::Response& source_sample_, std::string& target_buffer_)
    {
      target_buffer_.clear();
      ::protozero::pbf_writer response_writer{ target_buffer_ };
      SerializeServiceResponse(response_writer, source_sample_);
      return true;
    }

    bool DeserializeFromBuffer(const char* data_, size_t size_, Service::Response& target_sample_)
    {
      try
      {
        target_sample_.clear();
        ::protozero::pbf_reader message{ data_, size_ };
        DeserializeServiceResponse(message, target_sample_);
        return true;
      }
      catch (const std::exception& exception)
      {
        LogDeserializationException(exception, "eCAL::Service::Response");
        return false;
      }
    }
  }
}
