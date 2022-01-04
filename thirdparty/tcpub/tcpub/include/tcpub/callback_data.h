// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#pragma once

#include <vector>
#include <chrono>
#include <memory>
#include <stdint.h>

namespace tcpub
{
  struct CallbackData
  {
    // At the moment, this callback data only holds the buffer. But just in case
    // The tcp subsriber would want to return more than that in a later version,
    // we use this struct to improve API stability.

    std::shared_ptr<std::vector<char>> buffer_;
  };
}
