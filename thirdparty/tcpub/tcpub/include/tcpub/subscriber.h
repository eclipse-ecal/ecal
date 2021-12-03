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

  /**
   * @brief The Subscriber side of tcpub
   * 
   * A subscriber can connect to one or multiple publishers and receive data
   * from them. As TCP actually works with 1:1 connections, you need to create a
   * SubscriberSession for each connection to a publisher.
   * 
   * Once received, you will get the data via a callback. So after creating a
   * Subsriber, you should also call Subscriber::setCallback(). By default, Callbacks are asynchronous (and is is highly recommended that you keep it this way, unless you really know what you are doing!). Subscribers use a 1-element queue:
   * 
   * - 1 message is currently being processed by the callback you set. This
   *   callback runs in it's own thread, so you are allowed to do time-consuming
   *   computations in a callback.
   * 
   * - 1 message is kept as next-message. Once your callback has finished, it
   *   will automatically grab the next message, if available. If messages
   *   arrive too fast and the callback cannot process them, messages are being
   *   dropped, so your callback will always receive the latest data available.
   * 
   * All SubscriberSessions will share the same 1-message queue. It is assumend,
   * that the SubscriberSessions that are created in the same Subscriber are of
   * same "type" (whatever that means for you).
   */
  class Subscriber
  {
  public:
    /**
     * @brief creates a new Subscriber
     * 
     * A new subscriber will not do anything by default. In order to actually
     * receive data with it, you will have to add SubscriberSessions. Each
     * SubscriberSession is a connection to a publisher.
     * All Sessions in a Subscriber are assumed to receive data of similar
     * "type". All Sessions will share a single callback function and callback
     * thread.
     * 
     * @param[in] executor
     *              The (global) executor that shall execute the workload and be
     *              used for logging.
     */
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
    /**
     * @brief Add a new connection to a publisher
     * 
     * Adds a new connection to a publisher. As a publisher is identified by its
     * address and port, those parameters are mandatory. The address can both be
     * an IP Address or a URL / Hostname / etc. (something the Operating System
     * can resolve). 
     * 
     * By default, a Session will try to reconnect, when anything failes. How
     * often this shall happen until the Session will delete itself from the
     * Subscriber can be controlled by the max_reconnection_attempts parameter.
     * A negative value will cause the Session to retry indefinitively, so you
     * will probably have to write your own algorithm code that cancels Sessions
     * that will not recover any more.
     * Between reconnection attemps the Session will wait 1 second.
     * 
     * Even though it may usually not make sense, you can add multiple
     * sessions to a single publisher. 
     * 
     * This function is thread-safe.
     * 
     * @param[in] address
     *              IP or Hostname of the publisher
     * 
     * @param[in] port
     *              Port the publisher is listening on
     * 
     * @param[in] max_reconnection_attempts
     *              How often the Session will try to reconnect in case of an
     *              issue. A negative value means infinite reconnection attemps.
     * 
     * @return A shared pointer to the session. You don't need to store it.
     */
    std::shared_ptr<SubscriberSession>              addSession(const std::string& address, uint16_t port, int max_reconnection_attempts = -1);

    /**
     * @brief Get a list of all Sessions.
     * 
     * Get a list of all Sessions. 
     * 
     * This function is thread-safe.
     * 
     * @return a list of all sessions.
     */
    std::vector<std::shared_ptr<SubscriberSession>> getSessions() const;

    /**
     * @brief Set a receive-data-callback
     * 
     * Set a callback that is called when new data has been received. By
     * default, a thread is created that will execute your callback
     * (=> synchronous_execution = false). You usually do not want to change
     * that. You may do heavy computation in this callback. However, while your
     * callback is running, there only exists a 1-message-queue. This makes sure
     * that your callback always operates on the latest data available, but if
     * your callback needs too long to process, data may be dropped.
     * 
     * It is possible to set a callback from within a running callback.
     * 
     * Synchronous callbacks (DANGEROUS):
     * - Synchronous means synchronous in a session-manner. If you are having
     *   multiple sessions, the callbacks will run in parallel.
     * - While a synchronous callback is running, no new data can be read from
     *   the socket of the corresponding SubscriberSession. This may cause data
     *   to stack up in the Sockets buffer, if your callback consumes too much
     *   time.
     * - A synchronous callback is exeucted by the Executor's thread-pool. If
     *   your callback needs too long to process, you can cause all Publishers
     *   and Subscriber sessions to block, because the thread pool is busy
     *   processing callbacks and cannot work on the TCP sockets any more
     * - Internally in tcpub, a synchronous callback is used to feed the
     *   asynchronous callback with new data. This is what it is meant for. So
     *   if you only want to pass the received shared_ptr<char> to your own
     *   queue or worker thread, you may actually want to use a synchronous
     *   callback.
     * 
     * This function is thread-safe
     * 
     * @param callback_function
     * @param synchronous_execution
     */
    void setCallback  (const std::function<void(const CallbackData& callback_data)>& callback_function, bool synchronous_execution = false);

    /**
     * @brief Clears the callback and removes all references kept internally.
     */
    void clearCallback();

    /**
     * @brief Shuts down the Subscriber and all Sessions
     * 
     * After cancelling, no new Data will be received. Callbacks will not be fed
     * data any more.
     * 
     * Cancelling will wait until the running callback has finished processing,
     * before returning.
     * 
     * Cancelling from a callback is supported, but in that case cancel()
     * obviously will return before the callback has finished.
     * 
     */
    void cancel();

  private:
    std::shared_ptr<Subscriber_Impl> subscriber_impl_;
  };
}
