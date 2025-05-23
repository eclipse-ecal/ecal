#pragma once

#include <string>
#include <ecal/config.h>
#include <msclr/marshal_cppstd.h>

#include "clr_common.h"
#include "config/clr_configuration.h"

namespace Eclipse {
  namespace eCAL {
    namespace Core {
      namespace Config {

        public ref class Config {
        public:
          /**
           * @brief Gets the global eCAL configuration.
           * @return Managed wrapper for the global configuration.
           */
          static Configuration^ GetConfiguration();

          /**
           * @brief Gets the subscriber configuration.
           * @return Managed wrapper for the subscriber configuration.
           */
          static SubscriberConfiguration^ GetSubscriberConfiguration();

          /**
           * @brief Gets the publisher configuration.
           * @return Managed wrapper for the publisher configuration.
           */
          static PublisherConfiguration^ GetPublisherConfiguration();

          /**
           * @brief Gets the path to the loaded eCAL INI file.
           * @return Path to the loaded INI file.
           */
          static System::String^ GetLoadedEcalIniPath();

          /**
           * @brief Gets the registration timeout in milliseconds.
           * @return Registration timeout in milliseconds.
           */
          static int GetRegistrationTimeoutMs();

          /**
           * @brief Gets the registration refresh interval in milliseconds.
           * @return Registration refresh interval in milliseconds.
           */
          static int GetRegistrationRefreshMs();

          /**
           * @brief Checks if the network is enabled.
           * @return True if the network is enabled, otherwise false.
           */
          static bool IsNetworkEnabled();

          /**
           * @brief Gets the time synchronization module name.
           * @return Name of the time synchronization module.
           */
          static System::String^ GetTimesyncModuleName();

          /**
           * @brief Gets the time synchronization module for replay.
           * @return Name of the time synchronization module for replay.
           */
          static System::String^ GetTimesyncModuleReplay();

          /**
           * @brief Gets the terminal emulator command.
           * @return Terminal emulator command.
           */
          static System::String^ GetTerminalEmulatorCommand();

          /**
           * @brief Gets the eCAL sys filter exclusion list.
           * @return Sysfilter exclusion list as String.
           */
          static System::String^ GetEcalSysFilterExcludeList();

          /**
           * @brief Checks if out-of-order messages are dropped for subscribers.
           * @return True if out-of-order messages are dropped, otherwise false.
           */
          static bool GetDropOutOfOrderMessages();

          /**
           * @brief Gets the shared memory monitoring queue size.
           * @return Shared memory monitoring queue size.
           */
          static size_t GetShmMonitoringQueueSize();

          /**
           * @brief Gets the shared memory monitoring domain.
           * @return Shared memory monitoring domain as string.
           */
          static System::String^ GetShmMonitoringDomain();
        };

      } // namespace Config
    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse