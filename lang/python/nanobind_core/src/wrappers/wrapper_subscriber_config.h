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
 * @file   wrapper_publisher_config.h
 * @brief  Nanobind wrapper for structs of Publisher config
**/

#pragma once

#include <ecal/ecal.h>
#include <stdint.h>
#include <string>
#include <cstddef>
#include <ecal/ecal_os.h>
#include <ecal/types/ecal_custom_data_types.h>

#include <ecal/config/subscriber.h>
#include <nanobind/nanobind.h>

namespace eCAL
{
    namespace Subscriber
    {
        namespace SHM
        {
            struct CNBSHMConfiguration
            {
                bool enable;                                                  //!< enable layer
            };
        }

        namespace UDP
        {
            struct CNBUDPConfiguration
            {
                bool enable;                                                  //!< enable layer
            };
        }

        namespace TCP
        {
            struct CNBTCPConfiguration
            {
                bool enable;                                                  //!< enable layer
            };
        }

        struct CNBSubscriberConfiguration
        {
            CNBSubscriberConfiguration();

            SHM::Configuration shm;
            UDP::Configuration udp;
            TCP::Configuration tcp;
        };
    }
}
