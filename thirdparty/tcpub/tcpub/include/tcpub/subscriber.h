// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#pragma once

#include <stdint.h>

#include <memory>
#include <chrono>
#include <string>

#include "executor.h"
#include "subscriber_session.h"
#include "callback_data.h"

namespace tcpub
{
  class Subscriber_Impl;

  class Subscriber
  {
  public:
    Subscriber(const std::shared_ptr<Executor>& executor);

    // Copy
    Subscriber(const Subscriber&)            = default;
    Subscriber& operator=(const Subscriber&) = default;

    // Move
    Subscriber& operator=(Subscriber&&)      = default;
    Subscriber(Subscriber&&)                 = default;

    // Destructor
    ~Subscriber();

  public:
    std::shared_ptr<SubscriberSession>              addSession(const std::string& address, uint16_t port, int max_reconnection_attempts = -1);
    std::vector<std::shared_ptr<SubscriberSession>> getSessions() const;

    void setCallback  (const std::function<void(const CallbackData& callback_data)>callback_function, bool synchronous_execution = false);
    void clearCallback();

    void cancel();

  private:
    std::shared_ptr<Subscriber_Impl> subscriber_impl_;
  };
}
