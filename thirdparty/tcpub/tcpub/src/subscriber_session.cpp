// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#include <tcpub/subscriber_session.h>

#include "subscriber_session_impl.h"

namespace tcpub
{
  SubscriberSession::SubscriberSession(const std::shared_ptr<SubscriberSession_Impl>& impl)
    : subscriber_session_impl_(impl)
  {}

  SubscriberSession::~SubscriberSession()
  {
    subscriber_session_impl_->cancel();
  }

  std::string SubscriberSession::getAddress() const
    { return subscriber_session_impl_->getAddress(); }

  uint16_t    SubscriberSession::getPort()    const
    { return subscriber_session_impl_->getPort(); }

  void SubscriberSession::cancel()
    { subscriber_session_impl_->cancel(); }

  bool SubscriberSession::isConnected() const
    { return subscriber_session_impl_->isConnected(); }
}
