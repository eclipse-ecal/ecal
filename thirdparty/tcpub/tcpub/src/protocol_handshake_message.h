// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#pragma once

#include <stdint.h>

namespace tcpub
{
#pragma pack(push,1)
  // This message shall always contain little endian numbers.
  struct ProtocolHandshakeMessage
  {
    uint8_t           protocol_version     = 0;
  };
#pragma pack(pop)
}
