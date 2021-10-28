// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#include "publisher_impl.h"

#include "tcp_header.h"
#include "portable_endian.h"

#include "executor_impl.h"

namespace tcpub
{

  ////////////////////////////////////////////////
  // Constructor & Destructor
  ////////////////////////////////////////////////
  
  // Constructor
  Publisher_Impl::Publisher_Impl(const std::shared_ptr<Executor>& executor)
    : is_running_     (false)
    , executor_       (executor)
    , acceptor_       (*executor_->executor_impl_->ioService())
    , log_            (executor_->executor_impl_->logFunction())
  {}

  // Destructor
  Publisher_Impl::~Publisher_Impl()
  {
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    std::stringstream ss;
    ss << std::this_thread::get_id();
    std::string thread_id = ss.str();
    log_(logger::LogLevel::DebugVerbose, "Publisher " + localEndpointToString() + ": Deleting from thread " + thread_id + "...");
#endif

    if (is_running_)
    {
      cancel();
    }

#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    log_(logger::LogLevel::DebugVerbose, "Publisher " + localEndpointToString() + ": Deleted.");
#endif
  }

  ////////////////////////////////////////////////
  // Start & Stop
  ////////////////////////////////////////////////
  
  bool Publisher_Impl::start(const std::string& address, uint16_t port)
  {
    // TODO: Return false if already running
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    log_(logger::LogLevel::DebugVerbose, "Publisher: Parsing address " + address + ":" + std::to_string(port) + ".");
#endif

    // set up the acceptor to listen on the tcp port
    asio::error_code make_address_ec;
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address(address, make_address_ec), port);
    if (make_address_ec)
    {
      log_(logger::LogLevel::Error,  "Publisher: Error parsing address \"" + address + ":" + std::to_string(port) + "\": " + make_address_ec.message());
      return false;
    }
    
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    log_(logger::LogLevel::DebugVerbose, "Publisher " + toString(endpoint) + ": Opening acceptor.");
#endif

    {
      asio::error_code ec;
      acceptor_.open(endpoint.protocol(), ec);
      if (ec)
      {
        log_(logger::LogLevel::Error, "Publisher " + toString(endpoint) + ": Error opening acceptor: " + ec.message());
        return false;
      }
    }

#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    log_(logger::LogLevel::DebugVerbose, "Publisher " + toString(endpoint) + ": Setting \"reuse_address\" option.");
#endif

    {
      asio::error_code ec;
      acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true), ec);
      if (ec)
      {
        log_(logger::LogLevel::Error, "Publisher " + toString(endpoint) + ": Error setting reuse_address option : " + ec.message());
        return false;
      }
    }

#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    log_(logger::LogLevel::DebugVerbose, "Publisher " + toString(endpoint) + ": Binding acceptor to the endpoint.");
#endif

    {
      asio::error_code ec;
      acceptor_.bind(endpoint, ec);
      if (ec)
      {
        log_(logger::LogLevel::Error, "Publisher " + toString(endpoint) + ": Error binding acceptor: " + ec.message());
        return false;
      }
    }

#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    log_(logger::LogLevel::DebugVerbose, "Publisher " + toString(endpoint) + ": Listening on acceptor.");
#endif

    {
      asio::error_code ec;
      acceptor_.listen(asio::socket_base::max_listen_connections, ec);
      if (ec)
      {
        log_(logger::LogLevel::Error, "Publisher " + toString(endpoint) + ": Error listening on acceptor: " + ec.message());
        return false;
      }
    }

    log_(logger::LogLevel::Info, "Publisher " + toString(endpoint) + ": Created publisher and waiting for clients.");

    is_running_ = true;

    acceptClient();

    return true;
  }

  void Publisher_Impl::cancel()
  {

#if (TCPUB_LOG_DEBUG_ENABLED)
    log_(logger::LogLevel::Debug, "Publisher " + localEndpointToString() + ": Shutting down");
#endif

    {
      asio::error_code ec;
      acceptor_.close(ec);
      acceptor_.cancel(ec);
    }

    is_running_ = false;

    std::vector<std::shared_ptr<PublisherSession>> publisher_sessions;
    {
      // Copy the list, so we can safely iterate over it without locking the mutex
      std::lock_guard<std::mutex> publisher_sessions_lock(publisher_sessions_mutex_);
      publisher_sessions = publisher_sessions_;
    }
    for (const auto& session : publisher_sessions)
    {
      session->cancel();
    }
  }

  void Publisher_Impl::acceptClient()
  {
#if (TCPUB_LOG_DEBUG_ENABLED)
    log_(logger::LogLevel::Debug, "Publisher " + localEndpointToString() + ": Waiting for new client...");
#endif

    std::function<void(const std::shared_ptr<PublisherSession>&)> publisher_session_closed_handler
            = [me = shared_from_this()](const std::shared_ptr<PublisherSession>& session) -> void
              {
                std::lock_guard<std::mutex> publisher_sessions_lock(me->publisher_sessions_mutex_);

                auto session_it = std::find(me->publisher_sessions_.begin(), me->publisher_sessions_.end(), session);
                if (session_it != me->publisher_sessions_.end())
                {
                  me->publisher_sessions_.erase(session_it);
            #if (TCPUB_LOG_DEBUG_ENABLED)
                  me->log_(logger::LogLevel::Debug, "Publisher " + me->localEndpointToString() + ": Successfully removed Session to subscriber " + session->remoteEndpointToString() + ". Current subscriber count: " + std::to_string(me->publisher_sessions_.size()) + ".");
            #endif
                }
                else
                {
                  me->log_(logger::LogLevel::Error,  "Publisher " + me->localEndpointToString() + ": Trying to delete a non-existing publisher session.");
                }
              };

    // Create a new session
    auto session = std::make_shared<PublisherSession>(executor_->executor_impl_->ioService(), publisher_session_closed_handler, log_);
    acceptor_.async_accept(session->getSocket()
                          , [session, me = shared_from_this()](asio::error_code ec)
                          {
                            if (ec)
                            {
                              me->log_(logger::LogLevel::Error, "Publisher " + me->localEndpointToString() + ": Error while waiting for subsriber: " + ec.message());
                              return;
                            }
                            else
                            {
                              me->log_(logger::LogLevel::Info, "Publisher " + me->localEndpointToString() + ": Subscriber " + session->remoteEndpointToString() + " has connected.");
                            }

                            session->start();

                            // Add the session to the session list
                            {
                              std::lock_guard<std::mutex> publisher_sessions_lock_(me->publisher_sessions_mutex_);
                              me->publisher_sessions_.push_back(session);
#if (TCPUB_LOG_DEBUG_ENABLED)
                              me->log_(logger::LogLevel::Debug, "Publisher " + me->localEndpointToString() + ": Current subscriber count: " + std::to_string(me->publisher_sessions_.size()));
#endif
                            }

                            // Accept the next session
                            me->acceptClient();
                          });
  }

  ////////////////////////////////////////////////
  // Send data
  ////////////////////////////////////////////////

  bool Publisher_Impl::send(const std::vector<std::pair<const char* const, const size_t>>& payloads)
  {
    if (!is_running_)
    {
      log_(logger::LogLevel::Error, "Publisher::send " + localEndpointToString() + ": Tried to send data to a non-running publisher.");
      return false;
    }

    // Don' send data if no subscriber is connected
    {
      std::lock_guard<std::mutex> publisher_sessions_lock(publisher_sessions_mutex_);
      if (publisher_sessions_.empty())
      {
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
        log_(logger::LogLevel::DebugVerbose, "Publisher::send " + localEndpointToString() + ": No connection to any subscriber. Skip sending data.");
#endif
        return true;
      }
    }

    // If a subsriber is connected, we need to initialize a buffer.
    std::shared_ptr<std::vector<char>> buffer = buffer_pool.allocate();

#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    std::stringstream buffer_pointer_ss;
    buffer_pointer_ss << "0x" << std::hex << buffer.get();
    const std::string buffer_pointer_string = buffer_pointer_ss.str();
#endif

    // Check the size of the buffer and resize it
    {
      // Size of header
      size_t header_size = sizeof(TcpHeader);

      // Size of user data, i.e. all payload(s)
      size_t entire_payload_size = 0;
      for (const auto& payload : payloads)
      {
        entire_payload_size += payload.second;
      }

      // Size of header and user data
      const size_t complete_size = header_size + entire_payload_size;

      if (buffer->capacity() < complete_size)
      {
        buffer->reserve(static_cast<size_t>(complete_size * 1.1)); // Reserve 10% more bytes for later!
      }
      buffer->resize(complete_size);

#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
      log_(logger::LogLevel::DebugVerbose, "Publisher::send " + localEndpointToString() + ": Filling buffer " + buffer_pointer_string + " with header and data.Entire buffer size is " + std::to_string(buffer->size()) + " bytes.");
#endif

      // Fill header and copy the given data to the buffer
      auto header = reinterpret_cast<tcpub::TcpHeader*>(&(*buffer)[0]);
      header->header_size     = htole16(sizeof(TcpHeader));
      header->type            = MessageContentType::RegularPayload;
      header->reserved        = 0;
      header->data_size       = htole64(entire_payload_size);

      // copy the data into the buffer right after the header
      size_t current_position = header_size;
      for (const auto& payload : payloads)
      {
        memcpy(&((*buffer)[current_position]), payload.first, payload.second);
        current_position += payload.second;
      }
    }

    // Lock the sessions mutex again and send out the prepared buffer. All publisher sessions will operate on the same buffer!
    {
      std::lock_guard<std::mutex> publisher_sessions_lock(publisher_sessions_mutex_);

#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
      log_(logger::LogLevel::DebugVerbose, "Publisher::send " + localEndpointToString() + ": Sending buffer " + buffer_pointer_string + " to " + std::to_string(publisher_sessions_.size()) + " subsribers.");
#endif

      for (const auto& publisher_session : publisher_sessions_)
      {
        publisher_session->sendDataBuffer(buffer);
      }
    }

    return true;
  }

  ////////////////////////////////////////////////
  // (Status-) getters
  ////////////////////////////////////////////////

  uint16_t Publisher_Impl::getPort() const
  {
    if (is_running_)
    {
      asio::error_code ec;
      auto local_endpoint = acceptor_.local_endpoint();
      if (!ec)
        return local_endpoint.port();
      else
        return 0;
    }
    else
    {
      return 0;
    }
  }

  size_t Publisher_Impl::getSubscriberCount() const
  {
    std::lock_guard<std::mutex> publisher_sessions_lock(publisher_sessions_mutex_);
    return publisher_sessions_.size();
  }

  std::string Publisher_Impl::toString(const asio::ip::tcp::endpoint& endpoint) const
  {
    return endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
  }

  std::string Publisher_Impl::localEndpointToString() const
  {
    asio::error_code ec;
    auto local_endpoint = acceptor_.local_endpoint(ec);
    if (!ec)
      return toString(local_endpoint);
    else
      return "?";
  }
}