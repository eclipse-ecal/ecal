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
 * @file   ecal_internal_options.h
 * @brief  eCAL options for internal configuration
**/

namespace eCAL
{
  constexpr int LAYER_NONE       = 0x0000;
  constexpr int LAYER_UDP_UDP_MC = 0x0010;
  constexpr int LAYER_UDP_TCP    = 0x0011;
  constexpr int LAYER_SHM        = 0x0020;
  constexpr int LAYER_INMEMORY   = 0x0030;

  enum class NetworkTransportLayer
  {
    tlayer_none   = LAYER_NONE,
    tlayer_udp_mc = LAYER_UDP_UDP_MC,
    tlayer_tcp    = LAYER_UDP_TCP
  };

  enum class LocalTransportLayer
  {
    tlayer_none   = LAYER_NONE,
    tlayer_udp_mc = LAYER_UDP_UDP_MC,
    tlayer_tcp    = LAYER_UDP_TCP,
    tlayer_shm    = LAYER_SHM
  };

  enum class InprocTransportLayer
  {
    tlayer_none     = LAYER_NONE,
    tlayer_udp_mc   = LAYER_UDP_UDP_MC,
    tlayer_tcp      = LAYER_UDP_TCP,
    tlayer_shm      = LAYER_SHM,
    tlayer_inmemory = LAYER_INMEMORY
  };

  struct eCAL_UDP_MC_Publisher_Options
  {
    
  };

  struct eCAL_TCP_Publisher_Options
  {
    // should we go as far as to put the  TCP address and port here? - NO only user options
        // both pub/sub need to know
  };

  struct eCAL_SHM_Publisher_Options
  {
    bool      enable_zero_copy = false;

    long      buffer_count = 1;                      // 1 .. x
    long long acknowledge_timeout_ms = -1; // -1 -> no timeout (or directly std::chrono?

    // should we go as far as to put the memory filename (base) here? - No
    // however, part of it will be communicated via the registration layer, invisible to the user
  };

  struct PublisherOptions 
  {
    eCAL_UDP_MC_Publisher_Options udp_mc_options;
    eCAL_TCP_Publisher_Options    tcp_options;
    eCAL_SHM_Publisher_Options    shm_options;

    InprocTransportLayer  inproc_layer;
    LocalTransportLayer   local_layer;
    NetworkTransportLayer network_layer;

    bool share_topic_information = true;
  };

  struct SubscriberOptions
  {
    InprocTransportLayer  inproc_layer;
    LocalTransportLayer   local_layer;
    NetworkTransportLayer network_layer;

    bool share_topic_information = true;
  };

  // TODO PG: Clarify how to handle these
  struct InternalConfig
  {
    SubscriberOptions subscriber_options;
    PublisherOptions  publisher_options;
  };
}