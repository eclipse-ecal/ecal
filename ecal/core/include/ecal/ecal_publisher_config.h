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
 * @file   ecal_publisher_config.h
 * @brief  eCAL publisher configuration
**/

#pragma once

#include <ecal/ecal_tlayer.h>

namespace eCAL
{
  struct ECAL_API SHMPubConfig
  {
    TLayer::eSendMode  send_mode               = TLayer::smode_auto;  //!< shm layer send mode (default auto)
    bool               zero_copy_mode          = false;               //!< enable zero copy shared memory transport mode
    int                acknowledge_timeout_ms  = 0;                   /*!< force connected subscribers to send acknowledge event after processing the message
                                                                            the publisher send call is blocked on this event with this timeout (0 == no handshake) */
    size_t             memfile_min_size_bytes  = 4096;                //!< default memory file size for new publisher
    size_t             memfile_reserve_percent = 50;                  //!< dynamic file size reserve before recreating memory file if topic size changes
    size_t             memfile_buffer_count    = 1;                   //!< maximum number of used buffers (needs to be greater than 1, default = 1)
  };

  struct ECAL_API UDPPubConfig
  {
    TLayer::eSendMode  send_mode               = TLayer::smode_auto;  //!< udp layer send mode (default auto)
    int                sndbuf_size_bytes       = (5*1024*1024);       //!< udp send buffer size in bytes (default 5MB)
  };

  struct ECAL_API TCPPubConfig
  {
    TLayer::eSendMode  send_mode               = TLayer::smode_off;   //!<  tcp layer send mode (default off)
  };

  struct ECAL_API PubConfig
  {
    PubConfig();

    SHMPubConfig shm;
    UDPPubConfig udp;
    TCPPubConfig tcp;

    bool share_topic_type        = true;
    bool share_topic_description = true;
  };
}
