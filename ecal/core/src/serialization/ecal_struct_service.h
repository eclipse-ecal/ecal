/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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
    };

    // Service Request
    struct Request
    {
      ServiceHeader        header;          // Common service header
      std::string          request;         // Request payload
    };

    // Service Response
    struct Response
    {
      ServiceHeader        header;           // Common service header
      std::string          response;         // Response payload
      int64_t              ret_state = 0;    // Callback return state
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
    };

    // Service
    struct Service
    {
      int32_t              rclock = 0;       // Registration clock
      std::string          hname;            // Host name
      std::string          pname;            // Process name
      std::string          uname;            // Unit name
      int32_t              pid = 0;          // Process id
      std::string          sname;            // Service name
      std::string          sid;              // Service id
      std::vector<Method>  methods;          // List of methods
      uint32_t             version = 0;      // Service protocol version
      uint32_t             tcp_port_v0 = 0;  // The TCP port used for that service (v0)
      uint32_t             tcp_port_v1 = 0;  // The TCP port used for that service (v1)
    };

    // Client
    struct Client
    {
      int32_t              rclock = 0;       // Registration clock
      std::string          hname;            // Host name
      std::string          pname;            // Process name
      std::string          uname;            // Unit name
      int32_t              pid = 0;          // Process id
      std::string          sname;            // Service name
      std::string          sid;              // Service id
      uint32_t             version = 0;      // Client protocol version
    };
  }
}
