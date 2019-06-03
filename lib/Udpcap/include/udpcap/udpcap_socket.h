/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
 */

#pragma once

#include <udpcap/host_address.h>

#include <vector>
#include <memory>

/*
Differences to Winsocks:
- SetReceiveBufferSize() must be called before bind() and cannot be changed after binding. On a Winsocks Socket setting the receive buffer before binding would fail.
- Sockets are always opened shared and addresses are reused. There is no way to open a non-shared socket.
- When binding to a loopback address (e.g. 127.0.0.1), setting MulticastLoopbackEnabled=true and joining a multicast group, this implementation will receive loopback multicast traffic. Winsocks would not do that (It's not clear to me why).
*/
namespace Udpcap
{
  class UdpcapSocketPrivate;


  /**
   * @brief The UdpcapSocket is a (receive-only) UDP Socket implementation using Npcap.
   *
   * Supported features are:
   *    - Binding to an IPv4 address and a port
   *    - Setting the receive buffer size
   *    - Joining and leaving multicast groups
   *    - Enabling and disabling multicast loopback (I managed to fix the cold start issue)
   *    - Receiving unicast and multicast packages (Only one memcpy from kernel to user space memory)
   *
   * Non supported features:
   *    - Sending data
   *    - Setting bind flags (sockets are always opened shared)
   *    - IPv6
   *
   * Differneces to a normal Winsocks bases socket:
   *    - SetReceiveBufferSize() must be called before bind() and cannot be
   *      changed after binding. On a Winsocks Socket setting the receive buffer
   *      before binding would fail.
   *    - Sockets are always opened shared and addresses are reused. There is no
   *      way to open a non-shared socket.
   *    - When binding to a loopback address (e.g. 127.0.0.1), setting
   *      MulticastLoopbackEnabled=true and joining a multicast group, this
   *      implementation will receive loopback multicast traffic. Winsocks would
   *      not do that (It's not clear to me why).
   */
  class UdpcapSocket
  {
  public:
    /**
     * @brief Creates a new UDP Socket
     *
     * Npcap is automatically initialized. If Npcap cannot be initialized, the
     * socket will be invalid (see isValid()).
     * The Socket is not bound and mutlicast loopback is enabled.
     */
    UdpcapSocket();
    ~UdpcapSocket();

    UdpcapSocket(UdpcapSocket const&) = delete;
    UdpcapSocket& operator= (UdpcapSocket const&) = delete;

    /**
     * @brief Checks whether the socket is valid (i.e. npcap has been intialized successfully)
     */
    bool isValid() const;

    /**
     * @brief Binds the socket to an address and a port
     *
     * When bound successfully, the Socket is ready to receive data. If the
     * address is HostAddress::Any(), any traffic for the given port will be
     * received.
     *
     * @param local_address The address to bind to
     * @param local_port    The port to bind to
     *
     * @return Whether binding has been successfull
     */
    bool bind(const HostAddress& local_address, uint16_t local_port);

    /**
     * @brief Returns whether the socket is in bound state
     */
    bool isBound() const;

    /**
     * @brief Returns the local address used for bind(), or HostAddress::Invalid(), if the socket is not bound
     */
    HostAddress localAddress() const;

    /**
     * @brief Returns the local port used for bind(), or 0 if the socket is not bound
     */
    uint16_t localPort() const;

    /**
     * @brief Sets the receive buffer size (non-pagable memory) in bytes
     *
     * The buffer size has to be set before binding the socket.
     *
     * @param receive_buffer_size The new receive buffer size
     * @return true if successfull
     */
    bool setReceiveBufferSize(int receive_buffer_size);

    /**
     * @brief Returns whether there are datagrams ready to be read
     */
    bool hasPendingDatagrams() const;

    /**
     * @brief Blocks until A packet arives and returns it as char-vector
     */
    std::vector<char> receiveDatagram(HostAddress* source_address = nullptr, uint16_t* source_port = nullptr);

    /**
     * @brief Blocks for the given time until a packet arives and returns it as char-vector
     *
     * If the socket is not bound, this method will return immediatelly.
     * If a source_adress or source_port is provided, these will be filled with
     * the according information from the packet. If the given time elapses
     * before a datagram was available, an empty vector is returned.
     *
     * @param timeout_ms     [in]:  Maximum time to wait for a datagram in ms
     * @param source_address [out]: the sender address of the datagram
     * @param source_port    [out]: the sender port of the datagram
     *
     * @return The datagram binary data
     */
    std::vector<char> receiveDatagram(unsigned long timeout_ms, HostAddress* source_address = nullptr, uint16_t* source_port = nullptr);

    size_t receiveDatagram(char* data, size_t max_len, HostAddress* source_address = nullptr, uint16_t* source_port = nullptr);

    /**
     * @brief Blocks for the given time until a packet arives and copies it to the given memory
     *
     * If the socket is not bound, this method will return immediatelly.
     * If a source_adress or source_port is provided, these will be filled with
     * the according information from the packet. If the given time elapses
     * before a datagram was available, no data is copied and 0 is returned.
     *
     * @param data           [out]: The destination memory
     * @param max_len        [in]:  The maximum bytes available at the destination
     * @param timeout_ms     [in]:  Maximum time to wait for a datagram in ms
     * @param source_address [out]: the sender address of the datagram
     * @param source_port    [out]: the sender port of the datagram
     *
     * @return The number of bytes copied to the data pointer
     */
    size_t receiveDatagram(char* data, size_t max_len, unsigned long timeout_ms, HostAddress* source_address = nullptr, uint16_t* source_port = nullptr);

    /**
     * @brief Joins the given multicast group
     *
     * When successfull, the socket will then start receiving data from that
     * multicast group.
     *
     * Joining a multicast group fails, when the Socket is invalid, not bound,
     * the given address is not a multicast address or this Socket has already
     * joined the group.
     *
     * @param group_address: The multicast group to join
     *
     * @return True if successfull
     */
    bool joinMulticastGroup(const HostAddress& group_address);

    /**
     * @brief Leave the given multicast group
     *
     * Leaving a multicast group fails, when the Socket is invalid, not bound,
     * the given address is not a multicast address or this Socket has not
     * joined the group, yet.
     *
     * @param group_address: The multicast group to leave
     *
     * @return True if sucessfull
     */
    bool leaveMulticastGroup(const HostAddress& group_address);

    /**
     * @brief Sets whether local multicast traffic should be received
     *
     * If not set, the default value is true.
     *
     * @param enables whether local multicast traffic should be received
     */
    void setMulticastLoopbackEnabled(bool enabled);

    /**
     * @return Whether local multicast receiving is enabled
     */
    bool isMulticastLoopbackEnabled() const;

    /**
     * @brief Closes the socket
     */
    void close();

  private:
    /** This is where the actual implementation lies. But the implementation has
     * to include many nasty header files (e.g. Windows.h), which is why we only
     * forward declared the class.
     */
    std::unique_ptr<Udpcap::UdpcapSocketPrivate> udpcap_socket_private_;
  };
}
