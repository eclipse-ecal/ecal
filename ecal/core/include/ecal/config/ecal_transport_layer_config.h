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
 * @file   ecal_transport_layer_config.h
 * @brief  eCAL configuration for the transport layer
**/

// TODO PG: Deprecated?
#pragma once

#include <ecal/types/ecal_custom_data_types.h>

namespace eCAL
{
  namespace TransportLayer
  {
    struct TCPubsubOptions
    {
      size_t num_executor_reader{};                                           //!<
      size_t num_executor_writer{};                                           //!<
      size_t max_reconnections{};                                             //!<
    };

    struct SHMOptions
    {
      std::string                            host_group_name{};               //!<
      Types::ConstrainedInteger<4096, 4096>  memfile_minsize{};               //!<
      Types::ConstrainedInteger<50, 1, 100>  memfile_reserve{};               //!<
      int                                    memfile_ack_timeout{};           //!<
      Types::ConstrainedInteger<0, 1>        memfile_buffer_count{};          //!<
      bool                                   drop_out_of_order_messages{};    //!<
      bool                                   memfile_zero_copy{};             //!<
    };

    struct UdpMulticastOptions
    {
      Types::UdpConfigVersion                  config_version{};              //!<
      Types::IpAddressV4                       group{};                       //!<
      Types::IpAddressV4                       mask{};                        //!<
      Types::ConstrainedInteger<14000, 10>     port{};                        //!<
      unsigned int                             ttl{};                       
      // TODO PG: are these minimum limits correct?
      Types::ConstrainedInteger<5242880, 1024> sndbuf{};                      //!<
      Types::ConstrainedInteger<5242880, 1024> recbuf{};                      //!<
      bool                                     join_all_interfaces{};         //!<

      int  bandwidth_max_udp{};                                               //!<
      bool npcap_enabled{};                                                   //!<
    }; 
      
    struct Configuration
    {
      bool                network_enabled{};                                  //!<
      bool                drop_out_of_order_messages{};                       //!<
      UdpMulticastOptions mc_options{};
      TCPubsubOptions     tcp_options{};
      SHMOptions          shm_options{};
    };
  }
}