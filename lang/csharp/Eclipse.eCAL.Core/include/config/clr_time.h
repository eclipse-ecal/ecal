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

#include <string>
#include <ecal/config/time.h>

#include "clr_common.h"
namespace Eclipse {
  namespace eCAL {
    namespace Core {
      namespace Config {

        /**
         * @brief Managed wrapper for the native ::eCAL::Time::Configuration structure.
         */
        public ref class TimeConfiguration {
        public:
          /**
           * @brief Gets or sets the time synchronization module for real-time.
           */
          property System::String^ TimeSyncModuleRT;

          /**
           * @brief Gets or sets the time synchronization module for replay.
           */
          property System::String^ TimeSyncModuleReplay;

          /**
           * @brief Default constructor.
           */
          TimeConfiguration() {
            // Use the default values from the native structure
            ::eCAL::Time::Configuration native_config;
            TimeSyncModuleRT = Internal::StlStringToString(native_config.timesync_module_rt);
            TimeSyncModuleReplay = Internal::StlStringToString(native_config.timesync_module_replay);
          }

          /**
           * @brief Native struct constructor.
           */
          TimeConfiguration(const ::eCAL::Time::Configuration& native_config) {
            TimeSyncModuleRT = Internal::StlStringToString(native_config.timesync_module_rt);
            TimeSyncModuleReplay = Internal::StlStringToString(native_config.timesync_module_replay);
          }

          /**
           * @brief Converts this managed object to the native structure.
           * @return Native Time::Configuration structure.
           */
          ::eCAL::Time::Configuration ToNative() {
            ::eCAL::Time::Configuration native_config;
            native_config.timesync_module_rt = Internal::StringToStlString(TimeSyncModuleRT);
            native_config.timesync_module_replay = Internal::StringToStlString(TimeSyncModuleReplay);
            return native_config;
          }
        };

      } // namespace Config
    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse