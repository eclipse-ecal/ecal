// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#pragma once

#include <string>
#include <memory>

#include <stdint.h>

#include "tcpub_logger.h"

namespace tcpub
{
  // Foward-declare implementation
  class Executor_Impl;

  // Forward-declare friend classes
  class Publisher_Impl;
  class Subscriber_Impl;

  class Executor
  {
  public:
    Executor(size_t thread_count, const tcpub::logger::logger_t& log_function = tcpub::logger::default_logger);
    ~Executor();

    // Copy
    Executor(const Executor&)            = delete;
    Executor& operator=(const Executor&) = delete;

    // Move
    Executor& operator=(Executor&&)      = default;
    Executor(Executor&&)                 = default;

  private:
    friend ::tcpub::Publisher_Impl;
    friend ::tcpub::Subscriber_Impl;
    std::shared_ptr<Executor_Impl> executor_impl_;
  };
}
