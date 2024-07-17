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
 * @file   layer.h
 * @brief  eCAL configuration for common layer settings
**/

#pragma once

#include "ecal/types/ecal_custom_data_types.h"

namespace eCAL
{
  namespace Layer
  {
    namespace UDP
    {
      enum class MODE
      {
        CLOUD,
        LOCAL
      };

      struct General
      {
        Types::ConstrainedInteger<14000, 10>     port{};                        /*!< UDP multicast port number (eCAL will use at least the 2 following port
                                                                                    numbers too, so modify in steps of 10 (e.g. 1010, 1020 ...)(Default: 14000) */
        // TODO PG: are these minimum limits correct?
        Types::ConstrainedInteger<5242880, 1024> send_buffer{};                 //!< UDP send buffer in bytes (Default: 5242880)
        Types::ConstrainedInteger<5242880, 1024> receive_buffer{};              //!< UDP receive buffer in bytes (Default: 5242880)
        bool                                     join_all_interfaces{};         /*!< Linux specific setting to enable joining multicast groups on all network interfacs
                                                                                     independent of their link state. Enabling this makes sure that eCAL processes
                                                                                     receive data if they are started before network devices are up and running. (Default: false)*/
        bool                                     npcap_enabled{};               //!< Enable to receive UDP traffic with the Npcap based receiver (Default: false)
      };

      struct Local : General
      {

      };

      struct Network : General
      {
        Types::IpAddressV4                       group{};                       //!< UDP multicast group base (Default: 239.0.0.1)
        unsigned int                             ttl{};                         /*!< UDP ttl value, also known as hop limit, is used in determining 
                                                                                     the intermediate routers being traversed towards the destination(Default: 2) */
      };

      struct Configuration
      {
        Types::UdpConfigVersion                  config_version{};              /*!< UDP configuration version (Since eCAL 5.12.)
                                                                                     v1: default behavior
                                                                                     v2: new behavior, comes with a bit more intuitive handling regarding masking of the groups (Default: v1) */
        MODE                                     mode{};                        /*!< Valid modes: local, network (Default: local)*/                                                  
        
        General                                  general;
        Local                                    local;
        Network                                  network;
      }; 
    }

    namespace TCP
    {
      struct Configuration
      {
        size_t num_executor_reader{};                                           //!< reader amount of threads that shall execute workload (Default: 4)
        size_t num_executor_writer{};                                           //!< writer amount of threads that shall execute workload (Default: 4)
        size_t max_reconnections{};                                             //!< reconnection attemps the session will try to reconnect in (Default: 5)
      };
    }

    namespace SHM
    {
      struct Configuration
      {
        bool                                  zero_copy_mode;           //!< enable zero copy shared memory transport mode
        unsigned int                          acknowledge_timeout_ms;   /*!< force connected subscribers to send acknowledge event after processing the message
                                                                             the publisher send call is blocked on this event with this timeout (0 == no handshake) */
        Types::ConstrainedInteger<4096, 4096> memfile_min_size_bytes;   //!< default memory file size for new publisher
        Types::ConstrainedInteger<50, 1, 100> memfile_reserve_percent;  //!< dynamic file size reserve before recreating memory file if topic size changes
        Types::ConstrainedInteger<1, 1>       memfile_buffer_count;     //!< maximum number of used buffers (needs to be greater than 1, default = 1)
      };
    }
  }
}