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

#pragma once

#include <string>

#include <ecal/log_level.h>

namespace eCAL
{
  namespace Logging
  {
    struct SProviderAttributes
    {
      struct SSink
      {
        bool                enabled;
        Filter              filter_log;
      };

      struct SUDP
      {
        std::string address;
        int         port;
        int         ttl;
        bool        broadcast;
        bool        loopback;
        int         sndbuf;
      };

      struct SFile
      {
        std::string          path;
      };

      SSink                  udp_sink;
      SSink                  file_sink;
      SSink                  console_sink;

      SUDP                   udp_config;
      SFile                  file_config;

      int                    process_id;
      std::string            host_name;
      std::string            process_name;
      std::string            unit_name;
    };
  }
}