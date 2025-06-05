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
          return Internal::StlStringToString(::eCAL::Config::GetLoadedEcalIniPath());
        }

        unsigned int Config::GetRegistrationTimeoutMs() {
          return ::eCAL::Config::GetRegistrationTimeoutMs();
        }

        unsigned int Config::GetRegistrationRefreshMs() {
          return ::eCAL::Config::GetRegistrationRefreshMs();
        }

        bool Config::IsNetworkEnabled() {
          return ::eCAL::Config::IsNetworkEnabled();
        }

        String^ Config::GetTimesyncModuleName() {
          return Internal::StlStringToString(::eCAL::Config::GetTimesyncModuleName());
        }

        String^ Config::GetTimesyncModuleReplay() {
          return Internal::StlStringToString(::eCAL::Config::GetTimesyncModuleReplay());
        }

        String^ Config::GetTerminalEmulatorCommand() {
          return Internal::StlStringToString(::eCAL::Config::GetTerminalEmulatorCommand());
        }

        String^ Config::GetEcalSysFilterExcludeList() {
          return Internal::StlStringToString(::eCAL::Config::GetEcalSysFilterExcludeList());
        }

        bool Config::GetDropOutOfOrderMessages() {
          return ::eCAL::Config::GetDropOutOfOrderMessages();
        }

        size_t Config::GetShmMonitoringQueueSize() {
          return ::eCAL::Config::GetShmMonitoringQueueSize();
        }

        String^ Config::GetShmMonitoringDomain() {
          return Internal::StlStringToString(::eCAL::Config::GetShmMonitoringDomain());
        }

        bool Config::IsShmRegistrationEnabled() {
          return ::eCAL::Config::IsShmRegistrationEnabled();
        }

        int Config::GetUdpMulticastConfigVersion() {
          return static_cast<int>(::eCAL::Config::GetUdpMulticastConfigVersion());
        }

        String^ Config::GetUdpMulticastGroup() {
          return Internal::StlStringToString(::eCAL::Config::GetUdpMulticastGroup());
        }

        String^ Config::GetUdpMulticastMask() {
          return Internal::StlStringToString(::eCAL::Config::GetUdpMulticastMask());
        }

        unsigned int Config::GetUdpMulticastPort() {
          return ::eCAL::Config::GetUdpMulticastPort();
        }

        unsigned int Config::GetUdpMulticastTtl() {
          return ::eCAL::Config::GetUdpMulticastTtl();
        }

        unsigned int Config::GetUdpMulticastSndBufSizeBytes() {
          return ::eCAL::Config::GetUdpMulticastSndBufSizeBytes();
        }

        unsigned int Config::GetUdpMulticastRcvBufSizeBytes() {
          return ::eCAL::Config::GetUdpMulticastRcvBufSizeBytes();
        }

        bool Config::IsUdpMulticastJoinAllIfEnabled() {
          return ::eCAL::Config::IsUdpMulticastJoinAllIfEnabled();
        }

        bool Config::IsUdpMulticastRecEnabled() {
          return ::eCAL::Config::IsUdpMulticastRecEnabled();
        }

        bool Config::IsShmRecEnabled() {
          return ::eCAL::Config::IsShmRecEnabled();
        }

        bool Config::IsTcpRecEnabled() {
          return ::eCAL::Config::IsTcpRecEnabled();
        }

        bool Config::IsNpcapEnabled() {
          return ::eCAL::Config::IsNpcapEnabled();
        }

        size_t Config::GetTcpPubsubReaderThreadpoolSize() {
          return ::eCAL::Config::GetTcpPubsubReaderThreadpoolSize();
        }

        size_t Config::GetTcpPubsubWriterThreadpoolSize() {
          return ::eCAL::Config::GetTcpPubsubWriterThreadpoolSize();
        }

        int Config::GetTcpPubsubMaxReconnectionAttemps() {
          return ::eCAL::Config::GetTcpPubsubMaxReconnectionAttemps();
        }

        String^ Config::GetShmTransportDomain() {
          return Internal::StlStringToString(::eCAL::Config::GetShmTransportDomain());
        }

      } // namespace Config
    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse