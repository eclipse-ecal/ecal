// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#pragma once

#include <memory>
#include <string>
#include <stdint.h>

namespace tcpub
{
  // Forward-declare Implementation
  class SubscriberSession_Impl;

  // Friend class
  class Subscriber_Impl;

  class SubscriberSession
  {
  friend Subscriber_Impl;

  private:
    SubscriberSession(const std::shared_ptr<SubscriberSession_Impl>& impl);

  public:
    // Copy
    SubscriberSession(const SubscriberSession&)            = default;
    SubscriberSession& operator=(const SubscriberSession&) = default;

    // Move
    SubscriberSession& operator=(SubscriberSession&&)      = default;
    SubscriberSession(SubscriberSession&&)                 = default;

    // Destructor
    ~SubscriberSession();

  public:
    std::string getAddress() const;
    uint16_t    getPort()    const;

    void        cancel();
    bool        isConnected() const;

  private:
    const std::shared_ptr<SubscriberSession_Impl> subscriber_session_impl_;
  };
}