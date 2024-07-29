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

/**
 * @file   wrapper_datatypeinfo.h
 * @brief  Nanobind wrapper SDataTypeInformation struct
**/

#pragma once

#include <ecal/ecal.h>
#include <stdint.h>
#include <string>
#include <cstddef>
#include <ecal/ecal_os.h>
#include <ecal/types/ecal_custom_data_types.h>

#include <ecal/config/publisher.h>
#include <nanobind/nanobind.h>

namespace eCAL
{
    namespace Publisher
    {
        namespace SHM
        {
            struct CNBSHMConfiguration
            {
                bool                                  enable;                   //!< enable layer
                bool                                  zero_copy_mode;           //!< enable zero copy shared memory transport mode
                unsigned int                          acknowledge_timeout_ms;   /*!< force connected subscribers to send acknowledge event after processing the message
                                                                                     the publisher send call is blocked on this event with this timeout (0 == no handshake) */
                Types::ConstrainedInteger<4096, 4096> memfile_min_size_bytes;   //!< default memory file size for new publisher
                Types::ConstrainedInteger<50, 1, 100> memfile_reserve_percent;  //!< dynamic file size reserve before recreating memory file if topic size changes
                Types::ConstrainedInteger<1, 1>       memfile_buffer_count;     //!< maximum number of used buffers (needs to be greater than 1, default = 1)
            };
        }

        namespace UDP
        {
            struct CNBUDPConfiguration
            {
                bool                                      enable;               //!< enable layer
                bool                                      loopback;             //!< enable to receive udp messages on the same local machine
                Types::ConstrainedInteger<5242880, 1024>  sndbuf_size_bytes;    //!< udp send buffer size in bytes (default 5MB)
            };
        }

        namespace TCP
        {
            struct CNBTCPConfiguration
            {
                bool               enable;                                      //!< enable layer
            };
        }

        struct CNBPublisherConfiguration
        {
            CNBPublisherConfiguration();

            SHM::Configuration   shm;
            UDP::Configuration   udp;
            TCP::Configuration   tcp;

            bool                 share_topic_type;                            //!< share topic type via registration
            bool                 share_topic_description;                     //!< share topic description via registration
        };
    }
}
