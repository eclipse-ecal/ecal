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

#include "clr_common.h"

#include <string>
#include <ecal/config/application.h>

namespace Eclipse {
  namespace eCAL {
    namespace Core {
      namespace Config {

        /**
         * @brief Managed wrapper for the native ::eCAL::Application::Sys::Configuration structure.
         */
        public ref class SysConfiguration {
        public:
          /**
           * @brief Gets or sets the filter exclusion regex.
           */
          property System::String^ FilterExcl;

          /**
           * @brief Default constructor.
           */
          SysConfiguration() {
            // Use the default values from the native structure
            ::eCAL::Application::Sys::Configuration native_config;
            FilterExcl = Internal::StlStringToString(native_config.filter_excl);
          }

          /**
           * @brief Parameterized constructor.
           * @param native_config Native Sys::Configuration structure.
           */
          SysConfiguration(const ::eCAL::Application::Sys::Configuration& native_config) {
            FilterExcl = Internal::StlStringToString(native_config.filter_excl);
          }

          /**
           * @brief Converts this managed object to the native structure.
           * @return Native Sys::Configuration structure.
           */
          ::eCAL::Application::Sys::Configuration ToNative() {
            ::eCAL::Application::Sys::Configuration native_config;
            native_config.filter_excl = Internal::StringToStlString(FilterExcl);
            return native_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::Application::Startup::Configuration structure.
         */
        public ref class StartupConfiguration {
        public:
          /**
           * @brief Gets or sets the terminal emulator command.
           */
          property System::String^ TerminalEmulator;

          /**
           * @brief Default constructor.
           */
          StartupConfiguration() {
            // Use the default values from the native structure
            ::eCAL::Application::Startup::Configuration native_config;
            TerminalEmulator = Internal::StlStringToString(native_config.terminal_emulator);
          }

          /**
           * @brief Parameterized constructor.
           * @param native_config Native Startup::Configuration structure.
           */
          StartupConfiguration(const ::eCAL::Application::Startup::Configuration& native_config) {
            TerminalEmulator = Internal::StlStringToString(native_config.terminal_emulator);
          }

          /**
           * @brief Converts this managed object to the native structure.
           * @return Native Startup::Configuration structure.
           */
          ::eCAL::Application::Startup::Configuration ToNative() {
            ::eCAL::Application::Startup::Configuration native_config;
            native_config.terminal_emulator = Internal::StringToStlString(TerminalEmulator);
            return native_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::Application::Configuration structure.
         */
        public ref class ApplicationConfiguration {
        public:
          /**
           * @brief Gets or sets the system configuration.
           */
          property SysConfiguration^ Sys;

          /**
           * @brief Gets or sets the startup configuration.
           */
          property StartupConfiguration^ Startup;

          /**
           * @brief Default constructor.
           */
          ApplicationConfiguration() {
            // Use the default values from the native structure
            ::eCAL::Application::Configuration native_config;
            Sys = gcnew SysConfiguration(native_config.sys);
            Startup = gcnew StartupConfiguration(native_config.startup);
          }

          /**
           * @brief Parameterized constructor.
           * @param native_config Native Application::Configuration structure.
           */
          ApplicationConfiguration(const ::eCAL::Application::Configuration& native_config) {
            Sys = gcnew SysConfiguration(native_config.sys);
            Startup = gcnew StartupConfiguration(native_config.startup);
          }

          /**
           * @brief Converts this managed object to the native structure.
           * @return Native Application::Configuration structure.
           */
          ::eCAL::Application::Configuration ToNative() {
            ::eCAL::Application::Configuration native_config;
            native_config.sys = Sys->ToNative();
            native_config.startup = Startup->ToNative();
            return native_config;
          }
        };

      } // namespace Config
    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse