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
     * When the port can be opened, the Publisher will immediatelly accept
     * connections from Subscribers. Whether the Publisher is running can be
     * checked with isRunning().
     *
     * @param[in] executor
     *              The (global) executor that shall execute the workload and be
     *              used for logging.
     * 
     * @param[in] address
     *              The IP address to bind to. When setting this to "0.0.0.0"
     *              connections from any IP are accepted.
     * 
     * @param[in] port
     *              The port to accept connections from. When setting to "0",
     *              the operating system will usually autoamtically chooose a
     *              free port.
     */
    Publisher(const std::shared_ptr<Executor>& executor, const std::string& address, uint16_t port);

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
     * When the port can be opened, the Publisher will immediatelly accept
     * connections from Subscribers. Whether the Publisher is running can be
     * checked with isRunning().
     *
     * @param[in] executor
     *              The (global) executor that shall execute the workload and be
     *              used for logging.
     * 
     * @param[in] port
     *              The port to accept connections from. When omitting this
     *              parameter, the operating system will usually autoamtically
     *              chooose a free port.
     */
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
    /**
     * @brief Get the port that this publisher is listening to for new connections
     * 
     * - If the publisher has failed to open the given port, 0 is returned
     * 
     * - If the publisher has been created with a specific port, this port is
     *   returned
     * 
     * - If the publsiher has been created with port=0, the port chosen from the
     *   operating system is returned
     * 
     * @return The port of this publisher
     */
    uint16_t           getPort()            const;

    /**
     * @brief Get the amount of subscriptions to this publisher
     * 
     * This method is thread-safe
     * 
     * @return The number of active subscribers
     */
    size_t             getSubscriberCount() const;

    /**
     * @brief Check whether the publisher is running
     * 
     * A publisher is running, iff:
     *   - The publishers port could be opened and the publisher could bind to
     *     the given IP address (if provided)
     *   - cancel() has not been called
     * 
     * This method is thread-safe.
     * 
     * @return Whether the publisher is running
     */
    bool isRunning()                        const;

    /**
     * @brief Send data to all subscribers
     * 
     * Sends one buffer to all subsribers (if possbile). The data will be copied
     * to an internal buffer.
     * 
     * If you need to send multiple buffers as one element
     * (e.g. header + payload), you should use the send(std::vector buffers)
     * function, instead of copying the data into continuous memory in your own
     * code.
     * 
     * See send(std::vector buffers) for more details.
     * 
     * This function is thread safe.
     * 
     * @param[in] data
     *              Pointer to the data to send
     * 
     * @param[in] size
     *              Size of the data to send in number-of-bytes
     * 
     * @return Whether sending has been successfull (i.e. the publisher is running)
     */
    bool send(const char* const data, size_t size) const;

    /**
     * @brief Send data to all subscribers
     * 
     * Sends the given data to all subsribers (if possible). This function
     * supports sending an indefinite amount of sub-buffers, that will be copied
     * into one internal binary buffer.
     * You may release your memory after this function has returned.
     * 
     * Tip:
     * 
     *   Use this to send your custom Header and Payload as 1 buffer, without
     *   having to copy everything into one continuous memory chunk yourself!
     *   This method will copy the memory anyways (and only if there are active
     *   subsriptions), so copying in your could would be a waste of resources.
     * 
     *   A call could look like this:
     * 
     *     send({{header.data(), header.size()}, {payload.data(), payload.size()}});
     * 
     * The subscriber will receive the data as 1 binary blob containing all
     * sub-buffers in the same order they have been provided to this function.
     * So you need to make sure that your subscriber can recover the information
     * about the length of each sub-buffer, if you require to divide the data,
     * again.
     * 
     * Note that calling this function does not guarantee that the data will
     * physically be sent to the subsribers. Internally, a 1-element-queue is used:
     * 
     *   - 1 Element is currently sent to the subscriber. As TCP works with 1->1
     *     connections, this element may be a different one for all connections,
     *     if e.g. the link speed is different and some connections send their
     *     data faster than others.
     * 
     *   - 1 Element is kept in queue as next element. It will be sent once the
     *     current element has been processed. If you call send too fast, this
     *     buffer will be dropped. tcpub will always assume that the last
     *     element is the only important one.
     * 
     * If there are no active subscriptions, this function returns immediatelly
     * without doing anything. You don't need to check with getSubsriberCount()
     * in that case.
     * 
     * This method is thread-safe.
     * 
     * @param[in] buffers
     *              List of (sub-)buffers to send to all subscribers
     * 
     * @return True if sending was successfull (i.e. the publisher is running)
     */
    bool send(const std::vector<std::pair<const char* const, const size_t>>& buffers) const;

    /**
     * @brief Close all connections
     * 
     * Closes all connections to all subscribers. New subscribers are not
     * accepted any more. Data cannot be sent any more.
     * 
     * It is *not* required to call this before destroying the publisher object.
     * It is done automatically in the destructor.
     */
    void cancel();

  private:
    std::shared_ptr<Publisher_Impl> publisher_impl_;
  };
}
