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
 * @file   ecal_struct_service.h
 * @brief  eCAL service as struct
**/

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <tuple>

#include <ecal/types.h>
#include <util/expanding_vector.h>

namespace eCAL
{
  namespace Service
  {
    // eServiceCallState
    enum eMethodCallState
    {
      none     = 0,
      executed = 1,
      failed   = 2
    };

    // ServiceHeader
    struct ServiceHeader
    {
      std::string          host_name;       // Host name
      std::string          service_name;    // Service name
      std::string          service_id;             // Service id
      std::string          method_name;     // Method name
      std::string          error;           // Error message
      int32_t              id = 0;          // Session id
      eMethodCallState     state = none;    // Method call state

      bool operator==(const ServiceHeader& other) const {
        return host_name == other.host_name &&
          service_name == other.service_name &&
          service_id == other.service_id &&
          method_name == other.method_name &&
          error == other.error &&
          id == other.id &&
          state == other.state;
      }

      void clear()
      {
        host_name.clear();
        service_name.clear();
        service_id.clear();
        method_name.clear();
        error.clear();
        id = 0;
        state = none;
      }
    };

    // Service Request
    struct Request
    {
      ServiceHeader        header;          // Common service header
      std::string          request;         // Request payload

      bool operator==(const Request& other) const {
        return header == other.header &&
          request == other.request;
      }

      void clear()
      {
        header.clear();
        request.clear();
      }
    };

    // Service Response
    struct Response
    {
      ServiceHeader        header;           // Common service header
      std::string          response;         // Response payload
      int64_t              ret_state = 0;    // Callback return state

      bool operator==(const Response& other) const {
        return header == other.header &&
          response == other.response &&
          ret_state == other.ret_state;
      }

      void clear()
      {
        header.clear();
        response.clear();
        ret_state = 0;
      }
    };

    // Service Method
    struct Method
    {
      std::string          method_name;      // Method name

      std::string          req_type;         // Request  type       (deprecated use request_datatype_information)
      std::string          req_desc;         // Request  descriptor (deprecated use request_datatype_information)
      std::string          resp_type;        // Response type       (deprecated use response_datatype_information)
      std::string          resp_desc;        // Response descriptor (deprecated use response_datatype_information)

      SDataTypeInformation request_datatype_information;     // Request  datatype information (encoding & type & description)
      SDataTypeInformation response_datatype_information;    // Response datatype information (encoding & type & description)

      int64_t              call_count = 0;   // Call counter

      bool operator==(const Method& other) const {
        return method_name == other.method_name &&
          req_type == other.req_type &&
          req_desc == other.req_desc &&
          resp_type == other.resp_type &&
          resp_desc == other.resp_desc &&
          request_datatype_information == other.request_datatype_information &&
          response_datatype_information == other.response_datatype_information &&
          call_count == other.call_count;
      }

      void clear()
      {
        method_name.clear();
        req_type.clear();
        req_desc.clear();
        resp_type.clear();
        resp_desc.clear();
        call_count = 0;
      }
    };

    // Service
    // TODO: this naming is wrong, it should be Server!!!
    struct Service
    {
      int32_t                         registration_clock = 0;  // Registration clock
      std::string                     process_name;            // Process name
      std::string                     unit_name;               // Unit name
      std::string                     service_name;            // Service name
      Util::CExpandingVector<Method>  methods;                 // List of methods
      uint32_t                        version = 0;             // Service protocol version
      uint32_t                        tcp_port_v0 = 0;         // The TCP port used for that service (v0)
      uint32_t                        tcp_port_v1 = 0;         // The TCP port used for that service (v1)

      bool operator==(const Service& other) const {
        return registration_clock == other.registration_clock &&
          process_name == other.process_name &&
          unit_name == other.unit_name &&
          service_name == other.service_name &&
          methods == other.methods &&
          version == other.version &&
          tcp_port_v0 == other.tcp_port_v0 &&
          tcp_port_v1 == other.tcp_port_v1;
      }

      void clear()
      {
        registration_clock = 0;
        process_name.clear();
        unit_name.clear();
        service_name.clear();
        methods.clear();
        version = 0;
        tcp_port_v0 = 0;
        tcp_port_v1 = 0;
      }
    };

    // Client
    struct Client
    {
      int32_t                         registration_clock = 0;  // Registration clock
      std::string                     process_name;            // Process name
      std::string                     unit_name;               // Unit name
      std::string                     service_name;            // Service name
      Util::CExpandingVector<Method>  methods;                 // List of methods
      uint32_t                        version = 0;             // Client protocol version

      bool operator==(const Client& other) const {
        return registration_clock == other.registration_clock &&
          process_name == other.process_name &&
          unit_name == other.unit_name &&
          service_name == other.service_name &&
          methods == other.methods &&
          version == other.version;
      }

      void clear()
      {
        registration_clock = 0;
        process_name.clear();
        unit_name.clear();
        service_name.clear();
        methods.clear();
        version = 0;
      }
    };
  }
}
