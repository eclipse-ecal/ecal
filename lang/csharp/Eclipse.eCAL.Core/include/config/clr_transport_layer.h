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

#pragma once

#include <string>
#include <ecal/config/transport_layer.h>

#include "clr_common.h"
#include "clr_types.h"

namespace Eclipse {
  namespace eCAL {
    namespace Core {
      namespace Config {

        /**
         * @brief Managed enum for eCAL transport layer types, matching native eCAL::TransportLayer::eType.
         */
        public enum class eTransportLayerType
        {
          None = ::eCAL::TransportLayer::eType::none,
          UdpMc = ::eCAL::TransportLayer::eType::udp_mc,
          Shm = ::eCAL::TransportLayer::eType::shm,
          Tcp = ::eCAL::TransportLayer::eType::tcp
        };

        /**
         * @brief Helper class for converting between managed TransportLayereType and native eCAL::TransportLayer::eType.
         */
        public ref class TransportLayerTypeHelper abstract sealed
        {
        public:
          static eTransportLayerType FromNative(::eCAL::TransportLayer::eType nativeType)
          {
            return static_cast<eTransportLayerType>(nativeType);
          }

          static ::eCAL::TransportLayer::eType ToNative(eTransportLayerType managedType)
          {
            return static_cast<::eCAL::TransportLayer::eType>(managedType);
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::TransportLayer::UDP::MulticastConfiguration structure.
         */
        public ref class TransportLayerUdpMulticastConfiguration {
        public:
          property System::String^ Group;
          property unsigned int Ttl;

          TransportLayerUdpMulticastConfiguration() {
            ::eCAL::TransportLayer::UDP::MulticastConfiguration native_config;
            Group = Internal::StlStringToString(native_config.group.Get());
            Ttl = native_config.ttl;
          }

          // Native struct constructor
          TransportLayerUdpMulticastConfiguration(const ::eCAL::TransportLayer::UDP::MulticastConfiguration& native_config) {
            Group = Internal::StlStringToString(native_config.group.Get());
            Ttl = native_config.ttl;
          }

          ::eCAL::TransportLayer::UDP::MulticastConfiguration ToNative() {
            ::eCAL::TransportLayer::UDP::MulticastConfiguration native_config;
            native_config.group = Internal::StringToStlString(Group);
            native_config.ttl = Ttl;
            return native_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::TransportLayer::UDP::Configuration structure.
         */
        public ref class TransportLayerUdpConfiguration {
        public:
          property TypesUdpConfigVersion ConfigVersion;
          property unsigned int Port;
          property System::String^ Mask;
          property unsigned int SendBuffer;
          property unsigned int ReceiveBuffer;
          property bool JoinAllInterfaces;
          property bool NpcapEnabled;
          property TransportLayerUdpMulticastConfiguration^ Network;
          property TransportLayerUdpMulticastConfiguration^ Local;

          TransportLayerUdpConfiguration() {
            ::eCAL::TransportLayer::UDP::Configuration native_config;
            ConfigVersion = TypesUdpConfigVersionHelper::FromNative(native_config.config_version);
            Port = native_config.port;
            Mask = Internal::StlStringToString(native_config.mask.Get());
            SendBuffer = native_config.send_buffer;
            ReceiveBuffer = native_config.receive_buffer;
            JoinAllInterfaces = native_config.join_all_interfaces;
            NpcapEnabled = native_config.npcap_enabled;
            Network = gcnew TransportLayerUdpMulticastConfiguration(native_config.network);
            Local = gcnew TransportLayerUdpMulticastConfiguration(native_config.local);
          }

          // Native struct constructor
          TransportLayerUdpConfiguration(const ::eCAL::TransportLayer::UDP::Configuration& native_config) {
            ConfigVersion = TypesUdpConfigVersionHelper::FromNative(native_config.config_version);
            Port = native_config.port;
            Mask = Internal::StlStringToString(native_config.mask.Get());
            SendBuffer = native_config.send_buffer;
            ReceiveBuffer = native_config.receive_buffer;
            JoinAllInterfaces = native_config.join_all_interfaces;
            NpcapEnabled = native_config.npcap_enabled;
            Network = gcnew TransportLayerUdpMulticastConfiguration(native_config.network);
            Local = gcnew TransportLayerUdpMulticastConfiguration(native_config.local);
          }

          ::eCAL::TransportLayer::UDP::Configuration ToNative() {
            ::eCAL::TransportLayer::UDP::Configuration native_config;
            native_config.config_version = TypesUdpConfigVersionHelper::ToNative(ConfigVersion);
            native_config.port = Port;
            native_config.mask = Internal::StringToStlString(Mask);
            native_config.send_buffer = SendBuffer;
            native_config.receive_buffer = ReceiveBuffer;
            native_config.join_all_interfaces = JoinAllInterfaces;
            native_config.npcap_enabled = NpcapEnabled;
            native_config.network = Network->ToNative();
            native_config.local = Local->ToNative();
            return native_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::TransportLayer::TCP::Configuration structure.
         */
        public ref class TransportLayerTcpConfiguration {
        public:
          property size_t NumberExecutorReader;
          property size_t NumberExecutorWriter;
          property int MaxReconnections;

          TransportLayerTcpConfiguration() {
            ::eCAL::TransportLayer::TCP::Configuration native_config;
            NumberExecutorReader = native_config.number_executor_reader;
            NumberExecutorWriter = native_config.number_executor_writer;
            MaxReconnections = native_config.max_reconnections;
          }

          // Native struct constructor
          TransportLayerTcpConfiguration(const ::eCAL::TransportLayer::TCP::Configuration& native_config) {
            NumberExecutorReader = native_config.number_executor_reader;
            NumberExecutorWriter = native_config.number_executor_writer;
            MaxReconnections = native_config.max_reconnections;
          }

          ::eCAL::TransportLayer::TCP::Configuration ToNative() {
            ::eCAL::TransportLayer::TCP::Configuration native_config;
            native_config.number_executor_reader = NumberExecutorReader;
            native_config.number_executor_writer = NumberExecutorWriter;
            native_config.max_reconnections = MaxReconnections;
            return native_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::TransportLayer::Configuration structure.
         */
        public ref class TransportLayerConfiguration {
        public:
          property TransportLayerUdpConfiguration^ Udp;
          property TransportLayerTcpConfiguration^ Tcp;

          TransportLayerConfiguration() {
            ::eCAL::TransportLayer::Configuration native_config;
            Udp = gcnew TransportLayerUdpConfiguration(native_config.udp);
            Tcp = gcnew TransportLayerTcpConfiguration(native_config.tcp);
          }

          // Native struct constructor
          TransportLayerConfiguration(const ::eCAL::TransportLayer::Configuration& native_config) {
            Udp = gcnew TransportLayerUdpConfiguration(native_config.udp);
            Tcp = gcnew TransportLayerTcpConfiguration(native_config.tcp);
          }

          ::eCAL::TransportLayer::Configuration ToNative() {
            ::eCAL::TransportLayer::Configuration native_config;
            native_config.udp = Udp->ToNative();
            native_config.tcp = Tcp->ToNative();
            return native_config;
          }
        };

      } // namespace Config
    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse