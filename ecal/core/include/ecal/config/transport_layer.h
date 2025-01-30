/* =========================== LICENSE =================================
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
 * =========================== LICENSE =================================
 */

/**
 * @file   config/transport_layer.h
 * @brief  eCAL configuration for the transport layer
**/

#pragma once

#include <ecal/types/custom_data_types.h>
#include <ecal/os.h>

namespace eCAL
{
  namespace TransportLayer
  {
    enum class eType
    {
      none,
      udp_mc,
      shm,
      tcp,
    };

    namespace UDP
    {      
      struct MulticastConfiguration
      {
        Types::IpAddressV4 group{"239.0.0.1"}; //!< UDP multicast group base
        unsigned int       ttl;                /*!< UDP ttl value, also known as hop limit, is used in determining 
                                                    the intermediate routers being traversed towards the destination */
      };

      struct Configuration
      {
        Types::UdpConfigVersion config_version      { Types::UdpConfigVersion::V2 }; /*!< UDP configuration version (Since eCAL 5.12.)
                                                                                             v1: default behavior
                                                                                             v2: new behavior, comes with a bit more intuitive handling regarding masking of the groups (Default: v2) */
        unsigned int            port                { 14002 };                       /*!< UDP multicast port number (Default: 14002) */
        Types::UDPMode          mode                { Types::UDPMode::LOCAL };       /*!< Valid modes: local, network (Default: local)*/
        Types::IpAddressV4      mask                { "255.255.255.240" };           /*!< v1: Mask maximum number of dynamic multicast group (Default: 0.0.0.1-0.0.0.255)
                                                                                             v2: masks are now considered like routes masking (Default: 255.0.0.0-255.255.255.255)*/
                  
        unsigned int            send_buffer         { 5242880 }; //!< UDP send buffer in bytes (Default: 5242880)
        unsigned int            receive_buffer      { 5242880 }; //!< UDP receive buffer in bytes (Default: 5242880)
        bool                    join_all_interfaces { false };   /*!< Linux specific setting to enable joining multicast groups on all network interfacs
                                                                         independent of their link state. Enabling this makes sure that eCAL processes
                                                                         receive data if they are started before network devices are up and running. (Default: false)*/
        bool                    npcap_enabled       { false };   //!< Enable to receive UDP traffic with the Npcap based receiver (Default: false)
      
        MulticastConfiguration  network             { "239.0.0.1", 3U };      //!< default: "239.0.0.1", 3U
        MulticastConfiguration  local               { "127.255.255.255", 1U}; //!< default: "127.255.255.255", 1U
      }; 
    }

    namespace TCP
    {
      struct Configuration
      {
        size_t number_executor_reader { 4 }; //!< Reader amount of threads that shall execute workload (Default: 4)
        size_t number_executor_writer { 4 }; //!< Writer amount of threads that shall execute workload (Default: 4)
        int    max_reconnections      { 5 }; //!< Reconnection attemps the session will try to reconnect in (Default: 5)
      };
    }

    struct Configuration
    {
      UDP::Configuration udp;
      TCP::Configuration tcp;
    };
  }
}