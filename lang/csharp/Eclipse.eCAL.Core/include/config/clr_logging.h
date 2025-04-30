#pragma once

#include <string>
#include <ecal/config/logging.h>
#include <msclr/marshal_cppstd.h>

using namespace System;

namespace Eclipse {
  namespace eCAL {
    namespace Core {
      namespace Config {

        /**
         * @brief Managed wrapper for the native ::eCAL::Logging::Provider::Sink structure.
         */
        public ref class LoggingProviderSink {
        public:
          /**
           * @brief Gets or sets whether the sink is enabled.
           */
          property bool Enable;

          /**
           * @brief Gets or sets the log level.
           */
          property int LogLevel;

          /**
           * @brief Default constructor.
           */
          LoggingProviderSink() {
            // Use the default values from the native structure
            ::eCAL::Logging::Provider::Sink native_sink{};
            Enable = native_sink.enable;
            LogLevel = static_cast<int>(native_sink.log_level);
          }

          /**
           * @brief Parameterized constructor.
           * @param native_sink Native Sink structure.
           */
          LoggingProviderSink(const ::eCAL::Logging::Provider::Sink& native_sink) {
            Enable = native_sink.enable;
            LogLevel = static_cast<int>(native_sink.log_level);
          }

          /**
           * @brief Converts this managed object to the native structure.
           * @return Native Sink structure.
           */
          ::eCAL::Logging::Provider::Sink ToNative() {
            ::eCAL::Logging::Provider::Sink native_sink;
            native_sink.enable = Enable;
            native_sink.log_level = static_cast<::eCAL::Logging::Filter>(LogLevel);
            return native_sink;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::Logging::Provider::File::Configuration structure.
         */
        public ref class LoggingProviderFileConfiguration {
        public:
          /**
           * @brief Gets or sets the path to the log file.
           */
          property String^ Path;

          /**
           * @brief Default constructor.
           */
          LoggingProviderFileConfiguration() {
            // Use the default values from the native structure
            ::eCAL::Logging::Provider::File::Configuration native_file_config;
            Path = gcnew String(native_file_config.path.c_str());
          }

          /**
           * @brief Parameterized constructor.
           * @param native_file_config Native File::Configuration structure.
           */
          LoggingProviderFileConfiguration(const ::eCAL::Logging::Provider::File::Configuration& native_file_config) {
            Path = gcnew String(native_file_config.path.c_str());
          }

          /**
           * @brief Converts this managed object to the native structure.
           * @return Native File::Configuration structure.
           */
          ::eCAL::Logging::Provider::File::Configuration ToNative() {
            ::eCAL::Logging::Provider::File::Configuration native_file_config;
            native_file_config.path = msclr::interop::marshal_as<std::string>(Path);
            return native_file_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::Logging::Provider::UDP::Configuration structure.
         */
        public ref class LoggingProviderUDPConfiguration {
        public:
          /**
           * @brief Gets or sets the UDP port number.
           */
          property unsigned int Port;

          /**
           * @brief Default constructor.
           */
          LoggingProviderUDPConfiguration() {
            // Use the default values from the native structure
            ::eCAL::Logging::Provider::UDP::Configuration native_udp_config;
            Port = native_udp_config.port;
          }

          /**
           * @brief Parameterized constructor.
           * @param native_udp_config Native UDP::Configuration structure.
           */
          LoggingProviderUDPConfiguration(const ::eCAL::Logging::Provider::UDP::Configuration& native_udp_config) {
            Port = native_udp_config.port;
          }

          /**
           * @brief Converts this managed object to the native structure.
           * @return Native UDP::Configuration structure.
           */
          ::eCAL::Logging::Provider::UDP::Configuration ToNative() {
            ::eCAL::Logging::Provider::UDP::Configuration native_udp_config;
            native_udp_config.port = Port;
            return native_udp_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::Logging::Provider::Configuration structure.
         */
        public ref class LoggingProviderConfiguration {
        public:
          /**
           * @brief Gets or sets the console sink configuration.
           */
          property LoggingProviderSink^ Console;

          /**
           * @brief Gets or sets the file sink configuration.
           */
          property LoggingProviderSink^ File;

          /**
           * @brief Gets or sets the UDP sink configuration.
           */
          property LoggingProviderSink^ UDP;

          /**
           * @brief Gets or sets the file configuration.
           */
          property LoggingProviderFileConfiguration^ FileConfig;

          /**
           * @brief Gets or sets the UDP configuration.
           */
          property LoggingProviderUDPConfiguration^ UDPConfig;

          /**
           * @brief Default constructor.
           */
          LoggingProviderConfiguration() {
            // Use the default values from the native structure
            ::eCAL::Logging::Provider::Configuration native_provider_config;
            Console = gcnew LoggingProviderSink(native_provider_config.console);
            File = gcnew LoggingProviderSink(native_provider_config.file);
            UDP = gcnew LoggingProviderSink(native_provider_config.udp);
            FileConfig = gcnew LoggingProviderFileConfiguration(native_provider_config.file_config);
            UDPConfig = gcnew LoggingProviderUDPConfiguration(native_provider_config.udp_config);
          }

          /**
           * @brief Parameterized constructor.
           * @param native_provider_config Native Provider::Configuration structure.
           */
          LoggingProviderConfiguration(const ::eCAL::Logging::Provider::Configuration& native_provider_config) {
            Console = gcnew LoggingProviderSink(native_provider_config.console);
            File = gcnew LoggingProviderSink(native_provider_config.file);
            UDP = gcnew LoggingProviderSink(native_provider_config.udp);
            FileConfig = gcnew LoggingProviderFileConfiguration(native_provider_config.file_config);
            UDPConfig = gcnew LoggingProviderUDPConfiguration(native_provider_config.udp_config);
          }

          /**
           * @brief Converts this managed object to the native structure.
           * @return Native Provider::Configuration structure.
           */
          ::eCAL::Logging::Provider::Configuration ToNative() {
            ::eCAL::Logging::Provider::Configuration native_provider_config;
            native_provider_config.console = Console->ToNative();
            native_provider_config.file = File->ToNative();
            native_provider_config.udp = UDP->ToNative();
            native_provider_config.file_config = FileConfig->ToNative();
            native_provider_config.udp_config = UDPConfig->ToNative();
            return native_provider_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::Logging::Provider::UDP::Configuration structure.
         */
        public ref class LoggingReceiverUDPConfiguration {
          public:
            /**
             * @brief Gets or sets the UDP port number.
             */
            property unsigned int Port;
  
            /**
             * @brief Default constructor.
             */
            LoggingReceiverUDPConfiguration() {
              // Use the default values from the native structure
              ::eCAL::Logging::Receiver::UDP::Configuration native_udp_config;
              Port = native_udp_config.port;
            }
  
            /**
             * @brief Parameterized constructor.
             * @param native_udp_config Native UDP::Configuration structure.
             */
            LoggingReceiverUDPConfiguration(const ::eCAL::Logging::Receiver::UDP::Configuration& native_udp_config) {
              Port = native_udp_config.port;
            }
  
            /**
             * @brief Converts this managed object to the native structure.
             * @return Native UDP::Configuration structure.
             */
            ::eCAL::Logging::Receiver::UDP::Configuration ToNative() {
              ::eCAL::Logging::Receiver::UDP::Configuration native_udp_config;
              native_udp_config.port = Port;
              return native_udp_config;
            }
          };

        /**
         * @brief Managed wrapper for the native ::eCAL::Logging::Receiver::Configuration structure.
         */
        public ref class LoggingReceiverConfiguration {
        public:
          /**
           * @brief Gets or sets whether the receiver is enabled.
           */
          property bool Enable;

          /**
           * @brief Gets or sets the UDP configuration.
           */
          property LoggingReceiverUDPConfiguration^ UDPConfig;

          /**
           * @brief Default constructor.
           */
          LoggingReceiverConfiguration() {
            // Use the default values from the native structure
            ::eCAL::Logging::Receiver::Configuration native_receiver_config;
            Enable = native_receiver_config.enable;
            UDPConfig = gcnew LoggingReceiverUDPConfiguration(native_receiver_config.udp_config);
          }

          /**
           * @brief Parameterized constructor.
           * @param native_receiver_config Native Receiver::Configuration structure.
           */
          LoggingReceiverConfiguration(const ::eCAL::Logging::Receiver::Configuration& native_receiver_config) {
            Enable = native_receiver_config.enable;
            UDPConfig = gcnew LoggingReceiverUDPConfiguration(native_receiver_config.udp_config);
          }

          /**
           * @brief Converts this managed object to the native structure.
           * @return Native Receiver::Configuration structure.
           */
          ::eCAL::Logging::Receiver::Configuration ToNative() {
            ::eCAL::Logging::Receiver::Configuration native_receiver_config;
            native_receiver_config.enable = Enable;
            native_receiver_config.udp_config = UDPConfig->ToNative();
            return native_receiver_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::Logging::Configuration structure.
         */
        public ref class LoggingConfiguration {
        public:
          /**
           * @brief Gets or sets the provider configuration.
           */
          property LoggingProviderConfiguration^ Provider;

          /**
           * @brief Gets or sets the receiver configuration.
           */
          property LoggingReceiverConfiguration^ Receiver;

          /**
           * @brief Default constructor.
           */
          LoggingConfiguration() {
            // Use the default values from the native structure
            ::eCAL::Logging::Configuration native_config;
            Provider = gcnew LoggingProviderConfiguration(native_config.provider);
            Receiver = gcnew LoggingReceiverConfiguration(native_config.receiver);
          }

          /**
           * @brief Converts this managed object to the native structure.
           * @return Native Logging::Configuration structure.
           */
          ::eCAL::Logging::Configuration ToNative() {
            ::eCAL::Logging::Configuration native_config;
            native_config.provider = Provider->ToNative();
            native_config.receiver = Receiver->ToNative();
            return native_config;
          }
        };

      } // namespace Config
    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse