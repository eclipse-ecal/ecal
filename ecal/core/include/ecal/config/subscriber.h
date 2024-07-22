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
 * @file   ecal_subscriber_config.h
 * @brief  eCAL subscriber configuration
**/

#pragma once

#include <ecal/ecal_os.h>
#include <ecal/config/transport_layer.h>

#include <cstddef>

namespace eCAL
{
  namespace Subscriber
  {
    namespace SHM
    {
      struct Configuration
      {
        bool enable;                                                  //!< enable layer
      };
    }

    namespace UDP
    {
      struct Configuration
      {
        bool enable;                                                  //!< enable layer
      };
    }

    namespace TCP
    {
      struct Configuration
      {
        bool enable;                                                  //!< enable layer

        size_t num_executor_reader{};                                 //!< reader amount of threads that shall execute workload (Default: 4)
        size_t num_executor_writer{};                                 //!< writer amount of threads that shall execute workload (Default: 4)

        size_t max_reconnections{};                                   //!< reconnection attemps the session will try to reconnect in (Default: 5)
      };
    }

    struct Configuration
    {
      ECAL_API Configuration();

      SHM::Configuration shm;
      UDP::Configuration udp;
      TCP::Configuration tcp;

      bool drop_out_of_order_messages{}; 
    };
  }
}
