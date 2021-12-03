// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#include "subscriber_impl.h"

#include "tcp_header.h"
#include "portable_endian.h"
#include "subscriber_session_impl.h"
#include "executor_impl.h"

namespace tcpub
{
  ////////////////////////////////////////////////
  // Constructor & Destructor
  ////////////////////////////////////////////////
  Subscriber_Impl::Subscriber_Impl(const std::shared_ptr<Executor>& executor)
    : executor_                    (executor)
    , user_callback_is_synchronous_(true)
    , synchronous_user_callback_   ([](const auto&){})
    , callback_thread_stop_        (true)
    , log_                         (executor_->executor_impl_->logFunction())
  {}

  // Destructor
  Subscriber_Impl::~Subscriber_Impl()
  {
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    std::stringstream ss;
    ss << std::this_thread::get_id();
    std::string thread_id = ss.str();
    log_(logger::LogLevel::DebugVerbose, "Subscriber " + subscriberIdString() + ": Deleting from thread " + thread_id + "...");
#endif

#if (TCPUB_LOG_DEBUG_ENABLED)
    log_(logger::LogLevel::Debug, "Subscriber " + subscriberIdString() + ": Deleted.");
#endif
  }

  ////////////////////////////////////////////////
  // Session Management
  ////////////////////////////////////////////////
  std::shared_ptr<SubscriberSession> Subscriber_Impl::addSession(const std::string& address, uint16_t port, int max_reconnection_attempts)
  {
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    log_(logger::LogLevel::DebugVerbose, "Subscriber " + subscriberIdString() + ": Adding session for endpoint " + address + ":" + std::to_string(port) + ".");
#endif

    // Function for getting a free buffer
    std::function<std::shared_ptr<std::vector<char>>()> get_free_buffer_handler
            = [me = shared_from_this()]() -> std::shared_ptr<std::vector<char>>
              {
                return me->buffer_pool_.allocate();
              };

    // Function for cleaning up
    std::function<void(const std::shared_ptr<SubscriberSession_Impl>&)> subscriber_session_closed_handler
            = [me = shared_from_this()](const std::shared_ptr<SubscriberSession_Impl>& subscriber_session_impl) -> void
              {
#if (TCPUB_LOG_DEBUG_ENABLED)
                  me->log_(logger::LogLevel::Debug, "Subscriber " + me->subscriberIdString() + ": Removing session " + subscriber_session_impl->remoteEndpointToString() + ".");
#endif
                std::lock_guard<std::mutex>session_list_lock(me->session_list_mutex_);

                // Look up the current subscriber session and remove it from the list
                auto session_it = std::find_if(me->session_list_.begin()
                                              , me->session_list_.end()
                                              , [&subscriber_session_impl] (const std::shared_ptr<SubscriberSession>& session) ->bool
                                                { return subscriber_session_impl == session->subscriber_session_impl_; });
                if (session_it != me->session_list_.end())
                {
                  me->session_list_.erase(session_it);
#if (TCPUB_LOG_DEBUG_ENABLED)
                  me->log_(logger::LogLevel::Debug, "Subscriber " + me->subscriberIdString() + ": Current number of sessions: " + std::to_string(me->session_list_.size()));
#endif
                }
                else
                {
                  // This can never happen, unless I screwed up while implementing this
                  me->log_(logger::LogLevel::Fatal, "Subscriber " + me->subscriberIdString() + ": Error removing subscriber: The subscriber does not exist");
                }
              };

    // Create a new Subscriber Session. Unfortunatelly we cannot use
    // ::std::make_shared here, as the constructor is private and make_shared
    // cannot access it. Thus, we crate the object manually with new.
    std::shared_ptr<SubscriberSession> subscriber_session(
       new SubscriberSession(std::make_shared<SubscriberSession_Impl>(executor_->executor_impl_->ioService()
                                                                    , address
                                                                    , port
                                                                    , max_reconnection_attempts
                                                                    , get_free_buffer_handler
                                                                    , subscriber_session_closed_handler
                                                                    , log_)));

    setCallbackToSession(subscriber_session);

    {
      std::lock_guard<std::mutex> session_list_lock(session_list_mutex_);
      session_list_.push_back(subscriber_session);
      subscriber_session->subscriber_session_impl_->start();
    }

    return subscriber_session;
  }

  std::vector<std::shared_ptr<SubscriberSession>> Subscriber_Impl::getSessions() const
  {
    std::lock_guard<std::mutex> session_list_lock(session_list_mutex_);
    return session_list_;
  }

  void Subscriber_Impl::setCallback(const std::function<void(const CallbackData& callback_data)>& callback_function, bool synchronous_execution)
  {
#if (TCPUB_LOG_DEBUG_ENABLED)
    log_(logger::LogLevel::Debug, "Subscriber " + subscriberIdString() + ": Setting new " + (synchronous_execution ? "synchronous" : "asynchronous") + " callback.");
#endif

    // Stop and remove the old callback thread at first
    if (callback_thread_)
    {
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
      log_(logger::LogLevel::DebugVerbose, "Subscriber " + subscriberIdString() + ": Stopping old callback thread...");
#endif
      callback_thread_stop_ = true;
      last_callback_data_cv_.notify_all();

      // Join or detach the old thread. We cannot join a thread from it's own
      // thread, so we detach the thread in that case.
      if (std::this_thread::get_id() == callback_thread_->get_id())
        callback_thread_->detach();
      else
        callback_thread_->join();

      callback_thread_.reset();
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
      log_(logger::LogLevel::DebugVerbose, "Subscriber " + subscriberIdString() + ": Old callback thread has terminated.");
#endif
    }

    const bool renew_synchronous_callbacks = (synchronous_execution || user_callback_is_synchronous_);

    if (synchronous_execution)
    {
      // Save the callback as member variable. We need to pass it to all new sessions.
      synchronous_user_callback_    = callback_function;
      user_callback_is_synchronous_ = synchronous_execution;

      // Clean the last callback data, so any buffer in there is freed
      std::unique_lock<std::mutex> callback_lock(last_callback_data_mutex_);
      last_callback_data_ = CallbackData();
    }
    if (!synchronous_execution)
    {
      synchronous_user_callback_    = [](const auto&) {};
      user_callback_is_synchronous_ = synchronous_execution;

      // Create a new callback thread with the new callback from the function parameter
      callback_thread_stop_ = false;
      callback_thread_ = std::make_unique<std::thread>(
                    [me = shared_from_this(), callback_function]()
                    {
                      for (;;)
                      {
                        CallbackData this_callback_data; // create empty callback data

                        {
                          // Lock callback mutex and wait for valid data. Wake up if the callback data contains valid data or the user set a synchronous callback. In the latter case, we exit the thread.
                          std::unique_lock<std::mutex> callback_lock(me->last_callback_data_mutex_);
                          me->last_callback_data_cv_.wait(callback_lock, [&me]() -> bool { return bool(me->last_callback_data_.buffer_) || me->callback_thread_stop_; });

                          // Exit if the user has set a synchronous callback
                          if (me->callback_thread_stop_) return;

                          std::swap(this_callback_data, me->last_callback_data_); // Now an empty callback data is in "last_callback_data" again
                        }

#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
                        me->log_(logger::LogLevel::DebugVerbose, "Subscriber " + me->subscriberIdString() + ": Executing asynchronous callback");
#endif            
                        // Execute the user callback. Note that the callback mutex is not locked any more, so while the expensive user callback is executed, our tcp sessions can already store new data.
                        callback_function(this_callback_data);
                      }
                    });
    }

    //std::lock_guard<std::mutex> callback_lock(callback_mutex_);
    if (renew_synchronous_callbacks)
    {
      std::lock_guard<std::mutex> session_list_lock(session_list_mutex_);
      for (const auto& session : session_list_)
      {
        setCallbackToSession(session);
      }
    }
  }

  void Subscriber_Impl::setCallbackToSession(const std::shared_ptr<SubscriberSession>& session)
  {
    if (user_callback_is_synchronous_)
    {
      session->subscriber_session_impl_->setSynchronousCallback(
                [callback = synchronous_user_callback_, me = shared_from_this()](const std::shared_ptr<std::vector<char>>& buffer, const std::shared_ptr<TcpHeader>& /*header*/)->void
                {
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
                  me->log_(logger::LogLevel::DebugVerbose, "Subscriber " + me->subscriberIdString() + ": Executing synchronous callback");
#endif            
                  std::lock_guard<std::mutex> callback_lock(me->last_callback_data_mutex_);
                  if (me->user_callback_is_synchronous_)
                  {
                    CallbackData callback_data;
                    callback_data.buffer_           = buffer;
                    callback(callback_data);
                  }
                });
    }
    else
    {
      session->subscriber_session_impl_->setSynchronousCallback(
                [me = shared_from_this()](const std::shared_ptr<std::vector<char>>& buffer, const std::shared_ptr<TcpHeader>& /*header*/)->void
                {
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
                  me->log_(logger::LogLevel::DebugVerbose, "Subscriber " + me->subscriberIdString() + ": Storing data for  asynchronous callback");
#endif            
                  std::lock_guard<std::mutex> callback_lock(me->last_callback_data_mutex_);
                  if (!me->user_callback_is_synchronous_)
                  {
                    me->last_callback_data_.buffer_           = buffer;

                    me->last_callback_data_cv_.notify_all();
                  }
                });
    }
  }

  void Subscriber_Impl::cancel()
  {
#if (TCPUB_LOG_DEBUG_ENABLED)
    log_(logger::LogLevel::Debug, "Subscriber " + subscriberIdString() + ": Cancelling...");
#endif

    {
      std::lock_guard<std::mutex> session_list_lock(session_list_mutex_);
      for (const auto& session : session_list_)
      {
        session->cancel();
      }
    }

    // Stop and remove the old callback thread at first
    if (callback_thread_)
    {
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
      log_(logger::LogLevel::DebugVerbose, "Subscriber " + subscriberIdString() + ": Stopping callback thread...");
#endif
      callback_thread_stop_ = true;
      last_callback_data_cv_.notify_all();

      // Join or detach the old thread. We cannot join a thread from it's own
      // thread, so we detach the thread in that case.
      if (std::this_thread::get_id() == callback_thread_->get_id())
        callback_thread_->detach();
      else
        callback_thread_->join();

      callback_thread_.reset();
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
      log_(logger::LogLevel::DebugVerbose, "Subscriber " + subscriberIdString() + ": Callback thread has terminated.");
#endif
    }

    // Delete the user callback
    synchronous_user_callback_    = [](const auto&){};
    user_callback_is_synchronous_ = true;
  }

  std::string Subscriber_Impl::subscriberIdString() const
  {
    std::stringstream ss;
    ss << "0x" << std::hex << this;
    return ss.str();
  }
}
