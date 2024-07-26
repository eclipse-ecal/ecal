/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

#include "ecal_udp_configurations.h"

#include "ecal_def.h"
#include "ecal_udp_topic2mcast.h"

#include <ecal/ecal_config.h>
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
    std::array<char, 4> GeteCALDatagramHeader()
    {
      return std::array<char, 4>{ 'E', 'C', 'A', 'L' };
    }

    /**
     * @brief IsBroadcast() retrieves if we communicate via UDP Broadcast or UDP Multicast.
     *
     * @return True if broadcast mode is active.
     */
    bool IsBroadcast()
    {
      return !Config::IsNetworkEnabled();
    }

    /**
     * @brief IsNpcapEnabled() retrieves if we use the npcap UDP receiver (windows only).
     *
     * @return True if npcap mode is active.
     */
    bool IsNpcapEnabled()
    {
      return Config::IsNpcapEnabled();
    }

    /**
     * @brief Linux specific setting to enable joining multicast groups on all network interfacs independent of their link state.
     *
     * Enabling this makes sure that eCAL processes receive data if they are started before network devices are up and running.
     *
     * @return True if this setting is active.
     */
    bool IsUdpMulticastJoinAllIfEnabled()
    {
      return Config::IsUdpMulticastJoinAllIfEnabled();
    }

    /**
     * @brief GetLocalBroadcastAddress retrieves the broadcast address within the loopback range.
     *
     * This function returns the specific broadcast address 127.255.255.255, which is within
     * the loopback range (127.0.0.0 to 127.255.255.255). It is commonly used for local
     * communication within the same machine. Keep in mind that broadcasting to this address
     * will only reach processes running on the same host.
     *
     * @return The loopback broadcast address "127.255.255.255" as a string.
     */
    std::string GetLocalBroadcastAddress()
    {
      return "127.255.255.255";
    }

    std::string GetRegistrationAddress()
    {
      // check if the network is disabled
      const bool local_only = !Config::IsNetworkEnabled();
      if (local_only)
      {
        return GetLocalBroadcastAddress();
      }

      // both in v1 and v2, the multicast group is returned as the adress for the registration layer
      return Config::GetUdpMulticastGroup();
    }

    int GetRegistrationPort()
    {
      // retrieve the configured UDP multicast port from the configuration
      const int configured_port = Config::GetUdpMulticastPort();

      // add the specific offset, NET_UDP_MULTICAST_PORT_REG_OFF, to obtain the registration port
      return configured_port + NET_UDP_MULTICAST_PORT_REG_OFF;
    }
      
    std::string GetLoggingAddress()
    {
      // registration, logging and payload use the same addresses but different ports
      return GetRegistrationAddress();
    }

    int GetLoggingPort()
    {
      // retrieve the configured UDP multicast port from the configuration
      const int configured_port = Config::GetUdpMulticastPort();

      // add the specific offset, NET_UDP_MULTICAST_PORT_LOG_OFF, to obtain the logging port
      return configured_port + NET_UDP_MULTICAST_PORT_LOG_OFF;
    }

    std::string GetPayloadAddress()
    {
      // registration, logging and payload use the same addresses but different ports
      return GetRegistrationAddress();
    }

    std::string GetTopicPayloadAddress(const std::string& topic_name)
    {
      // check if the network is disabled
      const bool local_only = !Config::IsNetworkEnabled();
      if (local_only)
      {
        // if network is disabled, return the local broadcast address
        return GetLocalBroadcastAddress();
      }

      // determine the UDP multicast configuration version
      if (Config::GetUdpMulticastConfigVersion() == Types::UdpConfigVersion::V1)
      {
        // retrieve the corresponding multicast address based on the topic name using v1 implementation
        return UDP::V1::topic2mcast(topic_name, Config::GetUdpMulticastGroup(), Config::GetUdpMulticastMask());
      }
      // v2
      else
      {
        // retrieve the corresponding multicast address based on the topic name using v2 implementation
        return  UDP::V2::topic2mcast(topic_name, Config::GetUdpMulticastGroup(), Config::GetUdpMulticastMask());
      }
    }

    int GetPayloadPort()
    {
      // retrieve the configured UDP multicast port from the configuration
      const int configured_port = Config::GetUdpMulticastPort();

      // add the specific offset, NET_UDP_MULTICAST_PORT_SAMPLE_OFF, to obtain the payload port
      return configured_port + NET_UDP_MULTICAST_PORT_SAMPLE_OFF;
    }

    int GetMulticastTtl()
    {
      // check if the network is disabled
      const bool local_only = !Config::IsNetworkEnabled();
      if (local_only)
      {
        // if network is disabled, return a TTL of 0 to restrict multicast packets to the local machine
        return 1;
      }

      // if network is enabled, return the configured UDP multicast TTL value
      return Config::GetUdpMulticastTtl();
    }

    int GetReceiveBufferSize()
    {
      return Config::GetUdpMulticastRcvBufSizeBytes();
    }

    int GetSendBufferSize()
    {
      return Config::GetUdpMulticastSndBufSizeBytes();
    }
  }
}
