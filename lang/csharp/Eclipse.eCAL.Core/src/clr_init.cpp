#include "clr_init.h"

namespace Eclipse {
  namespace eCAL {
    namespace Core {
        Config::Configuration^ Init::InitConfiguration() {
            ::eCAL::Configuration native_config = ::eCAL::Init::Configuration();
            Config::Configuration^ managed_config = gcnew Config::Configuration();
            managed_config->TransportLayer = gcnew Config::TransportLayerConfiguration();
            managed_config->Registration = gcnew Config::RegistrationConfiguration();
            managed_config->Subscriber = gcnew Config::SubscriberConfiguration();
            managed_config->Publisher = gcnew Config::PublisherConfiguration();
            managed_config->TimeSync = gcnew Config::TimeConfiguration();
            managed_config->Application = gcnew Config::ApplicationConfiguration();
            managed_config->Logging = gcnew Config::LoggingConfiguration();
            managed_config->CommunicationMode = static_cast<int>(native_config.communication_mode);
            return managed_config;
          }
    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse