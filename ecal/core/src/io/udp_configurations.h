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
 * @brief  common configurations for eCAL UDP communication
**/

#pragma once

#include <string>

namespace eCAL
{
  namespace UDP
  {
    // return local broadcast address
    std::string LocalBroadcastAddress();

    // return the multicast adress/port used for sending/receiving the registration information
    std::string GetRegistrationMulticastAddress();
    int GetRegistrationPort();

    // return the multicast adress/port used for sending/receiving the logging information
    std::string GetLoggingMulticastAddress();
    int GetLoggingPort();

    // return the multicast adress/port used for sending/receiving the topic payload
    std::string GetPayloadMulticastAddress(const std::string& topic_name);
    int GetPayloadPort();

    // return multicast udp package time to live setting
    int GetMulticastTtl();
  }
}
