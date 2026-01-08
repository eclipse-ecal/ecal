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
 * @file   ecal_serialize_service.cpp
 * @brief  eCAL service (de)serialization
**/

#include "ecal_serialize_common.h"
#include "ecal_serialize_service.h"

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

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
