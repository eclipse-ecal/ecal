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

#pragma once

#include <string>

#include <ecal/ecal_log_level.h>

namespace eCAL
{
  namespace Logging
  {
    struct SUDP
    {
      bool                enabled;
      unsigned int        port;
      eCAL_Logging_Filter filter_log;
    };

    struct SFile
    {
      bool                enabled;
      std::string         path;
      eCAL_Logging_Filter filter_log;
    };

    struct SConsole
    {
      bool                enabled;
      eCAL_Logging_Filter filter_log;
    };

    struct SUDPSender
    {
      std::string address;
      int         port;
      int         ttl;
      bool        broadcast;
      bool        loopback;
      int         sndbuf;
    };

    struct SUDPReceiver
    {
      std::string address;
      int         port;
      bool        broadcast;
      bool        loopback;
      int         rcvbuf;
    };

    struct SAttributes
    {
      SUDP                   udp;
      SFile                  file;
      SConsole               console;

      SUDPSender             udp_sender;
      SUDPReceiver           udp_receiver;

      int                    process_id;
      bool                   network_enabled;
      std::string            host_name;
      std::string            process_name;
      std::string            unit_name;
      eCAL_Logging_eLogLevel level;
    };
  }
}