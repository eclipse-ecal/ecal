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

#pragma once

#include <cstdint>

namespace eCAL
{
  namespace service
  {
    // Message type used since protocol version 1
    enum class MessageType: std::uint8_t
    {
      Undefined                 = 0,
      ProtocolHandshakeRequest  = 1,
      ProtocolHandshakeResponse = 2,
      ServiceRequest            = 3,
      ServiceResponse           = 4,
    };

#pragma pack(push, 1)
    struct TcpHeaderV0
    {
      std::uint32_t package_size_n = 0;                        // package size in network byte order
      std::uint32_t reserved1      = 0;                        // reserved
      std::uint64_t reserved2      = 0;                        // reserved
    };

    // TCP Header
    //   - Used for service request since protocol version 1
    //   - Used for response since protocol version 0
    struct TcpHeaderV1
    {
      std::uint32_t package_size_n = 0;                        // package size in network byte order
      std::uint8_t  version        = 0;                        // protocol version                    (since protocol V1 / eCAL 5.12)
      MessageType   message_type   = MessageType::Undefined;   // message type                        (since protocol V1 / eCAL 5.12)
      std::uint16_t header_size_n  = 0;                        // header size in network byte order   (since protocol V1 / eCAL 5.12)
      std::uint64_t reserved       = 0;                        // reserved
    };

    // Handshake Request Message, since protocol v1
    struct ProtocolHandshakeRequestMessage
    {
      std::uint8_t min_supported_protocol_version = 0;
      std::uint8_t max_supported_protocol_version = 0;
    };

    // Handshake Response Message, since protocol v1
    struct ProtocolHandshakeResponseMessage
    {
      std::uint8_t accepted_protocol_version = 0;
    };
#pragma pack(pop)

  } // namespace service
} // namespace eCAL
