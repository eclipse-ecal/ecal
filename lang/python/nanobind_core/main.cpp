#include <nanobind/nanobind.h>
#include <ecal/ecal.h>
#include <nanobind/operators.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/tuple.h>
#include <cstdint>
#include <chrono>
#include <memory>
#include <string>
#include <cstddef>
#include <ecal/ecal_types.h>
#include <ecal/ecal_nb_subscriber.h>


NB_MODULE(nanobind_core, m) {
    // Struct eCAL::SDataTypeInformation
    nanobind::class_<eCAL::SDataTypeInformation>(m, "DataTypeInformation")
        .def(nanobind::init<>())
        .def_rw("name", &eCAL::SDataTypeInformation::name)
        .def_rw("encoding", &eCAL::SDataTypeInformation::encoding)
        .def_rw("descriptor", &eCAL::SDataTypeInformation::descriptor);

    auto NBSubscriber_cls = nanobind::class_<eCAL::CNBSubscriber>(m, "NBSubscriber")
        .def(nanobind::init<>())
        .def(nanobind::init<const std::string&>())
        .def(nanobind::init<const std::string&, const eCAL::SDataTypeInformation&>())
        .def("receive", &eCAL::CNBSubscriber::Receive)
        .def("create", nanobind::overload_cast<const std::string&>(&eCAL::CSubscriber::Create))
        .def("create", nanobind::overload_cast<const std::string&, const eCAL::SDataTypeInformation&>(&eCAL::CSubscriber::Create))
        .def("destroy", &eCAL::CSubscriber::Destroy)
        .def("set_attribute", &eCAL::CSubscriber::SetAttribute)
        .def("clear_attribute", &eCAL::CSubscriber::ClearAttribute)
        .def("set_id", &eCAL::CSubscriber::SetID)
        .def("receive_buffer", &eCAL::CSubscriber::ReceiveBuffer)
        .def("add_receive_callback", &eCAL::CSubscriber::AddReceiveCallback)
        .def("rem_receive_callback", &eCAL::CSubscriber::RemReceiveCallback)
        .def("rem_event_callback", &eCAL::CSubscriber::RemEventCallback)
        .def("add_event_callback", &eCAL::CSubscriber::AddEventCallback)
        .def("is_created", &eCAL::CSubscriber::IsCreated)
        .def("get_publisher_count", &eCAL::CSubscriber::GetPublisherCount)
        .def("get_topic_name", &eCAL::CSubscriber::GetTopicName)
        .def("get_datatype_information", &eCAL::CSubscriber::GetDataTypeInformation)
        .def("dump", &eCAL::CSubscriber::Dump);

    // Class and Functions from ecal_publisher.h
    auto Publisher_cls = nanobind::class_<eCAL::CPublisher>(m, "Publisher")
        .def(nanobind::init<>())
        .def(nanobind::init<const std::string&>())
        .def(nanobind::init<const std::string&, const eCAL::SDataTypeInformation&>())
        //        .def(nanobind::self = nanobind::self)
        .def("create", nanobind::overload_cast<const std::string&>(&eCAL::CPublisher::Create))
        .def("create", nanobind::overload_cast<const std::string&, const eCAL::SDataTypeInformation&>(&eCAL::CPublisher::Create))
        .def("send", nanobind::overload_cast<const void*, size_t, long long>(&eCAL::CPublisher::Send))
        .def("send", nanobind::overload_cast<eCAL::CPayloadWriter&, long long>(&eCAL::CPublisher::Send))
        .def("send", nanobind::overload_cast<const std::string&, long long>(&eCAL::CPublisher::Send))
        .def("destroy", &eCAL::CPublisher::Destroy)
        .def("set_attribute", &eCAL::CPublisher::SetAttribute)
        .def("clear_attribute", &eCAL::CPublisher::ClearAttribute)
        .def("set_id", &eCAL::CPublisher::SetID)
        .def("share_type", &eCAL::CPublisher::ShareType)
        .def("share_description", &eCAL::CPublisher::ShareDescription)
        .def("rem_event_callback", &eCAL::CPublisher::RemEventCallback)
        .def("add_event_callback", &eCAL::CPublisher::AddEventCallback)
        .def("is_created", &eCAL::CPublisher::IsCreated)
        .def("is_subscribed", &eCAL::CPublisher::IsSubscribed)
        .def("get_subscriber_count", &eCAL::CPublisher::GetSubscriberCount)
        .def("get_topic_name", &eCAL::CPublisher::GetTopicName)
        .def("get_datatype_information", &eCAL::CPublisher::GetDataTypeInformation)
        .def("dump", &eCAL::CPublisher::Dump);

    // Class and Functions from ecal_client.h
    auto ServiceClient_cls = nanobind::class_<eCAL::CServiceClient>(m, "ServiceClient")
        .def(nanobind::init<>())
        .def(nanobind::init<const std::string&>())
        //        .def(nanobind::self = nanobind::self)
        .def("create", &eCAL::CServiceClient::Create)
        .def("set_hostname", &eCAL::CServiceClient::SetHostName)
        .def("destroy", &eCAL::CServiceClient::Destroy)
        .def("call", nanobind::overload_cast<const std::string&, const std::string&, int>(&eCAL::CServiceClient::Call))
        .def("call", nanobind::overload_cast<const std::string&, const std::string&, int, eCAL::ServiceResponseVecT*>(&eCAL::CServiceClient::Call))
        .def("call_async", &eCAL::CServiceClient::CallAsync)
        .def("add_response_callback", &eCAL::CServiceClient::AddResponseCallback)
        .def("rem_response_callback", &eCAL::CServiceClient::RemResponseCallback)
        .def("rem_event_callback", &eCAL::CServiceClient::RemEventCallback)
        .def("add_event_callback", &eCAL::CServiceClient::AddEventCallback)
        .def("is_connected", &eCAL::CServiceClient::IsConnected)
        .def("get_service_name", &eCAL::CServiceClient::GetServiceName);

    // Class and Functions from ecal_server.h
    auto ServiceServer_cls = nanobind::class_<eCAL::CServiceServer>(m, "ServiceServer")
        .def(nanobind::init<>())
        .def(nanobind::init<const std::string&>())
        //        .def(nanobind::self = nanobind::self)
        .def("create", &eCAL::CServiceServer::Create)
        .def("add_description", &eCAL::CServiceServer::AddDescription)
        .def("destroy", &eCAL::CServiceServer::Destroy)
        .def("add_method_callback", &eCAL::CServiceServer::AddMethodCallback)
        .def("rem_method_callback", &eCAL::CServiceServer::RemMethodCallback)
        .def("rem_event_callback", &eCAL::CServiceServer::RemEventCallback)
        .def("add_event_callback", &eCAL::CServiceServer::AddEventCallback)
        .def("is_connected", &eCAL::CServiceServer::IsConnected)
        .def("get_service_name", &eCAL::CServiceServer::GetServiceName);

    // Functions from ecal_core.h
    m.def("get_version_string", []() { return eCAL::GetVersionString(); });
    m.def("get_version_date", []() { return eCAL::GetVersionDateString(); });
    m.def("set_unitname", [](const std::string& nb_unit_name) { return eCAL::SetUnitName(nb_unit_name.c_str()); });
    m.def("is_initialised", []() { return eCAL::IsInitialized(); });
    m.def("finalize", [](unsigned int nb_component_) { return eCAL::Finalize(nb_component_); });
    m.def("ok", []() { return eCAL::Ok(); });

    m.def("get_version", [](int* nb_major, int* nb_minor, int* nb_patch)
        { return eCAL::GetVersion(nb_major, nb_minor, nb_patch); });
    //   m.def("initialize", [](int nb_argc_, char *nb_argv_, const char* nb_unit_name_, unsigned int nb_components_)
    //       { return eCAL::Initialize(nb_argc_, nb_argv_, nb_unit_name_, nb_components_); });
    m.def("initialize", [](std::vector<std::string> nb_args_, std::string nb_unit_name_)
        { return eCAL::Initialize(nb_args_, nb_unit_name_.c_str(), eCAL::Init::Default); });
    m.def("initialize", []()
        { return eCAL::Initialize(); });


    // Functions from ecal_util.h
    m.def("enable_loopback", [](bool nb_state_) { return eCAL::Util::EnableLoopback(nb_state_); });
    m.def("pub_share_type", [](bool nb_state_) { return eCAL::Util::PubShareType(nb_state_); });
    m.def("pub_share_description", [](bool nb_state_) { return eCAL::Util::PubShareDescription(nb_state_); });

#if ECAL_CORE_MONITORING
    m.def("shutdown_core", []() { return eCAL::Util::ShutdownCore(); });
    m.def("shutdown_processes", []() { return eCAL::Util::ShutdownProcesses(); });
    m.def("shutdown_process", [](const std::string& nb_process_name_) { return eCAL::Util::ShutdownProcess(nb_process_name_); });
    m.def("shutdown_process", [](const int nb_process_id_) { return eCAL::Util::ShutdownProcess(nb_process_id_); });
#endif // ECAL_CORE_MONITORING

    m.def("get_user_settings_path", []() { return eCAL::Util::GeteCALUserSettingsPath(); });
    m.def("get_log_path", []() { return eCAL::Util::GeteCALLogPath(); });
    m.def("get_config_path", []() { return eCAL::Util::GeteCALConfigPath(); });
    m.def("get_active_ini_file", []() { return eCAL::Util::GeteCALActiveIniFile(); });

    m.def("get_topics", [](std::unordered_map<std::string, eCAL::SDataTypeInformation>& nb_topic_info_map_)
        { return eCAL::Util::GetTopics(nb_topic_info_map_); });

    m.def("get_topic_names", [](std::vector<std::string>& nb_topic_names_)
        { return eCAL::Util::GetTopicNames(nb_topic_names_); });
    m.def("get_topic_datatype_info", [](const std::string& nb_topic_name_)
        {
            eCAL::SDataTypeInformation nb_topic_info_;
            auto success = eCAL::Util::GetTopicDataTypeInformation(nb_topic_name_, nb_topic_info_);
            auto return_value = nanobind::make_tuple(success, nb_topic_info_);
            return return_value;
        });

    //    m.def("getservices", [](std::map<std::tuple<std::string, std::string>, eCAL::SServiceMethodInformation>& nb_service_info_map_)
    //        { return eCAL::Util::GetServices(nb_service_info_map_); });
    m.def("get_service_names", [](std::vector<std::tuple<std::string, std::string>>& nb_service_method_names_)
        { return eCAL::Util::GetServiceNames(nb_service_method_names_); });
    m.def("get_service_type_names", [](const std::string& nb_service_name_, const std::string& nb_method_name_, std::string& nb_req_type_, std::string& nb_resp_type_)
        { return eCAL::Util::GetServiceTypeNames(nb_service_name_, nb_method_name_, nb_req_type_, nb_resp_type_); });
    m.def("get_service_description", [](const std::string& nb_service_name_, const std::string& nb_method_name_, std::string& nb_req_desc_, std::string& nb_resp_desc_)
        { return eCAL::Util::GetServiceDescription(nb_service_name_, nb_method_name_, nb_req_desc_, nb_resp_desc_); });

    m.def("split_combined_topic_type", [](const std::string& nb_combined_topic_type_)
        { return eCAL::Util::SplitCombinedTopicType(nb_combined_topic_type_); });
    m.def("combined_topic_encoding_and_type", [](const std::string& nb_topic_encoding_, const std::string& nb_topic_type_)
        { return eCAL::Util::CombinedTopicEncodingAndType(nb_topic_encoding_, nb_topic_type_); });
}