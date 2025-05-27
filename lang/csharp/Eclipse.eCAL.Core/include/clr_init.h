/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#pragma once

#include <ecal/init.h>
#include <msclr/marshal_cppstd.h>
#include "config/clr_configuration.h"


namespace Eclipse {
  namespace eCAL {
    namespace Core {

      /**
       * @brief Managed wrapper for the eCAL initialization functionality.
       */
      public ref class Init {
      public:
        /**
         * @brief Managed enumeration for eCAL initialization flags.
         */
        [System::Flags]
        enum class Flags : unsigned int {
          None       = ::eCAL::Init::None,
          Publisher  = ::eCAL::Init::Publisher,
          Subscriber = ::eCAL::Init::Subscriber,
          Service    = ::eCAL::Init::Service,
          Monitoring = ::eCAL::Init::Monitoring,
          Logging    = ::eCAL::Init::Logging,
          TimeSync   = ::eCAL::Init::TimeSync,
          Default    = ::eCAL::Init::Default,
          All        = ::eCAL::Init::All
        };

        /**
         * @brief Gets the init eCAL configuration.
         * @return Managed wrapper for the init configuration.
         */
        static Config::Configuration^ InitConfiguration();
      };

    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse