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

#include <string>

namespace Udpcap
{
  /**
   * @brief Representation of a Host address
   *
   * Currently, only IPv4 addresses are supported. A HostAddress can be
   * constructed from a string (like "127.0.0.1") or a corresponding 32bit
   * integer.
   * Providing a faulty IPv4 string will result in an invalid host address
   * (check with isValid()). Using the default constructor will also result in
   * an invalid host address.
   *
   * There are several predefined Host addresses:
   *   HostAddress::Invalid()
   *   HostAddress::Any()
   *   HostAddress::LocalHost()
   *   HostAddress::Broadcast()
   *
   */
  class HostAddress
  {
  ////////////////////////////////
  // Host address
  ////////////////////////////////
  public:
    /** @brief Constructs an Invalid Host address */
    HostAddress();

    /**
     * @brief Constructs a Host address from a IPv4 string.
     * If the given tring is not parsable, the HostAddress will be inavlid.
     */
    HostAddress(const std::string& address);

    /** @brief Constructs a HostAddress from a 32bit integer in host byte order. */
    HostAddress(const uint32_t address);

    ~HostAddress();

    /** @brief Checks if the Host Address is valid.
     * Invalid HostAddresses are created when providing a wrong IPv4 string,
     * using the empty default constructor or the HostAddress::Invalid()
     * predefined address.
     */
    bool isValid() const;

    /**
     * @brief Checks if the HostAddress is a loopback address
     *
     * The IPv4 Loopback address range is from 127.0.0.0 to 127.255.255.255.
     * Invalid addresses are not considered to be loopback.
     *
     * You should always use this function to check for loopback addresses. Do
     * not only compare it to HostAddress::LocalHost() (=> 127.0.0.1), as the
     * loopback address range consists of many more addresses than the localhost
     * address.
     *
     * @return true, if the address is a loopback address
     */
    bool isLoopback() const;

    /**
     * @brief Checks if the HostAddress is a multicast address
     *
     * The IPv4 multicast address range is from 224.0.0.0 to 239.255.255.255.
     * Invalid addresses are not considered to be multicast.
     *
     * @return true, if the address is a multicast address
     */
    bool isMulticast() const;

    /**
     * @brief Creates a string representation of the HostAddress
     *
     * If invalid, an empty string is returned.
     * For HostAddress::Any() "0.0.0.0" will be returned
     *
     * @return A string representation of the HostAddress
     */
    std::string toString() const;

    /**
     * @brief Creates a 32bit Host-order integer representation of the HostAddress
     * If invalid or Any(), 0 will be returned.
     * @return The host-order integer representation of the HostAddress
     */
    uint32_t    toInt()    const;

  private:
    bool     valid_;  /**< Whether this address is valdid */
    uint32_t ipv4_;   /**< The host-order integer representation of this address */

  ////////////////////////////////
  // Compare operators
  ////////////////////////////////
  public:
    bool operator==(const HostAddress& other) const;
    bool operator!=(const HostAddress& other) const;
    bool operator< (const HostAddress& other) const;
    bool operator> (const HostAddress& other) const;
    bool operator<=(const HostAddress& other) const;
    bool operator>=(const HostAddress& other) const;

  ////////////////////////////////
  // Special addresses
  ////////////////////////////////
  public:
    /**
     * @brief Constructs an invald HostAddress (same as default constructor)
     * @return An invalid HostAddress
     */
    static HostAddress Invalid();

    /**
     * @brief Constructs a HostAddress representing any address (0.0.0.0)
     * @return A HostAddress representing any address
     */
    static HostAddress Any();

    /**
     * @brief Constructs a localhost HostAddress (127.0.0.1)
     * @return A localhost HostAddress
     */
    static HostAddress LocalHost();

    /**
     * @brief Constructs a broadcast HostAddress (255.255.255.255)
     * @return A broadcast HostAddress
     */
    static HostAddress Broadcast();
  };
}

