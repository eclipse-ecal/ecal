#include "clr_monitoring.h"

using namespace Eclipse::eCAL::Core;

// --- MonitoringTransportLayerTypeHelper ---
eMonitoringTransportLayerType MonitoringTransportLayerTypeHelper::FromNative(::eCAL::Monitoring::eTransportLayerType native)
{
  return static_cast<eMonitoringTransportLayerType>(native);
}
::eCAL::Monitoring::eTransportLayerType MonitoringTransportLayerTypeHelper::ToNative(eMonitoringTransportLayerType managed)
{
  return static_cast<::eCAL::Monitoring::eTransportLayerType>(managed);
}

// --- MonitoringTransportLayer ---
MonitoringTransportLayer::MonitoringTransportLayer(const ::eCAL::Monitoring::STransportLayer& native)
  : type_(MonitoringTransportLayerTypeHelper::FromNative(native.type)),
    version_(native.version),
    active_(native.active)
{}
eMonitoringTransportLayerType MonitoringTransportLayer::Type::get() { return type_; }
int MonitoringTransportLayer::Version::get() { return version_; }
bool MonitoringTransportLayer::Active::get() { return active_; }

// --- MonitoringTopic ---
MonitoringTopic::MonitoringTopic(const ::eCAL::Monitoring::STopic& native)
  : registrationClock_(native.registration_clock),
    hostName_(Internal::StlStringToString(native.host_name)),
    processName_(Internal::StlStringToString(native.process_name)),
    unitName_(Internal::StlStringToString(native.unit_name)),
    processId_(native.process_id),
    topicId_(static_cast<System::UInt64>(native.topic_id)),
    topicName_(Internal::StlStringToString(native.topic_name)),
    shmTransportDomain_(Internal::StlStringToString(native.shm_transport_domain)),
    direction_(Internal::StlStringToString(native.direction)),
    datatypeInformation_(gcnew DataTypeInformation(
      Internal::StlStringToString(native.datatype_information.name),
      Internal::StlStringToString(native.datatype_information.encoding),
      Internal::StlStringToByteArray(native.datatype_information.descriptor))),
    topicSize_(native.topic_size),
    connectionsLocal_(native.connections_local),
    connectionsExternal_(native.connections_external),
    messageDrops_(native.message_drops),
    dataId_(native.data_id),
    dataClock_(native.data_clock),
    dataFrequency_(native.data_frequency)
{
  auto list = gcnew System::Collections::Generic::List<MonitoringTransportLayer^>();
  for (const auto& tl : native.transport_layer)
    list->Add(gcnew MonitoringTransportLayer(tl));
  transportLayer_ = list->AsReadOnly();
}
int MonitoringTopic::RegistrationClock::get() { return registrationClock_; }
System::String^ MonitoringTopic::HostName::get() { return hostName_; }
System::String^ MonitoringTopic::ProcessName::get() { return processName_; }
System::String^ MonitoringTopic::UnitName::get() { return unitName_; }
int MonitoringTopic::ProcessId::get() { return processId_; }
System::UInt64 MonitoringTopic::TopicId::get() { return topicId_; }
System::String^ MonitoringTopic::TopicName::get() { return topicName_; }
System::String^ MonitoringTopic::ShmTransportDomain::get() { return shmTransportDomain_; }
System::String^ MonitoringTopic::Direction::get() { return direction_; }
DataTypeInformation^ MonitoringTopic::DatatypeInformation::get() { return datatypeInformation_; }
IReadOnlyList<MonitoringTransportLayer^>^ MonitoringTopic::TransportLayer::get() { return transportLayer_; }
int MonitoringTopic::TopicSize::get() { return topicSize_; }
int MonitoringTopic::ConnectionsLocal::get() { return connectionsLocal_; }
int MonitoringTopic::ConnectionsExternal::get() { return connectionsExternal_; }
int MonitoringTopic::MessageDrops::get() { return messageDrops_; }
System::Int64 MonitoringTopic::DataId::get() { return dataId_; }
System::Int64 MonitoringTopic::DataClock::get() { return dataClock_; }
int MonitoringTopic::DataFrequency::get() { return dataFrequency_; }

// --- MonitoringProcess ---
MonitoringProcess::MonitoringProcess(const ::eCAL::Monitoring::SProcess& native)
  : registrationClock_(native.registration_clock),
    hostName_(Internal::StlStringToString(native.host_name)),
    processName_(Internal::StlStringToString(native.process_name)),
    unitName_(Internal::StlStringToString(native.unit_name)),
    processId_(native.process_id),
    shmTransportDomain_(Internal::StlStringToString(native.shm_transport_domain)),
    processParameter_(Internal::StlStringToString(native.process_parameter)),
    stateSeverity_(native.state_severity),
    stateSeverityLevel_(native.state_severity_level),
    stateInfo_(Internal::StlStringToString(native.state_info)),
    timeSyncState_(native.time_sync_state),
    timeSyncModuleName_(Internal::StlStringToString(native.time_sync_module_name)),
    componentInitState_(native.component_init_state),
    componentInitInfo_(Internal::StlStringToString(native.component_init_info)),
    ecalRuntimeVersion_(Internal::StlStringToString(native.ecal_runtime_version)),
    configFilePath_(Internal::StlStringToString(native.config_file_path))
{}
int MonitoringProcess::RegistrationClock::get() { return registrationClock_; }
System::String^ MonitoringProcess::HostName::get() { return hostName_; }
System::String^ MonitoringProcess::ProcessName::get() { return processName_; }
System::String^ MonitoringProcess::UnitName::get() { return unitName_; }
int MonitoringProcess::ProcessId::get() { return processId_; }
System::String^ MonitoringProcess::ShmTransportDomain::get() { return shmTransportDomain_; }
System::String^ MonitoringProcess::ProcessParameter::get() { return processParameter_; }
int MonitoringProcess::StateSeverity::get() { return stateSeverity_; }
int MonitoringProcess::StateSeverityLevel::get() { return stateSeverityLevel_; }
System::String^ MonitoringProcess::StateInfo::get() { return stateInfo_; }
int MonitoringProcess::TimeSyncState::get() { return timeSyncState_; }
System::String^ MonitoringProcess::TimeSyncModuleName::get() { return timeSyncModuleName_; }
int MonitoringProcess::ComponentInitState::get() { return componentInitState_; }
System::String^ MonitoringProcess::ComponentInitInfo::get() { return componentInitInfo_; }
System::String^ MonitoringProcess::EcalRuntimeVersion::get() { return ecalRuntimeVersion_; }
System::String^ MonitoringProcess::ConfigFilePath::get() { return configFilePath_; }

// --- MonitoringMethod ---
MonitoringMethod::MonitoringMethod(const ::eCAL::Monitoring::SMethod& native)
  : methodName_(Internal::StlStringToString(native.method_name)),
    requestDatatypeInformation_(gcnew DataTypeInformation(
      Internal::StlStringToString(native.request_datatype_information.name),
      Internal::StlStringToString(native.request_datatype_information.encoding),
      Internal::StlStringToByteArray(native.request_datatype_information.descriptor))),
    responseDatatypeInformation_(gcnew DataTypeInformation(
      Internal::StlStringToString(native.response_datatype_information.name),
      Internal::StlStringToString(native.response_datatype_information.encoding),
      Internal::StlStringToByteArray(native.response_datatype_information.descriptor))),
    callCount_(native.call_count)
{}
System::String^ MonitoringMethod::MethodName::get() { return methodName_; }
DataTypeInformation^ MonitoringMethod::RequestDatatypeInformation::get() { return requestDatatypeInformation_; }
DataTypeInformation^ MonitoringMethod::ResponseDatatypeInformation::get() { return responseDatatypeInformation_; }
System::Int64 MonitoringMethod::CallCount::get() { return callCount_; }

// --- MonitoringServer ---
MonitoringServer::MonitoringServer(const ::eCAL::Monitoring::SServer& native)
  : registrationClock_(native.registration_clock),
    hostName_(Internal::StlStringToString(native.host_name)),
    processName_(Internal::StlStringToString(native.process_name)),
    unitName_(Internal::StlStringToString(native.unit_name)),
    processId_(native.process_id),
    serviceName_(Internal::StlStringToString(native.service_name)),
    serviceId_(static_cast<System::UInt64>(native.service_id)),
    version_(native.version),
    tcpPortV0_(native.tcp_port_v0),
    tcpPortV1_(native.tcp_port_v1)
{
  auto list = gcnew System::Collections::Generic::List<MonitoringMethod^>();
  for (const auto& m : native.methods)
    list->Add(gcnew MonitoringMethod(m));
  methods_ = list->AsReadOnly();
}
int MonitoringServer::RegistrationClock::get() { return registrationClock_; }
System::String^ MonitoringServer::HostName::get() { return hostName_; }
System::String^ MonitoringServer::ProcessName::get() { return processName_; }
System::String^ MonitoringServer::UnitName::get() { return unitName_; }
int MonitoringServer::ProcessId::get() { return processId_; }
System::String^ MonitoringServer::ServiceName::get() { return serviceName_; }
System::UInt64 MonitoringServer::ServiceId::get() { return serviceId_; }
System::UInt32 MonitoringServer::Version::get() { return version_; }
System::UInt32 MonitoringServer::TcpPortV0::get() { return tcpPortV0_; }
System::UInt32 MonitoringServer::TcpPortV1::get() { return tcpPortV1_; }
IReadOnlyList<MonitoringMethod^>^ MonitoringServer::Methods::get() { return methods_; }

// --- MonitoringClient ---
MonitoringClient::MonitoringClient(const ::eCAL::Monitoring::SClient& native)
  : registrationClock_(native.registration_clock),
    hostName_(Internal::StlStringToString(native.host_name)),
    processName_(Internal::StlStringToString(native.process_name)),
    unitName_(Internal::StlStringToString(native.unit_name)),
    processId_(native.process_id),
    serviceName_(Internal::StlStringToString(native.service_name)),
    serviceId_(static_cast<System::UInt64>(native.service_id)),
    version_(native.version)
{
  auto list = gcnew System::Collections::Generic::List<MonitoringMethod^>();
  for (const auto& m : native.methods)
    list->Add(gcnew MonitoringMethod(m));
  methods_ = list->AsReadOnly();
}
int MonitoringClient::RegistrationClock::get() { return registrationClock_; }
System::String^ MonitoringClient::HostName::get() { return hostName_; }
System::String^ MonitoringClient::ProcessName::get() { return processName_; }
System::String^ MonitoringClient::UnitName::get() { return unitName_; }
int MonitoringClient::ProcessId::get() { return processId_; }
System::String^ MonitoringClient::ServiceName::get() { return serviceName_; }
System::UInt64 MonitoringClient::ServiceId::get() { return serviceId_; }
System::UInt32 MonitoringClient::Version::get() { return version_; }
IReadOnlyList<MonitoringMethod^>^ MonitoringClient::Methods::get() { return methods_; }

// --- SMonitoring ---
SMonitoring::SMonitoring(const ::eCAL::Monitoring::SMonitoring& native)
{
  {
    auto list = gcnew System::Collections::Generic::List<MonitoringProcess^>();
    for (const auto& p : native.processes)
      list->Add(gcnew MonitoringProcess(p));
    processes_ = list->AsReadOnly();
  }
  {
    auto list = gcnew System::Collections::Generic::List<MonitoringTopic^>();
    for (const auto& t : native.publishers)
      list->Add(gcnew MonitoringTopic(t));
    publishers_ = list->AsReadOnly();
  }
  {
    auto list = gcnew System::Collections::Generic::List<MonitoringTopic^>();
    for (const auto& t : native.subscribers)
      list->Add(gcnew MonitoringTopic(t));
    subscribers_ = list->AsReadOnly();
  }
  {
    auto list = gcnew System::Collections::Generic::List<MonitoringServer^>();
    for (const auto& s : native.servers)
      list->Add(gcnew MonitoringServer(s));
    servers_ = list->AsReadOnly();
  }
  {
    auto list = gcnew System::Collections::Generic::List<MonitoringClient^>();
    for (const auto& c : native.clients)
      list->Add(gcnew MonitoringClient(c));
    clients_ = list->AsReadOnly();
  }
}
IReadOnlyList<MonitoringProcess^>^ SMonitoring::Processes::get() { return processes_; }
IReadOnlyList<MonitoringTopic^>^ SMonitoring::Publishers::get() { return publishers_; }
IReadOnlyList<MonitoringTopic^>^ SMonitoring::Subscribers::get() { return subscribers_; }
IReadOnlyList<MonitoringServer^>^ SMonitoring::Servers::get() { return servers_; }
IReadOnlyList<MonitoringClient^>^ SMonitoring::Clients::get() { return clients_; }