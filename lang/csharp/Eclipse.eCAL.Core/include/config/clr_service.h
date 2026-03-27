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

#include <ecal/config/service.h>

namespace Eclipse {
  namespace eCAL {
    namespace Core {
      namespace Config {

        /**
         * @brief Managed wrapper for the native ::eCAL::Service::Configuration structure.
         */
        public ref class ServiceConfiguration {
        public:
          /**
           * @brief Gets or sets the timeout in ms to wait for the client to send its entity ID after connecting.
           *        If the client does not send its entity ID within this time, the server will use a placeholder.
           *        Set to 0 to disable waiting (old behavior). Default: 1000
           */
          property unsigned int ServerClientIdTimeoutMs;

          /**
           * @brief Default constructor.
           */
          ServiceConfiguration() {
            ::eCAL::Service::Configuration native{};
            ServerClientIdTimeoutMs = native.server_client_id_timeout_ms;
          }

          /**
           * @brief Native struct constructor.
           * @param native_config Native ::eCAL::Service::Configuration structure.
           */
          ServiceConfiguration(const ::eCAL::Service::Configuration& native_config) {
            ServerClientIdTimeoutMs = native_config.server_client_id_timeout_ms;
          }

          /**
           * @brief Converts this managed object to the native structure.
           * @return Native ::eCAL::Service::Configuration structure.
           */
          ::eCAL::Service::Configuration ToNative() {
            ::eCAL::Service::Configuration native_config;
            native_config.server_client_id_timeout_ms = ServerClientIdTimeoutMs;
            return native_config;
          }
        };

      } // namespace Config
    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse
