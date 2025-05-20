#include "clr_config.h"
#include <msclr/marshal_cppstd.h>
        
using namespace Eclipse::eCAL::Core::Config;

namespace Eclipse {
  namespace eCAL {
    namespace Core {
      namespace Config {
        Configuration^ ConfigWrapper::GetConfiguration() {
          const ::eCAL::Configuration& native_config = ::eCAL::GetConfiguration();
          return gcnew Configuration(native_config);
        }

        SubscriberConfiguration^ ConfigWrapper::GetSubscriberConfiguration() {
          const ::eCAL::Subscriber::Configuration& native_config = ::eCAL::GetSubscriberConfiguration();
          return gcnew SubscriberConfiguration(native_config);
        }

        PublisherConfiguration^ ConfigWrapper::GetPublisherConfiguration() {
          const ::eCAL::Publisher::Configuration& native_config = ::eCAL::GetPublisherConfiguration();
          return gcnew PublisherConfiguration(native_config);
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