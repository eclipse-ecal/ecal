// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#pragma once

#include <stdint.h>

namespace tcpub
{
  enum class MessageContentType : uint8_t
  {
    RegularPayload    = 0, // The Content is a user-defined payload that shall be given to the user code
    ProtocolHandshake = 1, // The contnet is a handshake message that defines which protocol version shall be used

    // This is meant for future use. At the moment, received messages that don't
    // have the type set to "RegularPayload" are discarded. So in the future,
    // e.g. when we decide to send meta information or anything else over the 
    // same socket, old subscribers will not try to interpret those messages
    // as payload.
  };
  
#pragma pack(push,1)

  // This Header shall always contain little endian numbers.
  struct TcpHeader
  {
    uint16_t           header_size     = 0;
    MessageContentType type            = MessageContentType::RegularPayload;
    uint8_t            reserved        = 0;                                   // Added for 32bit-alignment. Can later be reused e.g. as a flag field or similar.
    uint64_t           data_size       = 0;
  };

#pragma pack(pop)

}