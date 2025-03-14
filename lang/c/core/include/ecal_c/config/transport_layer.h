/* =========================== LICENSE =================================
 *
 * Copyright (C) 2016 - 5 Continental Corporation
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

#ifndef ecal_c_config_transport_layer_h_included
#define ecal_c_config_transport_layer_h_included

#include <stddef.h>

#include <ecal_c/types/custom_data_types.h>

enum eCAL_TransportLayer_eType
{
  eCAL_TransportLayer_eType_none,
  eCAL_TransportLayer_eType_udp_mc,
  eCAL_TransportLayer_eType_shm,
  eCAL_TransportLayer_eType_tcp,
};

struct eCAL_TransportLayer_UDP_MulticastConfiguration
{
  char group[16]; //!< UDP multicast group base
  unsigned int ttl; //!< UDP ttl value, also known as hop limit, is used in determining the intermediate routers being traversed towards the destination
};

struct eCAL_TransportLayer_UDP_Configuration
{
  enum eCAL_Types_UdpConfigVersion config_version; //!< UDP configuration version (Since eCAL 5.12.)
  unsigned int port; //!< UDP multicast port number (Default: 14002)
  char mask[16]; //!< Mask maximum number of dynamic multicast group
  unsigned int send_buffer; //!< UDP send buffer in bytes (Default: 5242880)
  unsigned int receive_buffer; //!< UDP receive buffer in bytes (Default: 5242880)
  int join_all_interfaces; //!< Linux specific setting to enable joining multicast groups on all network interfaces
  int npcap_enabled; //!< Enable to receive UDP traffic with the Npcap based receiver (Default: false)
  struct eCAL_TransportLayer_UDP_MulticastConfiguration network; //!< default: "239.0.0.1", 3U
  struct eCAL_TransportLayer_UDP_MulticastConfiguration local; //!< default: "127.255.255.255", 1U
};

struct eCAL_TransportLayer_TCP_Configuration
{
  size_t number_executor_reader; //!< Reader amount of threads that shall execute workload (Default: 4)
  size_t number_executor_writer; //!< Writer amount of threads that shall execute workload (Default: 4)
  int max_reconnections; //!< Reconnection attempts the session will try to reconnect in (Default: 5)
};

struct eCAL_TransportLayer_Configuration
{
  struct eCAL_TransportLayer_UDP_Configuration udp;
  struct eCAL_TransportLayer_TCP_Configuration tcp;
};

#endif /* ecal_c_config_transport_layer_h_included */