#include "clr_config.h"
#include <msclr/marshal_cppstd.h>
        
using namespace Eclipse::eCAL::Core::Config;
using namespace System;

namespace Eclipse {
  namespace eCAL {
    namespace Core {
      namespace Config {
        Configuration^ Config::GetConfiguration() {
          const ::eCAL::Configuration& native_config = ::eCAL::GetConfiguration();
          return gcnew Configuration(native_config);
        }

        SubscriberConfiguration^ Config::GetSubscriberConfiguration() {
          const ::eCAL::Subscriber::Configuration& native_config = ::eCAL::GetSubscriberConfiguration();
          return gcnew SubscriberConfiguration(native_config);
        }

        PublisherConfiguration^ Config::GetPublisherConfiguration() {
          const ::eCAL::Publisher::Configuration& native_config = ::eCAL::GetPublisherConfiguration();
          return gcnew PublisherConfiguration(native_config);
        }

        String^ Config::GetLoadedEcalIniPath() {
          return gcnew String(::eCAL::Config::GetLoadedEcalIniPath().c_str());
        }

        int Config::GetRegistrationTimeoutMs() {
          return ::eCAL::Config::GetRegistrationTimeoutMs();
        }

        int Config::GetRegistrationRefreshMs() {
          return ::eCAL::Config::GetRegistrationRefreshMs();
        }

        bool Config::IsNetworkEnabled() {
          return ::eCAL::Config::IsNetworkEnabled();
        }

        String^ Config::GetTimesyncModuleName() {
          return gcnew String(::eCAL::Config::GetTimesyncModuleName().c_str());
        }

        String^ Config::GetTimesyncModuleReplay() {
          return gcnew String(::eCAL::Config::GetTimesyncModuleReplay().c_str());
        }

        String^ Config::GetTerminalEmulatorCommand() {
          return gcnew String(::eCAL::Config::GetTerminalEmulatorCommand().c_str());
        }

        String^ Config::GetEcalSysFilterExcludeList() {
          return gcnew String(::eCAL::Config::GetEcalSysFilterExcludeList().c_str());
        }

        bool Config::GetDropOutOfOrderMessages() {
          return ::eCAL::Config::GetDropOutOfOrderMessages();
        }

        size_t Config::GetShmMonitoringQueueSize() {
          return ::eCAL::Config::GetShmMonitoringQueueSize();
        }

        String^ Config::GetShmMonitoringDomain() {
          return gcnew String(::eCAL::Config::GetShmMonitoringDomain().c_str());
        }

      } // namespace Config
    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse