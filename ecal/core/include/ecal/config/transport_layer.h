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
 * @file   transport_layer.h
 * @brief  eCAL configuration for the transport layer
**/

#pragma once

#include <ecal/types/ecal_custom_data_types.h>

namespace eCAL
{
  namespace TransportLayer
  {
    namespace UDP
    {      
      namespace Network
      {
        struct Configuration
        {
          Types::IpAddressV4 group { "239.0.0.1" }; //!< UDP multicast group base (Default: 239.0.0.1)
          unsigned int       ttl   { 3U };          /*!< UDP ttl value, also known as hop limit, is used in determining 
                                                         the intermediate routers being traversed towards the destination (Default: 3) */
        };
      }

      struct Configuration
      {
        Types::UdpConfigVersion config_version { Types::UdpConfigVersion::V2 }; /*!< UDP configuration version (Since eCAL 5.12.)
                                                                                     v1: default behavior
                                                                                     v2: new behavior, comes with a bit more intuitive handling regarding masking of the groups (Default: v2) */
        unsigned int            port           { 14002 };                       /*!< UDP multicast port number (Default: 14002) */
        Types::UDPMode          mode           { Types::UDPMode::LOCAL };       /*!< Valid modes: local, network (Default: local)*/
        Types::IpAddressV4      mask           { "255.255.255.240" };           /*!< v1: Mask maximum number of dynamic multicast group (Default: 0.0.0.1-0.0.0.255)
                                                                                     v2: masks are now considered like routes masking (Default: 255.0.0.0-255.255.255.255)*/
                    
        // TODO PG: are these minimum limits correct?
        Types::ConstrainedInteger<5242880, 1024> send_buffer         { 5242880 }; //!< UDP send buffer in bytes (Default: 5242880)
        Types::ConstrainedInteger<5242880, 1024> receive_buffer      { 5242880 }; //!< UDP receive buffer in bytes (Default: 5242880)
        bool                                     join_all_interfaces { false };   /*!< Linux specific setting to enable joining multicast groups on all network interfacs
                                                                                       independent of their link state. Enabling this makes sure that eCAL processes
                                                                                       receive data if they are started before network devices are up and running. (Default: false)*/
        bool                                     npcap_enabled       { false };   //!< Enable to receive UDP traffic with the Npcap based receiver (Default: false)
      
        Network::Configuration                   network;
      }; 
    }

    namespace TCP
    {
      struct Configuration
      {
        size_t number_executor_reader { 4 }; //!< Reader amount of threads that shall execute workload (Default: 4)
        size_t number_executor_writer { 4 }; //!< Writer amount of threads that shall execute workload (Default: 4)
        size_t max_reconnections      { 5 }; //!< Reconnection attemps the session will try to reconnect in (Default: 5)
      };
    }

    namespace SHM 
    {
      struct Configuration
      {
        Types::ConstrainedInteger<4096, 4096> memfile_min_size_bytes  { 4096 }; //!< Default memory file size for new publisher (Default: 4096)
        Types::ConstrainedInteger<50, 1, 100> memfile_reserve_percent { 50 };   //!< Dynamic file size reserve before recreating memory file if topic size changes (Default: 50)
      };
    }

    struct Configuration
    {
      UDP::Configuration udp;
      TCP::Configuration tcp;
      SHM::Configuration shm;
    };
  }
}