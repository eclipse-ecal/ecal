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

#include <ecal/types/monitoring.h>
#include "../clr_types.h"
#include "../clr_common.h"

using namespace System::Collections::Generic;
using namespace Eclipse::eCAL::Core;

namespace Eclipse {
  namespace eCAL {
    namespace Core {

      // Managed representation of the native monitoring entities bitmask.
      [System::Flags]
      public enum class MonitoringEntity : unsigned int
      {
        None       = ::eCAL::Monitoring::Entity::None,
        Publisher  = ::eCAL::Monitoring::Entity::Publisher,
        Subscriber = ::eCAL::Monitoring::Entity::Subscriber,
        Server     = ::eCAL::Monitoring::Entity::Server,
        Client     = ::eCAL::Monitoring::Entity::Client,
        Process    = ::eCAL::Monitoring::Entity::Process,
        Host       = ::eCAL::Monitoring::Entity::Host,
        All        = ::eCAL::Monitoring::Entity::All
      };

      /**
       * @brief Managed wrapper for the native eCAL::Monitoring::eTransportLayerType enum.
       */
      public enum class eMonitoringTransportLayerType
      {
        None   = ::eCAL::Monitoring::eTransportLayerType::none,   ///< No transport layer
        UdpMc  = ::eCAL::Monitoring::eTransportLayerType::udp_mc, ///< UDP multicast transport layer
        Shm    = ::eCAL::Monitoring::eTransportLayerType::shm,    ///< Shared memory transport layer
        Tcp    = ::eCAL::Monitoring::eTransportLayerType::tcp     ///< TCP transport layer
      };

      /**
       * @brief Helper class for converting between managed and native eMonitoringTransportLayerType enums.
       */
      public ref class MonitoringTransportLayerTypeHelper abstract sealed
      {
      public:
        static eMonitoringTransportLayerType FromNative(::eCAL::Monitoring::eTransportLayerType native);
        static ::eCAL::Monitoring::eTransportLayerType ToNative(eMonitoringTransportLayerType managed);
      };

      /**
       * @brief Managed wrapper for the native STransportLayer struct.
       *
       * Represents a transport layer used by a topic (e.g. UDP, SHM, TCP).
       */
      public ref class MonitoringTransportLayer sealed
      {
      public:
        property eMonitoringTransportLayerType Type { eMonitoringTransportLayerType get(); }
        property int Version { int get(); }
        property bool Active { bool get(); }

        MonitoringTransportLayer(const ::eCAL::Monitoring::STransportLayer& native);

      private:
        initonly eMonitoringTransportLayerType type_;
        initonly int version_;
        initonly bool active_;
      };

      /**
       * @brief Managed wrapper for the native STopic struct.
       *
       * Represents a monitored topic (publisher or subscriber).
       */
      public ref class MonitoringTopic sealed
      {
      public:
        property int RegistrationClock { int get(); }                           ///< Registration clock (heartbeat)
        property System::String^ HostName { System::String^ get(); }            ///< Host name
        property System::String^ ProcessName { System::String^ get(); }         ///< Process name
        property System::String^ UnitName { System::String^ get(); }            ///< Unit name
        property int ProcessId { int get(); }                                   ///< Process ID
        property System::UInt64 TopicId { System::UInt64 get(); }               ///< Topic ID
        property System::String^ TopicName { System::String^ get(); }           ///< Topic name
        property System::String^ ShmTransportDomain { System::String^ get(); }  ///< SHM transport domain
        property System::String^ Direction { System::String^ get(); }           ///< Direction (publisher, subscriber)
        property DataTypeInformation^ DatatypeInformation { DataTypeInformation^ get(); } ///< Topic datatype information (name, encoding, descriptor)
        property IReadOnlyList<MonitoringTransportLayer^>^ TransportLayer { IReadOnlyList<MonitoringTransportLayer^>^ get(); } ///< Transport layer details
        property int TopicSize { int get(); }                                   ///< Topic size
        property int ConnectionsLocal { int get(); }                            ///< Number of local connected entities
        property int ConnectionsExternal { int get(); }                         ///< Number of external connected entities
        property int MessageDrops { int get(); }                                ///< Dropped messages
        property System::Int64 DataId { System::Int64 get(); }                  ///< Data send ID (publisher setid)
        property System::Int64 DataClock { System::Int64 get(); }               ///< Data clock (send/receive action)
        property int DataFrequency { int get(); }                               ///< Data frequency (send/receive samples per second) [mHz]

        MonitoringTopic(const ::eCAL::Monitoring::STopic& native);

      private:
        initonly int registrationClock_;
        initonly System::String^ hostName_;
        initonly System::String^ processName_;
        initonly System::String^ unitName_;
        initonly int processId_;
        initonly System::UInt64 topicId_;
        initonly System::String^ topicName_;
        initonly System::String^ shmTransportDomain_;
        initonly System::String^ direction_;
        initonly DataTypeInformation^ datatypeInformation_;
        initonly IReadOnlyList<MonitoringTransportLayer^>^ transportLayer_;
        initonly int topicSize_;
        initonly int connectionsLocal_;
        initonly int connectionsExternal_;
        initonly int messageDrops_;
        initonly System::Int64 dataId_;
        initonly System::Int64 dataClock_;
        initonly int dataFrequency_;
      };

      /**
       * @brief Managed wrapper for the native SProcess struct.
       *
       * Represents a monitored process.
       */
      public ref class MonitoringProcess sealed
      {
      public:
        property int RegistrationClock { int get(); }              ///< Registration clock
        property System::String^ HostName { System::String^ get(); }           ///< Host name
        property System::String^ ProcessName { System::String^ get(); }        ///< Process name
        property System::String^ UnitName { System::String^ get(); }           ///< Unit name
        property int ProcessId { int get(); }                      ///< Process ID
        property System::String^ ShmTransportDomain { System::String^ get(); } ///< SHM transport domain
        property System::String^ ProcessParameter { System::String^ get(); }   ///< Process parameter
        property int StateSeverity { int get(); }                  ///< Process state info severity
        property int StateSeverityLevel { int get(); }             ///< Process state info severity level
        property System::String^ StateInfo { System::String^ get(); }          ///< Process state info as human readable string
        property int TimeSyncState { int get(); }                  ///< Time synchronization state
        property System::String^ TimeSyncModuleName { System::String^ get(); } ///< Time synchronization module name
        property int ComponentInitState { int get(); }             ///< eCAL component initialization state
        property System::String^ ComponentInitInfo { System::String^ get(); }  ///< Human readable component init info
        property System::String^ EcalRuntimeVersion { System::String^ get(); } ///< Loaded/runtime eCAL version of a component
        property System::String^ ConfigFilePath { System::String^ get(); }     ///< Filepath of the configuration file that was loaded

        MonitoringProcess(const ::eCAL::Monitoring::SProcess& native);

      private:
        initonly int registrationClock_;
        initonly System::String^ hostName_;
        initonly System::String^ processName_;
        initonly System::String^ unitName_;
        initonly int processId_;
        initonly System::String^ shmTransportDomain_;
        initonly System::String^ processParameter_;
        initonly int stateSeverity_;
        initonly int stateSeverityLevel_;
        initonly System::String^ stateInfo_;
        initonly int timeSyncState_;
        initonly System::String^ timeSyncModuleName_;
        initonly int componentInitState_;
        initonly System::String^ componentInitInfo_;
        initonly System::String^ ecalRuntimeVersion_;
        initonly System::String^ configFilePath_;
      };

      /**
       * @brief Managed wrapper for the native SMethod struct.
       *
       * Represents a monitored RPC method.
       */
      public ref class MonitoringMethod sealed
      {
      public:
        property System::String^ MethodName { System::String^ get(); }                       ///< Method name
        property DataTypeInformation^ RequestDatatypeInformation { DataTypeInformation^ get(); }  ///< Request datatype information
        property DataTypeInformation^ ResponseDatatypeInformation { DataTypeInformation^ get(); } ///< Response datatype information
        property System::Int64 CallCount { System::Int64 get(); }                          ///< Call counter

        MonitoringMethod(const ::eCAL::Monitoring::SMethod& native);

      private:
        initonly System::String^ methodName_;
        initonly DataTypeInformation^ requestDatatypeInformation_;
        initonly DataTypeInformation^ responseDatatypeInformation_;
        initonly System::Int64 callCount_;
      };

      /**
       * @brief Managed wrapper for the native SServer struct.
       *
       * Represents a monitored server.
       */
      public ref class MonitoringServer sealed
      {
      public:
        property int RegistrationClock { int get(); }            ///< Registration clock
        property System::String^ HostName { System::String^ get(); }         ///< Host name
        property System::String^ ProcessName { System::String^ get(); }      ///< Process name
        property System::String^ UnitName { System::String^ get(); }         ///< Unit name
        property int ProcessId { int get(); }                    ///< Process ID
        property System::String^ ServiceName { System::String^ get(); }      ///< Service name
        property System::UInt64 ServiceId { System::UInt64 get(); }         ///< Service ID
        property System::UInt32 Version { System::UInt32 get(); }           ///< Service protocol version
        property System::UInt32 TcpPortV0 { System::UInt32 get(); }         ///< TCP port protocol version 0
        property System::UInt32 TcpPortV1 { System::UInt32 get(); }         ///< TCP port protocol version 1
        property IReadOnlyList<MonitoringMethod^>^ Methods { IReadOnlyList<MonitoringMethod^>^ get(); } ///< List of methods

        MonitoringServer(const ::eCAL::Monitoring::SServer& native);

      private:
        initonly int registrationClock_;
        initonly System::String^ hostName_;
        initonly System::String^ processName_;
        initonly System::String^ unitName_;
        initonly int processId_;
        initonly System::String^ serviceName_;
        initonly System::UInt64 serviceId_;
        initonly System::UInt32 version_;
        initonly System::UInt32 tcpPortV0_;
        initonly System::UInt32 tcpPortV1_;
        initonly IReadOnlyList<MonitoringMethod^>^ methods_;
      };

      /**
       * @brief Managed wrapper for the native SClient struct.
       *
       * Represents a monitored client.
       */
      public ref class MonitoringClient sealed
      {
      public:
        property int RegistrationClock { int get(); }            ///< Registration clock
        property System::String^ HostName { System::String^ get(); }         ///< Host name
        property System::String^ ProcessName { System::String^ get(); }      ///< Process name
        property System::String^ UnitName { System::String^ get(); }         ///< Unit name
        property int ProcessId { int get(); }                    ///< Process ID
        property System::String^ ServiceName { System::String^ get(); }      ///< Service name
        property System::UInt64 ServiceId { System::UInt64 get(); }         ///< Service ID
        property System::UInt32 Version { System::UInt32 get(); }           ///< Client protocol version
        property IReadOnlyList<MonitoringMethod^>^ Methods { IReadOnlyList<MonitoringMethod^>^ get(); } ///< List of methods

        MonitoringClient(const ::eCAL::Monitoring::SClient& native);

      private:
        initonly int registrationClock_;
        initonly System::String^ hostName_;
        initonly System::String^ processName_;
        initonly System::String^ unitName_;
        initonly int processId_;
        initonly System::String^ serviceName_;
        initonly System::UInt64 serviceId_;
        initonly System::UInt32 version_;
        initonly IReadOnlyList<MonitoringMethod^>^ methods_;
      };

      /**
       * @brief Managed wrapper for the native SMonitoring struct.
       *
       * Represents the overall monitoring state (all processes, topics, servers, clients).
       */
      public ref class SMonitoring sealed
      {
      public:
        property IReadOnlyList<MonitoringProcess^>^ Processes { IReadOnlyList<MonitoringProcess^>^ get(); } ///< List of monitored processes
        property IReadOnlyList<MonitoringTopic^>^ Publishers { IReadOnlyList<MonitoringTopic^>^ get(); }    ///< List of monitored publishers
        property IReadOnlyList<MonitoringTopic^>^ Subscribers { IReadOnlyList<MonitoringTopic^>^ get(); }   ///< List of monitored subscribers
        property IReadOnlyList<MonitoringServer^>^ Servers { IReadOnlyList<MonitoringServer^>^ get(); }     ///< List of monitored servers
        property IReadOnlyList<MonitoringClient^>^ Clients { IReadOnlyList<MonitoringClient^>^ get(); }     ///< List of monitored clients

        SMonitoring(const ::eCAL::Monitoring::SMonitoring& native);

      private:
        initonly IReadOnlyList<MonitoringProcess^>^ processes_;
        initonly IReadOnlyList<MonitoringTopic^>^ publishers_;
        initonly IReadOnlyList<MonitoringTopic^>^ subscribers_;
        initonly IReadOnlyList<MonitoringServer^>^ servers_;
        initonly IReadOnlyList<MonitoringClient^>^ clients_;
      };

    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse