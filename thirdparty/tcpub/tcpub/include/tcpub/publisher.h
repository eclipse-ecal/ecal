// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#pragma once

#include <stdint.h>

#include <memory>
#include <string>
#include <chrono>
#include <vector>

#include "executor.h"

namespace tcpub
{
  class Publisher_Impl;

  class Publisher
  {
  public:
    Publisher(const std::shared_ptr<Executor>& executor, const std::string& address, uint16_t port); // TODO: Check what happens if the port cannot be opened
    Publisher(const std::shared_ptr<Executor>& executor, uint16_t port = 0);

    // Copy
    Publisher(const Publisher&)            = default;
    Publisher& operator=(const Publisher&) = default;

    // Move
    Publisher& operator=(Publisher&&)      = default;
    Publisher(Publisher&&)                 = default;

    // Destructor
    ~Publisher();

  public:
    uint16_t           getPort()            const;
    size_t             getSubscriberCount() const;

    bool send(const char* const data, size_t size) const;
    bool send(const std::vector<std::pair<const char* const, const size_t>>&) const;

    void cancel();

  private:
    std::shared_ptr<Publisher_Impl> publisher_impl_;
  };
}