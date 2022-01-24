// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#pragma once

#include <memory>
#include <string>
#include <mutex>
#include <atomic>

#include <asio.hpp>
#include <recycle/shared_pool.hpp>

#include <tcpub/executor.h>
#include "tcpub_logger_abstraction.h"
#include "publisher_session.h"

namespace tcpub
{
  class Publisher_Impl : public std::enable_shared_from_this<Publisher_Impl>
  {

  ////////////////////////////////////////////////
  // Constructor & Destructor
  ////////////////////////////////////////////////
  
  public:
    // Constructor
    Publisher_Impl(const std::shared_ptr<Executor>& executor);

    // Copy
    Publisher_Impl(const Publisher_Impl&)            = delete;
    Publisher_Impl& operator=(const Publisher_Impl&) = delete;

    // Move
    Publisher_Impl& operator=(Publisher_Impl&&)      = delete;
    Publisher_Impl(Publisher_Impl&&)                 = delete;

    // Destructor
    ~Publisher_Impl();

  ////////////////////////////////////////////////
  // Start & Stop
  ////////////////////////////////////////////////
  
  public:
    bool start(const std::string& address, uint16_t port);
    void cancel();

  private:
    void acceptClient();

  ////////////////////////////////////////////////
  // Send data
  ////////////////////////////////////////////////
  
  public:
    bool send(const std::vector<std::pair<const char* const, const size_t>>& payloads);

  ////////////////////////////////////////////////
  // (Status-) getters
  ////////////////////////////////////////////////

  public:
    uint16_t           getPort()            const;
    size_t             getSubscriberCount() const;

    bool               isRunning()          const;

  private:
    std::string toString(const asio::ip::tcp::endpoint& endpoint) const;
    std::string localEndpointToString() const;

  ////////////////////////////////////////////////
  // Member variables
  ////////////////////////////////////////////////
  
  private:
    std::atomic<bool> is_running_;                                              /// Indicates whether this publisher is running and can send data. May be false, if e.g. binding to the given address has failed.

    // Asio
    const std::shared_ptr<Executor>                executor_;                   /// Global Executor (holding the io_service and thread pool)
    asio::ip::tcp::acceptor                        acceptor_;                   /// Acceptor used for waiting for clients (i.e. subscribers)
                                                
    // Logger                                    
    const logger::logger_t                         log_;                        /// Function for logging
                                                   
    // Sessions                                       
    mutable std::mutex                             publisher_sessions_mutex_;   
    std::vector<std::shared_ptr<PublisherSession>> publisher_sessions_;         /// List of all sessions (i.e. connections to subsribers)

    // Buffer pool
    struct buffer_pool_lock_policy_
    {
      using mutex_type = std::mutex;
      using lock_type  = std::lock_guard<mutex_type>;
    };
    recycle::shared_pool<std::vector<char>, buffer_pool_lock_policy_> buffer_pool; /// Buffer pool that let's us reuse memory chunks
  };
}
