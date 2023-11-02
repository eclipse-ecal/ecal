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
    // these functions return specific multicast address for registration / logging and payload transport
    // in the local host mode (network_enabled = false) they all return LocalHost()

    // return the local host udp address
    std::string LocalHost();

    // return the multicast adress used for sending/receiving the registration information
    std::string GetRegistrationMulticastAddress();

    // return the multicast adress used for sending/receiving the logging information
    std::string GetLoggingMulticastAddress();

    // return the multicast adress used for sending/receiving the topic payload
    std::string GetTopicMulticastAddress(const std::string& topic_name);
  }
}
