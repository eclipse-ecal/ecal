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
 * 
 * This publisher configuration struct can be used to define the behavior of an eCAL publisher. Additional information on 
 * selected configuration parameters:
 * 
 * ----------------------------------------------------------------------------------
 * Zero copy shared memory transport mode (SHM::Configuration::zero_copy_mode)
 * ----------------------------------------------------------------------------------
 *
 * By default, the built-in shared memory layer is configured to make two memory copies
 * one on the publisher and one on the subscriber side.
 * 
 * The intention of this implementation is to free the file as fast as possible after writing and reading
 * its content to allow other processes to access the content with minimal latency. The publisher and subscribers
 * are fully decoupled and can access their internal memory copy independently.
 * 
 * If the zero copy mode is switched on no memory will be copied at all using the low level binary publish / subscribe API.
 * On publisher side the memory copy is exectuted into the opened memory file. On the subscriber side the user message 
 * callback is called right after opening the memory file. A direct pointer to the memory payload is forwarded
 * and can be processed with no latency. The memory file will be closed after the user callback function
 * returned.
 *
 * The advantage of this configuration is a much higher performance for large payloads (> 1024 kB).
 * The disadvantage of this configuration is that in the time when the callback is executed the memory file 
 * is blocked for other subscribers and for writing publishers too. Maybe this can be eliminated
 * by a better memory file read/write access implementation (lock free read) in future releases.
 * 
 * Today, for specific scenarios (1:1 pub/sub connections with large payloads for example) this feature
 * can increase the performance remarkable. But please keep in mind to return from the message callback function
 * as fast as possible to not delay subsequent read/write access operations.
 * 
 * By using the eCAL::CPayloadWriter API a full zero copy implementation is possible by providing separate methods
 * for the initialization and the modification of the memory file content (see CPayloadWriter documentation).
 *
 * 
 * ----------------------------------------------------------------------------------
 * Acknowledgement timeout (SHM::Configuration::acknowledge_timeout_ms)
 * ----------------------------------------------------------------------------------
 *
 * Most applications perform very well with the default behavior. If subscribers are too slow
 * to process incoming messages then the overall software architecture needs to be checked, software components
 * need to be optimized or parallelized.
 *
 * There may still be cases where it could make sense to synchronize the transfer of the payload from a publisher
 * to a subscriber by using an additional handshake event. This event is signaled by a subscriber back to the
 * sending publisher to confirm the complete payload transmission and the processed subscriber callback.
 *
 * The publisher will wait up to the specified timeout for the acknowledge signals of all connected subscribers
 * before sending new content. Finally that means the publishers CPublisher::Send API function call is now blocked
 * and will not return until all subscriber have read and processed their content or the timeout has been reached.
 *
 *
 * ----------------------------------------------------------------------------------
 * Number of handled memory files (SHM::Configuration::memfile_buffer_count)
 * ----------------------------------------------------------------------------------
 *
 * By default, each publisher creates one memory file to distribute its payload to the subscribers. Since eCAL does not 
 * currently support a rw lock synchronisation mechanism for interprocess communication, reading subscribers are blocking
 * the memory file and thus are preventing a publisher from writing the next payload into the file.
 *
 * This blocking behavior can be mitigated by using multiple memory files per publisher/subscriber connection. These memory
 * files are then written in a kind of ring buffer.
 * 
 * The disadvantage of this setting (memfile_buffer_count > 1) is the higher consumption of resources (memory files, events..)
 *
**/

#pragma once

#include <ecal/ecal_tlayer.h>

#include <cstddef>

namespace eCAL
{
  namespace Publisher
  {
    namespace SHM
    {
      struct ECAL_API Configuration
      {
        TLayer::eSendMode  send_mode               = TLayer::smode_auto;  //!< shm layer send mode (default auto)
        bool               zero_copy_mode          = false;               //!< enable zero copy shared memory transport mode
        int                acknowledge_timeout_ms  = 0;                   /*!< force connected subscribers to send acknowledge event after processing the message
                                                                                 the publisher send call is blocked on this event with this timeout (0 == no handshake) */
        size_t             memfile_min_size_bytes  = 4096;                //!< default memory file size for new publisher
        size_t             memfile_reserve_percent = 50;                  //!< dynamic file size reserve before recreating memory file if topic size changes
        size_t             memfile_buffer_count    = 1;                   //!< maximum number of used buffers (needs to be greater than 1, default = 1)
      };
    }

    namespace UDP
    {
      struct ECAL_API Configuration
      {
        TLayer::eSendMode  send_mode               = TLayer::smode_auto;  //!< udp layer send mode (default auto)
        int                sndbuf_size_bytes       = (5*1024*1024);       //!< udp send buffer size in bytes (default 5MB)
      };
    }

    namespace TCP
    {
      struct ECAL_API Configuration
      {
        TLayer::eSendMode  send_mode               = TLayer::smode_off;   //!< tcp layer send mode (default off)
      };
    }

    struct ECAL_API Configuration
    {
      Configuration();

      SHM::Configuration shm;
      UDP::Configuration udp;
      TCP::Configuration tcp;

      bool share_topic_type                      = true;                //!< share topic type via registration
      bool share_topic_description               = true;                //!< share topic description via registration
    };
  }
}
