#include "clr_config.h"

namespace Eclipse {
  namespace eCAL {
    namespace Core {
      namespace Config {
        Configuration^ ConfigWrapper::GetConfiguration() {
          const ::eCAL::Configuration& native_config = ::eCAL::GetConfiguration();
          Configuration^ managed_config = gcnew Configuration();
          managed_config->TransportLayer = gcnew TransportLayerConfiguration();
          managed_config->Registration = gcnew RegistrationConfiguration();
          managed_config->Subscriber = gcnew SubscriberConfiguration();
          managed_config->Publisher = gcnew PublisherConfiguration();
          managed_config->TimeSync = gcnew TimeConfiguration();
          managed_config->Application = gcnew ApplicationConfiguration();
          managed_config->Logging = gcnew LoggingConfiguration();
          managed_config->CommunicationMode = static_cast<int>(native_config.communication_mode);
          return managed_config;
        }

        SubscriberConfiguration^ ConfigWrapper::GetSubscriberConfiguration() {
          const ::eCAL::Subscriber::Configuration& native_config = ::eCAL::GetSubscriberConfiguration();
          SubscriberConfiguration^ managed_config = gcnew SubscriberConfiguration();
          managed_config->Layer = gcnew SubscriberLayerConfiguration();
          managed_config->DropOutOfOrderMessages = native_config.drop_out_of_order_messages;
          return managed_config;
        }

        PublisherConfiguration^ ConfigWrapper::GetPublisherConfiguration() {
          const ::eCAL::Publisher::Configuration& native_config = ::eCAL::GetPublisherConfiguration();
          PublisherConfiguration^ managed_config = gcnew PublisherConfiguration();
          managed_config->Layer = gcnew PublisherLayerConfiguration();
          managed_config->LayerPriorityLocal = gcnew System::Collections::Generic::List<int>();
          for (auto layer : native_config.layer_priority_local) {
            managed_config->LayerPriorityLocal->Add(static_cast<int>(layer));
          }
          managed_config->LayerPriorityRemote = gcnew System::Collections::Generic::List<int>();
          for (auto layer : native_config.layer_priority_remote) {
            managed_config->LayerPriorityRemote->Add(static_cast<int>(layer));
          }
          return managed_config;
        }

        String^ ConfigWrapper::GetLoadedEcalIniPath() {
          return gcnew String(::eCAL::Config::GetLoadedEcalIniPath().c_str());
        }

        int ConfigWrapper::GetRegistrationTimeoutMs() {
          return ::eCAL::Config::GetRegistrationTimeoutMs();
        }

        int ConfigWrapper::GetRegistrationRefreshMs() {
          return ::eCAL::Config::GetRegistrationRefreshMs();
        }

        bool ConfigWrapper::IsNetworkEnabled() {
          return ::eCAL::Config::IsNetworkEnabled();
        }

        String^ ConfigWrapper::GetTimesyncModuleName() {
          return gcnew String(::eCAL::Config::GetTimesyncModuleName().c_str());
        }

        String^ ConfigWrapper::GetTimesyncModuleReplay() {
          return gcnew String(::eCAL::Config::GetTimesyncModuleReplay().c_str());
        }

        String^ ConfigWrapper::GetTerminalEmulatorCommand() {
          return gcnew String(::eCAL::Config::GetTerminalEmulatorCommand().c_str());
        }

        String^ ConfigWrapper::GetEcalSysFilterExcludeList() {
          return gcnew String(::eCAL::Config::GetEcalSysFilterExcludeList().c_str());
        }

        bool ConfigWrapper::GetDropOutOfOrderMessages() {
          return ::eCAL::Config::GetDropOutOfOrderMessages();
        }

        size_t ConfigWrapper::GetShmMonitoringQueueSize() {
          return ::eCAL::Config::GetShmMonitoringQueueSize();
        }

        String^ ConfigWrapper::GetShmMonitoringDomain() {
          return gcnew String(::eCAL::Config::GetShmMonitoringDomain().c_str());
        }

      } // namespace Config
    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse