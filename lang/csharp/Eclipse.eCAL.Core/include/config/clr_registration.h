#pragma once

#include <string>
#include <ecal/config/registration.h>
#include <msclr/marshal_cppstd.h>

using namespace System;

namespace Eclipse {
  namespace eCAL {
    namespace Core {
      namespace Config {

        /**
         * @brief Managed wrapper for the native ::eCAL::Registration::Local::SHM::Configuration structure.
         */
        public ref class RegistrationLocalSHMConfiguration {
        public:
          property String^ Domain;
          property size_t QueueSize;

          RegistrationLocalSHMConfiguration() {
            ::eCAL::Registration::Local::SHM::Configuration native_config;
            Domain = gcnew String(native_config.domain.c_str());
            QueueSize = native_config.queue_size;
          }

          ::eCAL::Registration::Local::SHM::Configuration ToNative() {
            ::eCAL::Registration::Local::SHM::Configuration native_config;
            native_config.domain = msclr::interop::marshal_as<std::string>(Domain);
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
          property int TransportType;
          property RegistrationLocalSHMConfiguration^ SHM;
          property RegistrationLocalUDPConfiguration^ UDP;

          RegistrationLocalConfiguration() {
            ::eCAL::Registration::Local::Configuration native_config;
            TransportType = static_cast<int>(native_config.transport_type);
            SHM = gcnew RegistrationLocalSHMConfiguration();
            UDP = gcnew RegistrationLocalUDPConfiguration();
          }

          ::eCAL::Registration::Local::Configuration ToNative() {
            ::eCAL::Registration::Local::Configuration native_config;
            native_config.transport_type = static_cast<::eCAL::Registration::Local::eTransportType>(TransportType);
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
          property int TransportType;
          property RegistrationNetworkUDPConfiguration^ UDP;

          RegistrationNetworkConfiguration() {
            ::eCAL::Registration::Network::Configuration native_config;
            TransportType = static_cast<int>(native_config.transport_type);
            UDP = gcnew RegistrationNetworkUDPConfiguration();
          }

          ::eCAL::Registration::Network::Configuration ToNative() {
            ::eCAL::Registration::Network::Configuration native_config;
            native_config.transport_type = static_cast<::eCAL::Registration::Network::eTransportType>(TransportType);
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
          property String^ ShmTransportDomain;
          property RegistrationLocalConfiguration^ Local;
          property RegistrationNetworkConfiguration^ Network;

          RegistrationConfiguration() {
            ::eCAL::Registration::Configuration native_config;
            RegistrationTimeout = native_config.registration_timeout;
            RegistrationRefresh = native_config.registration_refresh;
            Loopback = native_config.loopback;
            ShmTransportDomain = gcnew String(native_config.shm_transport_domain.c_str());
            Local = gcnew RegistrationLocalConfiguration();
            Network = gcnew RegistrationNetworkConfiguration();
          }

          ::eCAL::Registration::Configuration ToNative() {
            ::eCAL::Registration::Configuration native_config;
            native_config.registration_timeout = RegistrationTimeout;
            native_config.registration_refresh = RegistrationRefresh;
            native_config.loopback = Loopback;
            native_config.shm_transport_domain = msclr::interop::marshal_as<std::string>(ShmTransportDomain);
            native_config.local = Local->ToNative();
            native_config.network = Network->ToNative();
            return native_config;
          }
        };

      } // namespace Config
    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse