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
        static eMonitoringTransportLayerType FromNative(::eCAL::Monitoring::eTransportLayerType native)
        {
          return static_cast<eMonitoringTransportLayerType>(native);
        }

        static ::eCAL::Monitoring::eTransportLayerType ToNative(eMonitoringTransportLayerType managed)
        {
          return static_cast<::eCAL::Monitoring::eTransportLayerType>(managed);
        }
      };

      /**
       * @brief Managed wrapper for the native STransportLayer struct.
       *
       * Represents a transport layer used by a topic (e.g. UDP, SHM, TCP).
       */
      public ref class MonitoringTransportLayer
      {
      public:
        property eMonitoringTransportLayerType Type;
        property int Version;
        property bool Active;

        MonitoringTransportLayer() {}

        MonitoringTransportLayer(const ::eCAL::Monitoring::STransportLayer& native)
        {
          Type = MonitoringTransportLayerTypeHelper::FromNative(native.type);
          Version = native.version;
          Active = native.active;
        }
      };

      /**
       * @brief Managed wrapper for the native STopic struct.
       *
       * Represents a monitored topic (publisher or subscriber).
       */
      public ref class MonitoringTopic
      {
      public:
        property int RegistrationClock;                           ///< Registration clock (heartbeat)
        property System::String^ HostName;                        ///< Host name
        property System::String^ ShmTransportDomain;              ///< SHM transport domain
        property int ProcessId;                                   ///< Process ID
        property System::String^ ProcessName;                     ///< Process name
        property System::String^ UnitName;                        ///< Unit name
        property System::UInt64 TopicId;                          ///< Topic ID
        property System::String^ TopicName;                       ///< Topic name
        property System::String^ Direction;                       ///< Direction (publisher, subscriber)
        property DataTypeInformation^ DatatypeInformation;        ///< Topic datatype information (name, encoding, descriptor)
        property List<MonitoringTransportLayer^>^ TransportLayer; ///< Transport layer details
        property int TopicSize;                                   ///< Topic size
        property int ConnectionsLocal;                            ///< Number of local connected entities
        property int ConnectionsExternal;                         ///< Number of external connected entities
        property int MessageDrops;                                ///< Dropped messages
        property System::Int64 DataId;                            ///< Data send ID (publisher setid)
        property System::Int64 DataClock;                         ///< Data clock (send/receive action)
        property int DataFrequency;                               ///< Data frequency (send/receive samples per second) [mHz]

        MonitoringTopic()
        {
          TransportLayer = gcnew List<MonitoringTransportLayer^>();
        }

        MonitoringTopic(const ::eCAL::Monitoring::STopic& native)
        {
          RegistrationClock = native.registration_clock;
          HostName = Internal::StlStringToString(native.host_name);
          ShmTransportDomain = Internal::StlStringToString(native.shm_transport_domain);
          ProcessId = native.process_id;
          ProcessName = Internal::StlStringToString(native.process_name);
          UnitName = Internal::StlStringToString(native.unit_name);
          TopicId = static_cast<System::UInt64>(native.topic_id);
          TopicName = Internal::StlStringToString(native.topic_name);
          Direction = Internal::StlStringToString(native.direction);
          DatatypeInformation = gcnew DataTypeInformation(
            Internal::StlStringToString(native.datatype_information.name),
            Internal::StlStringToString(native.datatype_information.encoding),
            Internal::StlStringToByteArray(native.datatype_information.descriptor)
          );
          TransportLayer = gcnew List<MonitoringTransportLayer^>();
          for (const auto& tl : native.transport_layer)
            TransportLayer->Add(gcnew MonitoringTransportLayer(tl));
          TopicSize = native.topic_size;
          ConnectionsLocal = native.connections_local;
          ConnectionsExternal = native.connections_external;
          MessageDrops = native.message_drops;
          DataId = native.data_id;
          DataClock = native.data_clock;
          DataFrequency = native.data_frequency;
        }
      };

      /**
       * @brief Managed wrapper for the native SProcess struct.
       *
       * Represents a monitored process.
       */
      public ref class MonitoringProcess
      {
      public:
        property int RegistrationClock;              ///< Registration clock
        property System::String^ HostName;           ///< Host name
        property System::String^ ShmTransportDomain; ///< SHM transport domain
        property int ProcessId;                      ///< Process ID
        property System::String^ ProcessName;        ///< Process name
        property System::String^ UnitName;           ///< Unit name
        property System::String^ ProcessParameter;   ///< Process parameter
        property int StateSeverity;                  ///< Process state info severity
        property int StateSeverityLevel;             ///< Process state info severity level
        property System::String^ StateInfo;          ///< Process state info as human readable string
        property int TimeSyncState;                  ///< Time synchronization state
        property System::String^ TimeSyncModuleName; ///< Time synchronization module name
        property int ComponentInitState;             ///< eCAL component initialization state
        property System::String^ ComponentInitInfo;  ///< Human readable component init info
        property System::String^ EcalRuntimeVersion; ///< Loaded/runtime eCAL version of a component
        property System::String^ ConfigFilePath;     ///< Filepath of the configuration file that was loaded

        MonitoringProcess() {}

        MonitoringProcess(const ::eCAL::Monitoring::SProcess& native)
        {
          RegistrationClock = native.registration_clock;
          HostName = Internal::StlStringToString(native.host_name);
          ShmTransportDomain = Internal::StlStringToString(native.shm_transport_domain);
          ProcessId = native.process_id;
          ProcessName = Internal::StlStringToString(native.process_name);
          UnitName = Internal::StlStringToString(native.unit_name);
          ProcessParameter = Internal::StlStringToString(native.process_parameter);
          StateSeverity = native.state_severity;
          StateSeverityLevel = native.state_severity_level;
          StateInfo = Internal::StlStringToString(native.state_info);
          TimeSyncState = native.time_sync_state;
          TimeSyncModuleName = Internal::StlStringToString(native.time_sync_module_name);
          ComponentInitState = native.component_init_state;
          ComponentInitInfo = Internal::StlStringToString(native.component_init_info);
          EcalRuntimeVersion = Internal::StlStringToString(native.ecal_runtime_version);
          ConfigFilePath = Internal::StlStringToString(native.config_file_path);
        }
      };

      /**
       * @brief Managed wrapper for the native SMethod struct.
       *
       * Represents a monitored RPC method.
       */
      public ref class MonitoringMethod
      {
      public:
        property System::String^ MethodName;                       ///< Method name
        property DataTypeInformation^ RequestDatatypeInformation;  ///< Request datatype information
        property DataTypeInformation^ ResponseDatatypeInformation; ///< Response datatype information
        property System::Int64 CallCount;                          ///< Call counter

        MonitoringMethod() {}

        MonitoringMethod(const ::eCAL::Monitoring::SMethod& native)
        {
          MethodName = Internal::StlStringToString(native.method_name);
          RequestDatatypeInformation = gcnew DataTypeInformation(
            Internal::StlStringToString(native.request_datatype_information.name),
            Internal::StlStringToString(native.request_datatype_information.encoding),
            Internal::StlStringToByteArray(native.request_datatype_information.descriptor)
          );
          ResponseDatatypeInformation = gcnew DataTypeInformation(
            Internal::StlStringToString(native.response_datatype_information.name),
            Internal::StlStringToString(native.response_datatype_information.encoding),
            Internal::StlStringToByteArray(native.response_datatype_information.descriptor)
          );
          CallCount = native.call_count;
        }
      };

      /**
       * @brief Managed wrapper for the native SServer struct.
       *
       * Represents a monitored server.
       */
      public ref class MonitoringServer
      {
      public:
        property int RegistrationClock;            ///< Registration clock
        property System::String^ HostName;         ///< Host name
        property System::String^ ProcessName;      ///< Process name
        property System::String^ UnitName;         ///< Unit name
        property int ProcessId;                    ///< Process ID
        property System::String^ ServiceName;      ///< Service name
        property System::UInt64 ServiceId;         ///< Service ID
        property System::UInt32 Version;           ///< Service protocol version
        property System::UInt32 TcpPortV0;         ///< TCP port protocol version 0
        property System::UInt32 TcpPortV1;         ///< TCP port protocol version 1
        property List<MonitoringMethod^>^ Methods; ///< List of methods

        MonitoringServer()
        {
          Methods = gcnew List<MonitoringMethod^>();
        }

        MonitoringServer(const ::eCAL::Monitoring::SServer& native)
        {
          RegistrationClock = native.registration_clock;
          HostName = Internal::StlStringToString(native.host_name);
          ProcessName = Internal::StlStringToString(native.process_name);
          UnitName = Internal::StlStringToString(native.unit_name);
          ProcessId = native.process_id;
          ServiceName = Internal::StlStringToString(native.service_name);
          ServiceId = static_cast<System::UInt64>(native.service_id);
          Version = native.version;
          TcpPortV0 = native.tcp_port_v0;
          TcpPortV1 = native.tcp_port_v1;
          Methods = gcnew List<MonitoringMethod^>();
          for (const auto& m : native.methods)
            Methods->Add(gcnew MonitoringMethod(m));
        }
      };

      /**
       * @brief Managed wrapper for the native SClient struct.
       *
       * Represents a monitored client.
       */
      public ref class MonitoringClient
      {
      public:
        property int RegistrationClock;            ///< Registration clock
        property System::String^ HostName;         ///< Host name
        property System::String^ ProcessName;      ///< Process name
        property System::String^ UnitName;         ///< Unit name
        property int ProcessId;                    ///< Process ID
        property System::String^ ServiceName;      ///< Service name
        property System::UInt64 ServiceId;         ///< Service ID
        property List<MonitoringMethod^>^ Methods; ///< List of methods
        property System::UInt32 Version;           ///< Client protocol version

        MonitoringClient()
        {
          Methods = gcnew List<MonitoringMethod^>();
        }

        MonitoringClient(const ::eCAL::Monitoring::SClient& native)
        {
          RegistrationClock = native.registration_clock;
          HostName = Internal::StlStringToString(native.host_name);
          ProcessName = Internal::StlStringToString(native.process_name);
          UnitName = Internal::StlStringToString(native.unit_name);
          ProcessId = native.process_id;
          ServiceName = Internal::StlStringToString(native.service_name);
          ServiceId = static_cast<System::UInt64>(native.service_id);
          Methods = gcnew List<MonitoringMethod^>();
          for (const auto& m : native.methods)
            Methods->Add(gcnew MonitoringMethod(m));
          Version = native.version;
        }
      };

      /**
       * @brief Managed wrapper for the native SMonitoring struct.
       *
       * Represents the overall monitoring state (all processes, topics, servers, clients).
       */
      public ref class SMonitoring
      {
      public:
        property List<MonitoringProcess^>^ Processes; ///< List of monitored processes
        property List<MonitoringTopic^>^ Publishers;  ///< List of monitored publishers
        property List<MonitoringTopic^>^ Subscribers; ///< List of monitored subscribers
        property List<MonitoringServer^>^ Servers;    ///< List of monitored servers
        property List<MonitoringClient^>^ Clients;    ///< List of monitored clients

        SMonitoring()
        {
          Processes = gcnew List<MonitoringProcess^>();
          Publishers = gcnew List<MonitoringTopic^>();
          Subscribers = gcnew List<MonitoringTopic^>();
          Servers = gcnew List<MonitoringServer^>();
          Clients = gcnew List<MonitoringClient^>();
        }

        SMonitoring(const ::eCAL::Monitoring::SMonitoring& native)
        {
          Processes = gcnew List<MonitoringProcess^>();
          for (const auto& p : native.processes)
            Processes->Add(gcnew MonitoringProcess(p));
          Publishers = gcnew List<MonitoringTopic^>();
          for (const auto& t : native.publishers)
            Publishers->Add(gcnew MonitoringTopic(t));
          Subscribers = gcnew List<MonitoringTopic^>();
          for (const auto& t : native.subscribers)
            Subscribers->Add(gcnew MonitoringTopic(t));
          Servers = gcnew List<MonitoringServer^>();
          for (const auto& s : native.servers)
            Servers->Add(gcnew MonitoringServer(s));
          Clients = gcnew List<MonitoringClient^>();
          for (const auto& c : native.clients)
            Clients->Add(gcnew MonitoringClient(c));
        }
      };

    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse