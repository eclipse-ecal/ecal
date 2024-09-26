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
 * @file   ecal_struct_service.h
 * @brief  eCAL service as struct
**/

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <tuple>

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
      std::string          hname;           // Host name
      std::string          sname;           // Service name
      std::string          sid;             // Service id
      std::string          mname;           // Method name
      std::string          error;           // Error message
      int32_t              id = 0;          // Session id
      eMethodCallState     state = none;    // Method call state

      bool operator==(const ServiceHeader& other) const {
        return hname == other.hname &&
          sname == other.sname &&
          sid == other.sid &&
          mname == other.mname &&
          error == other.error &&
          id == other.id &&
          state == other.state;
      }

      void clear()
      {
        hname.clear();
        sname.clear();
        sid.clear();
        mname.clear();
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
      std::string          mname;            // Method name
      std::string          req_type;         // Request type
      std::string          req_desc;         // Request descriptor
      std::string          resp_type;        // Response type
      std::string          resp_desc;        // Response descriptor
      int64_t              call_count = 0;   // Call counter

      bool operator==(const Method& other) const {
        return mname == other.mname &&
          req_type == other.req_type &&
          req_desc == other.req_desc &&
          resp_type == other.resp_type &&
          resp_desc == other.resp_desc &&
          call_count == other.call_count;
      }

      void clear()
      {
        mname.clear();
        req_type.clear();
        req_desc.clear();
        resp_type.clear();
        resp_desc.clear();
        call_count = 0;
      }
    };

    // Service
    struct Service
    {
      int32_t                         rclock = 0;       // Registration clock
      std::string                     pname;            // Process name
      std::string                     uname;            // Unit name
      std::string                     sname;            // Service name
      Util::CExpandingVector<Method>  methods;          // List of methods
      uint32_t                        version = 0;      // Service protocol version
      uint32_t                        tcp_port_v0 = 0;  // The TCP port used for that service (v0)
      uint32_t                        tcp_port_v1 = 0;  // The TCP port used for that service (v1)

      bool operator==(const Service& other) const {
        return rclock == other.rclock &&
          pname == other.pname &&
          uname == other.uname &&
          sname == other.sname &&
          methods == other.methods &&
          version == other.version &&
          tcp_port_v0 == other.tcp_port_v0 &&
          tcp_port_v1 == other.tcp_port_v1;
      }

      void clear()
      {
        rclock = 0;
        pname.clear();
        uname.clear();
        sname.clear();
        methods.clear();
        version = 0;
        tcp_port_v0 = 0;
        tcp_port_v1 = 0;
      }
    };

    // Client
    struct Client
    {
      int32_t                         rclock = 0;       // Registration clock
      std::string                     pname;            // Process name
      std::string                     uname;            // Unit name
      std::string                     sname;            // Service name
      Util::CExpandingVector<Method>  methods;          // List of methods
      uint32_t                        version = 0;      // Client protocol version

      bool operator==(const Client& other) const {
        return rclock == other.rclock &&
          pname == other.pname &&
          uname == other.uname &&
          sname == other.sname &&
          methods == other.methods &&
          version == other.version;
      }

      void clear()
      {
        rclock = 0;
        pname.clear();
        uname.clear();
        sname.clear();
        methods.clear();
        version = 0;
      }
    };
  }
}
