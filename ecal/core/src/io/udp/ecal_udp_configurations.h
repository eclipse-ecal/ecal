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

/**
 * @brief  common configurations for eCAL UDP communication
**/

#pragma once

#include <array>
#include <string>

namespace eCAL
{
  namespace UDP
  {
    /**
     * @brief Create eCAL specific UDP datagram header.
     *
     * @return The four magic letters :).
     */
    std::array<char, 4> GeteCALDatagramHeader();

    /**
     * @brief IsBroadcast() retrieves if we communicate via UDP Broadcast or UDP Multicast.
     *
     * @return True if broadcast mode is active.
     */
    bool IsBroadcast();
    
    /**
     * @brief IsNpcapEnabled() retrieves if we use the npcap UDP receiver (windows only).
     *
     * @return True if npcap mode is active.
     */
    bool IsNpcapEnabled();

    /**
     * @brief Linux specific setting to enable joining multicast groups on all network interfacs independent of their link state.
     *
     * Enabling this makes sure that eCAL processes receive data if they are started before network devices are up and running.
     *
     * @return True if this setting is active.
     */
    bool IsUdpMulticastJoinAllIfEnabled();

    /**
     * @brief GetRegistrationAddress retrieves the UDP registration address based on network configuration.
     *
     * If the network mode is disabled, it returns the local broadcast address.
     * Otherwise, it retrieves the UDP multicast group address from the global configuration.
     *
     * @return The UDP registration address based on the network configuration.
     */
    std::string GetRegistrationAddress();
    
    /**
     * @brief GetRegistrationPort retrieves the registration port based on the configured UDP multicast port.
     *
     * This function adds an offset, NET_UDP_MULTICAST_PORT_SAMPLE_OFF, to the UDP multicast port
     * obtained from the configuration. The resulting port is used for registration communication.
     *
     * @return The registration port calculated by adding an offset to the configured UDP multicast port.
     */    
    int GetRegistrationPort();

    /**
     * @brief GetLoggingAddress retrieves the UDP logging address based on network configuration.
     *
     * If the network mode is disabled, it returns the local broadcast address.
     * Otherwise, it retrieves the UDP multicast group address from the global configuration.
     *
     * @return The UDP logging address based on the network configuration.
     */
    std::string GetLoggingAddress();


    /**
     * @brief GetLoggingPort retrieves the logging port based on the configured UDP multicast port.
     *
     * This function adds an offset, NET_UDP_MULTICAST_PORT_SAMPLE_OFF, to the UDP multicast port
     * obtained from the configuration. The resulting port is used for logging communication.
     *
     * @return The logging port calculated by adding an offset to the configured UDP multicast port.
     */
    int GetLoggingPort();

    /**
     * @brief GetPayloadAddress retrieves the UDP payload address used as base address for udp receivers.
     *
     * If the network mode is disabled, it returns the local broadcast address.
     * Otherwise, it retrieves the UDP multicast group address from the global configuration.
     *
     * @return The UDP payload address based on the network configuration.
     */
    std::string GetPayloadAddress();

    /**
     * @brief GetTopicPayloadAddress retrieves the UDP payload address based on network configuration and the topic name.
     *
     * If the network mode is disabled, it returns the local broadcast address.
     *
     * If the topic name is empty, it returns base UDP multicast base group based on the global configuration.
     *
     * If a topic name is provided, it returns the payload address based on the topic name and
-    * UDP multicast configuration.
     *
     * @param  topic_name  The name of the topic for which the payload address is requested.
     *
     * @return The payload address based on the network configuration and the topic name.
     */
    std::string GetTopicPayloadAddress(const std::string& topic_name);

    /**
     * @brief GetPayloadPort retrieves the payload port based on the configured UDP multicast port.
     *
     * This function adds an offset, NET_UDP_MULTICAST_PORT_SAMPLE_OFF, to the UDP multicast port
     * obtained from the configuration. The resulting port is used for payload communication.
     *
     * @return The payload port calculated by adding an offset to the configured UDP multicast port.
     */    
    int GetPayloadPort();
 
    /**
     * @brief GetMulticastTtl retrieves the Time-to-Live (TTL) value for UDP multicast communication.
     *
     * If the network is disabled, it returns a TTL value of 0, indicating that multicast
     * packets should not be forwarded beyond the local machine.
     * If the network is enabled, it retrieves the configured UDP multicast TTL value from global configuration.
     *
     * @return The TTL value for UDP multicast communication based on the network configuration.
     */    
    int GetMulticastTtl();

    /**
     * @brief GetReceiveBufferSize retrieves the receive buffer size for UDP multicast communication.
     * 
     * @return The receive buffer size in bytes.
     */
    int GetReceiveBufferSize();

    /**
     * @brief GetSendBufferSize retrieves the send buffer size for UDP multicast communication.
     * 
     * @return The send buffer size in bytes.
     */
    int GetSendBufferSize();

  }
}
