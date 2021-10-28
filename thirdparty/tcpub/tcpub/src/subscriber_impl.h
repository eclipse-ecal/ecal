// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#pragma once

#include <memory>
#include <string>
#include <mutex>
#include <vector>
#include <functional>

#include <asio.hpp>
#include <recycle/shared_pool.hpp>

#include <tcpub/executor.h>
#include <tcpub/subscriber_session.h>
#include <tcpub/callback_data.h>

#include "tcpub_logger_abstraction.h"

namespace tcpub
{
  class Subscriber_Impl : public std::enable_shared_from_this<Subscriber_Impl>
  {
  ////////////////////////////////////////////////
  // Constructor & Destructor
  ////////////////////////////////////////////////
  public:
    // Constructor
    Subscriber_Impl(const std::shared_ptr<Executor>& executor);

    // Copy
    Subscriber_Impl(const Subscriber_Impl&)            = delete;
    Subscriber_Impl& operator=(const Subscriber_Impl&) = delete;

    // Move
    Subscriber_Impl& operator=(Subscriber_Impl&&)      = delete;
    Subscriber_Impl(Subscriber_Impl&&)                 = delete;

    // Destructor
    ~Subscriber_Impl();

  ////////////////////////////////////////////////
  // Session Management
  ////////////////////////////////////////////////
  public: 
    std::shared_ptr<SubscriberSession>              addSession(const std::string& address, uint16_t port, int max_reconnection_attempts);
    std::vector<std::shared_ptr<SubscriberSession>> getSessions() const;

    void setCallback(const std::function<void(const CallbackData& callback_data)>callback_function, bool synchronous_execution);
  private:
    void setCallbackToSession(const std::shared_ptr<SubscriberSession>& session);

  public:
    void cancel();

  private:
    std::string subscriberIdString() const;

  ////////////////////////////////////////////////
  // Member variables
  ////////////////////////////////////////////////
  private:
    // Asio
    const std::shared_ptr<Executor>                 executor_;                 /// Global Executor

    // List of all Sessions
    mutable std::mutex                              session_list_mutex_;
    std::vector<std::shared_ptr<SubscriberSession>> session_list_;

    // Callback
    mutable std::mutex                              last_callback_data_mutex_;
    std::condition_variable                         last_callback_data_cv_;
    CallbackData                                    last_callback_data_;

    std::atomic<bool>                               user_callback_is_synchronous_;
    std::function<void(const CallbackData&)>        synchronous_user_callback_;

    std::unique_ptr<std::thread>                    callback_thread_;
    std::atomic<bool>                               callback_thread_stop_;

    // Buffer pool
    struct buffer_pool_lock_policy_
    {
      using mutex_type = std::mutex;
      using lock_type  = std::lock_guard<mutex_type>;
    };
    recycle::shared_pool<std::vector<char>, buffer_pool_lock_policy_> buffer_pool_;                 /// Buffer pool that let's us reuse memory chunks

    // Log function
    const tcpub::logger::logger_t log_;
  };
}