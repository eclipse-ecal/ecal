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

  /**
   * @brief The Publisher side of tcpub
   *
   * Once created, a Publisher waits for connections on a port. You can either
   * choose a port to listen on or let that decision to the operating system.
   * See the constructor for more details.
   *
   * A Publisher uses a 1-element send-queue:
   *
   * - 1 message is being sent to a subscriber. As each Subscriber connects
   *   individually and with it's own link speed, this message will not be the
   *   same for each subscription.
   *
   * - One message is being kept as next-message. Once a Session has finished
   *   sending its message to its subscriber, it will take that message as next
   *   one. If messages are provided faster than the link speed can handle, only
   *   the last message is kept and other messages will be dropped.
   *
   */
  class Publisher
  {
  public:
    /**
     * @brief Creates a new publisher
     *
     * You can either choose a specific port, or leave it to the OS to choose a
     * free port for you:
     *
     *   - port = 0:  The operating system will choose a free port. You can
     *                retrieve that port by Publisher::getPort().
     *
     *   - port != 0: The given port will be used. When this port is already
     *                opened by another application or the OS denies opening the
     *                port, isRunning() will return false.
     *
     * When the given port can be opened, the Publisher will immediatelly accept
     * connections from Subscribers.
     *
     * @param executor
     * @param address
     * @param port
     */
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

    bool isRunning()                        const;

    bool send(const char* const data, size_t size) const;
    bool send(const std::vector<std::pair<const char* const, const size_t>>&) const;

    void cancel();

  private:
    std::shared_ptr<Publisher_Impl> publisher_impl_;
  };
}
