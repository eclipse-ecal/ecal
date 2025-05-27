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
#include <ecal/config/registration.h>

#include "clr_common.h"

namespace Eclipse {
  namespace eCAL {
    namespace Core {
      namespace Config {

        /**
         * @brief Specifies the local transport types available for registration.
         */
        public enum class eRegistrationLocalTransportType
        {
          Shm = ::eCAL::Registration::Local::eTransportType::shm,
          Udp = ::eCAL::Registration::Local::eTransportType::udp
        };

        /**
         * @brief Helper class for converting between native and managed local transport types.
         */
        public ref class RegistrationLocalTransportTypeHelper abstract sealed
        {
          public:
            static eRegistrationLocalTransportType FromNative(::eCAL::Registration::Local::eTransportType nativeType)
            {
              return static_cast<eRegistrationLocalTransportType>(nativeType);
            }

            static ::eCAL::Registration::Local::eTransportType ToNative(eRegistrationLocalTransportType managedType)
            {
              return static_cast<::eCAL::Registration::Local::eTransportType>(managedType);
            }
        };

        /**
         * @brief Specifies the network transport types available for registration.
         */
        public enum class eRegistrationNetworkTransportType
        {
          Udp = ::eCAL::Registration::Network::eTransportType::udp
        };

        /**
         * @brief Helper class for converting between native and managed network transport types.
         */
        public ref class RegistrationNetworkTransportTypeHelper abstract sealed
        {
          public:
            static eRegistrationNetworkTransportType FromNative(::eCAL::Registration::Network::eTransportType nativeType)
            {
              return static_cast<eRegistrationNetworkTransportType>(nativeType);
            }

            static ::eCAL::Registration::Network::eTransportType ToNative(eRegistrationNetworkTransportType managedType)
            {
              return static_cast<::eCAL::Registration::Network::eTransportType>(managedType);
            }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::Registration::Local::SHM::Configuration structure.
         */
        public ref class RegistrationLocalSHMConfiguration {
        public:
          property System::String^ Domain;
          property size_t QueueSize;

          RegistrationLocalSHMConfiguration() {
            ::eCAL::Registration::Local::SHM::Configuration native_config;
            Domain = Internal::StlStringToString(native_config.domain);
            QueueSize = native_config.queue_size;
          }

          // Native struct constructor
          RegistrationLocalSHMConfiguration(const ::eCAL::Registration::Local::SHM::Configuration& native_config) {
            Domain = Internal::StlStringToString(native_config.domain);
            QueueSize = native_config.queue_size;
          }

          ::eCAL::Registration::Local::SHM::Configuration ToNative() {
            ::eCAL::Registration::Local::SHM::Configuration native_config;
            native_config.domain = Internal::StringToStlString(Domain);
            native_config.queue_size = QueueSize;
            return native_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::Registration::Local::UDP::Configuration structure.
         */
        public ref class RegistrationLocalUDPConfiguration {
        public:
          property unsigned int Port;

          RegistrationLocalUDPConfiguration() {
            ::eCAL::Registration::Local::UDP::Configuration native_config;
            Port = native_config.port;
          }

          // Native struct constructor
          RegistrationLocalUDPConfiguration(const ::eCAL::Registration::Local::UDP::Configuration& native_config) {
            Port = native_config.port;
          }

          ::eCAL::Registration::Local::UDP::Configuration ToNative() {
            ::eCAL::Registration::Local::UDP::Configuration native_config;
            native_config.port = Port;
            return native_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::Registration::Local::Configuration structure.
         */
        public ref class RegistrationLocalConfiguration {
        public:
          property eRegistrationLocalTransportType TransportType;
          property RegistrationLocalSHMConfiguration^ SHM;
          property RegistrationLocalUDPConfiguration^ UDP;

          RegistrationLocalConfiguration() {
            ::eCAL::Registration::Local::Configuration native_config;
            TransportType = RegistrationLocalTransportTypeHelper::FromNative(native_config.transport_type);
            SHM = gcnew RegistrationLocalSHMConfiguration(native_config.shm);
            UDP = gcnew RegistrationLocalUDPConfiguration(native_config.udp);
          }

          // Native struct constructor
          RegistrationLocalConfiguration(const ::eCAL::Registration::Local::Configuration& native_config) {
            TransportType = RegistrationLocalTransportTypeHelper::FromNative(native_config.transport_type);
            SHM = gcnew RegistrationLocalSHMConfiguration(native_config.shm);
            UDP = gcnew RegistrationLocalUDPConfiguration(native_config.udp);
          }

          ::eCAL::Registration::Local::Configuration ToNative() {
            ::eCAL::Registration::Local::Configuration native_config;
            native_config.transport_type = RegistrationLocalTransportTypeHelper::ToNative(TransportType);
            native_config.shm = SHM->ToNative();
            native_config.udp = UDP->ToNative();
            return native_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::Registration::Network::UDP::Configuration structure.
         */
        public ref class RegistrationNetworkUDPConfiguration {
        public:
          property unsigned int Port;

          RegistrationNetworkUDPConfiguration() {
            ::eCAL::Registration::Network::UDP::Configuration native_config;
            Port = native_config.port;
          }

          // Native struct constructor
          RegistrationNetworkUDPConfiguration(const ::eCAL::Registration::Network::UDP::Configuration& native_config) {
            Port = native_config.port;
          }

          ::eCAL::Registration::Network::UDP::Configuration ToNative() {
            ::eCAL::Registration::Network::UDP::Configuration native_config;
            native_config.port = Port;
            return native_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::Registration::Network::Configuration structure.
         */
        public ref class RegistrationNetworkConfiguration {
        public:
          property eRegistrationNetworkTransportType TransportType;
          property RegistrationNetworkUDPConfiguration^ UDP;

          RegistrationNetworkConfiguration() {
            ::eCAL::Registration::Network::Configuration native_config;
            TransportType = RegistrationNetworkTransportTypeHelper::FromNative(native_config.transport_type);
            UDP = gcnew RegistrationNetworkUDPConfiguration(native_config.udp);
          }

          // Native struct constructor
          RegistrationNetworkConfiguration(const ::eCAL::Registration::Network::Configuration& native_config) {
            TransportType = RegistrationNetworkTransportTypeHelper::FromNative(native_config.transport_type);
            UDP = gcnew RegistrationNetworkUDPConfiguration(native_config.udp);
          }

          ::eCAL::Registration::Network::Configuration ToNative() {
            ::eCAL::Registration::Network::Configuration native_config;
            native_config.transport_type = RegistrationNetworkTransportTypeHelper::ToNative(TransportType);
            native_config.udp = UDP->ToNative();
            return native_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::Registration::Configuration structure.
         */
        public ref class RegistrationConfiguration {
        public:
          property unsigned int RegistrationTimeout;
          property unsigned int RegistrationRefresh;
          property bool Loopback;
          property System::String^ ShmTransportDomain;
          property RegistrationLocalConfiguration^ Local;
          property RegistrationNetworkConfiguration^ Network;

          RegistrationConfiguration() {
            ::eCAL::Registration::Configuration native_config;
            RegistrationTimeout = native_config.registration_timeout;
            RegistrationRefresh = native_config.registration_refresh;
            Loopback = native_config.loopback;
            ShmTransportDomain = Internal::StlStringToString(native_config.shm_transport_domain);
            Local = gcnew RegistrationLocalConfiguration(native_config.local);
            Network = gcnew RegistrationNetworkConfiguration(native_config.network);
          }

          // Native struct constructor
          RegistrationConfiguration(const ::eCAL::Registration::Configuration& native_config) {
            RegistrationTimeout = native_config.registration_timeout;
            RegistrationRefresh = native_config.registration_refresh;
            Loopback = native_config.loopback;
            ShmTransportDomain = Internal::StlStringToString(native_config.shm_transport_domain);
            Local = gcnew RegistrationLocalConfiguration(native_config.local);
            Network = gcnew RegistrationNetworkConfiguration(native_config.network);
          }

          ::eCAL::Registration::Configuration ToNative() {
            ::eCAL::Registration::Configuration native_config;
            native_config.registration_timeout = RegistrationTimeout;
            native_config.registration_refresh = RegistrationRefresh;
            native_config.loopback = Loopback;
            native_config.shm_transport_domain = Internal::StringToStlString(ShmTransportDomain);
            native_config.local = Local->ToNative();
            native_config.network = Network->ToNative();
            return native_config;
          }
        };

      } // namespace Config
    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse