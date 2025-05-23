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
          static unsigned int GetRegistrationTimeoutMs();

          /**
           * @brief Gets the registration refresh interval in milliseconds.
           * @return Registration refresh interval in milliseconds.
           */
          static unsigned int GetRegistrationRefreshMs();

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

          /**
           * @brief Checks if shared memory registration is enabled.
           * @return True if enabled, otherwise false.
           */
          static bool IsShmRegistrationEnabled();

          /**
           * @brief Gets the UDP multicast config version.
           * @return UDP multicast config version.
           */
          static int GetUdpMulticastConfigVersion();

          /**
           * @brief Gets the UDP multicast group.
           * @return UDP multicast group as string.
           */
          static System::String^ GetUdpMulticastGroup();

          /**
           * @brief Gets the UDP multicast mask.
           * @return UDP multicast mask as string.
           */
          static System::String^ GetUdpMulticastMask();

          /**
           * @brief Gets the UDP multicast port.
           * @return UDP multicast port.
           */
          static unsigned int GetUdpMulticastPort();

          /**
           * @brief Gets the UDP multicast TTL.
           * @return UDP multicast TTL.
           */
          static unsigned int GetUdpMulticastTtl();

          /**
           * @brief Gets the UDP multicast send buffer size in bytes.
           * @return Send buffer size in bytes.
           */
          static unsigned int GetUdpMulticastSndBufSizeBytes();

          /**
           * @brief Gets the UDP multicast receive buffer size in bytes.
           * @return Receive buffer size in bytes.
           */
          static unsigned int GetUdpMulticastRcvBufSizeBytes();

          /**
           * @brief Checks if UDP multicast join-all-interfaces is enabled.
           * @return True if enabled, otherwise false.
           */
          static bool IsUdpMulticastJoinAllIfEnabled();

          /**
           * @brief Checks if UDP multicast receive is enabled.
           * @return True if enabled, otherwise false.
           */
          static bool IsUdpMulticastRecEnabled();

          /**
           * @brief Checks if shared memory receive is enabled.
           * @return True if enabled, otherwise false.
           */
          static bool IsShmRecEnabled();

          /**
           * @brief Checks if TCP receive is enabled.
           * @return True if enabled, otherwise false.
           */
          static bool IsTcpRecEnabled();

          /**
           * @brief Checks if NPCAP is enabled.
           * @return True if enabled, otherwise false.
           */
          static bool IsNpcapEnabled();

          /**
           * @brief Gets the TCP pubsub reader threadpool size.
           * @return Threadpool size.
           */
          static size_t GetTcpPubsubReaderThreadpoolSize();

          /**
           * @brief Gets the TCP pubsub writer threadpool size.
           * @return Threadpool size.
           */
          static size_t GetTcpPubsubWriterThreadpoolSize();

          /**
           * @brief Gets the TCP pubsub max reconnection attempts.
           * @return Max reconnection attempts.
           */
          static int GetTcpPubsubMaxReconnectionAttemps();

          /**
           * @brief Gets the shared memory transport domain.
           * @return Shared memory transport domain as string.
           */
          static System::String^ GetShmTransportDomain();
        };

      } // namespace Config
    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse