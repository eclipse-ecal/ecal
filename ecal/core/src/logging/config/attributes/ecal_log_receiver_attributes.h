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

#include <ecal/log_level.h>

namespace eCAL
{
  namespace Logging
  {
    struct SReceiverAttributes
    {
      struct SUDPReceiver
      {
        std::string address;
        int         port;
        bool        broadcast;
        bool        loopback;
        int         rcvbuf;
      };
      
      SUDPReceiver udp_receiver;

      bool         network_enabled;
      bool         receive_enabled;
      std::string  host_name;
   };
  }
}