#pragma once

#include <string>
#include <ecal/config/configuration.h>

#include "clr_application.h"
#include "clr_common.h"
#include "clr_registration.h"
#include "clr_logging.h"
#include "clr_publisher.h"
#include "clr_subscriber.h"
#include "clr_time.h"
#include "clr_transport_layer.h"


namespace Eclipse {
  namespace eCAL {
    namespace Core {
      namespace Config {

        /**
         * @brief Managed wrapper for the native ::eCAL::Configuration structure.
         */
        public ref class Configuration {
        public:
          /**
           * @brief Gets or sets the transport layer configuration.
           */
          property TransportLayerConfiguration^ TransportLayer;

          /**
           * @brief Gets or sets the registration configuration.
           */
          property RegistrationConfiguration^ Registration;

          /**
           * @brief Gets or sets the subscriber configuration.
           */
          property SubscriberConfiguration^ Subscriber;

          /**
           * @brief Gets or sets the publisher configuration.
           */
          property PublisherConfiguration^ Publisher;

          /**
           * @brief Gets or sets the time synchronization configuration.
           */
          property TimeConfiguration^ TimeSync;

          /**
           * @brief Gets or sets the application configuration.
           */
          property ApplicationConfiguration^ Application;

          /**
           * @brief Gets or sets the logging configuration.
           */
          property LoggingConfiguration^ Logging;

          /**
           * @brief Gets or sets the communication mode.
           */
          property int CommunicationMode;

          /**
           * @brief Default constructor.
           */
          Configuration() {
            native_config = new ::eCAL::Configuration();
            TransportLayer = gcnew TransportLayerConfiguration();
            Registration = gcnew RegistrationConfiguration();
            Subscriber = gcnew SubscriberConfiguration();
            Publisher = gcnew PublisherConfiguration();
            TimeSync = gcnew TimeConfiguration();
            Application = gcnew ApplicationConfiguration();
            Logging = gcnew LoggingConfiguration();
            CommunicationMode = static_cast<int>(native_config->communication_mode);
          }

          /**
           * @brief Native struct constructor.
           * @param native_config Native ::eCAL::Configuration structure.
           */
          Configuration(const ::eCAL::Configuration& native_config) {
            // Initialize managed properties from the native struct
            TransportLayer = gcnew TransportLayerConfiguration(native_config.transport_layer);
            Registration   = gcnew RegistrationConfiguration(native_config.registration);
            Subscriber     = gcnew SubscriberConfiguration(native_config.subscriber);
            Publisher      = gcnew PublisherConfiguration(native_config.publisher);
            TimeSync       = gcnew TimeConfiguration(native_config.timesync);
            Application    = gcnew ApplicationConfiguration(native_config.application);
            Logging        = gcnew LoggingConfiguration(native_config.logging);
            CommunicationMode = static_cast<int>(native_config.communication_mode);

            // Optionally, you can also initialize the native_config pointer if needed:
            this->native_config = new ::eCAL::Configuration(native_config);
          }

          /**
           * @brief Destructor to clean up native resources.
           */
          ~Configuration() {
            delete native_config;
          }

          /**
           * @brief Initializes the configuration from the default configuration.
           */
          void InitFromConfig() {
            native_config->InitFromConfig();
            UpdateManagedProperties();
          }

          /**
           * @brief Initializes the configuration from a YAML file.
           * @param yamlPath Path to the YAML configuration file.
           */
          void InitFromFile(System::String^ yamlPath) {
            std::string native_yaml_path = Internal::StringToStlString(yamlPath);
            native_config->InitFromFile(native_yaml_path);
            UpdateManagedProperties();
          }

          /**
           * @brief Gets the path to the current configuration file.
           * @return Path to the configuration file.
           */
          System::String^ GetConfigurationFilePath() {
            return Internal::StlStringToString(native_config->GetConfigurationFilePath());
          }

          /**
           * @brief Converts this managed object to the native structure.
           * @return Native ::eCAL::Configuration structure.
           */
          ::eCAL::Configuration ToNative() {
            ::eCAL::Configuration native_config_copy;
            native_config_copy.transport_layer = TransportLayer->ToNative();
            native_config_copy.registration = Registration->ToNative();
            native_config_copy.subscriber = Subscriber->ToNative();
            native_config_copy.publisher = Publisher->ToNative();
            native_config_copy.timesync = TimeSync->ToNative();
            native_config_copy.application = Application->ToNative();
            native_config_copy.logging = Logging->ToNative();
            native_config_copy.communication_mode = static_cast<::eCAL::eCommunicationMode>(CommunicationMode);
            return native_config_copy;
          }

        private:
          ::eCAL::Configuration* native_config;

          /**
           * @brief Updates the managed properties from the native configuration.
           */
          void UpdateManagedProperties() {
            TransportLayer = gcnew TransportLayerConfiguration();
            Registration = gcnew RegistrationConfiguration();
            Subscriber = gcnew SubscriberConfiguration();
            Publisher = gcnew PublisherConfiguration();
            TimeSync = gcnew TimeConfiguration();
            Application = gcnew ApplicationConfiguration();
            Logging = gcnew LoggingConfiguration();
            CommunicationMode = static_cast<int>(native_config->communication_mode);
          }
        };

      } // namespace Config
    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse