/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

/**
 * @brief  Add ecal_util functions to nanobind module
**/

#include <modules/module_core.h>
#include <ecal/ecal_util.h>

void AddUtilFuncToModule(nanobind::module_& module)
{
    // Functions from ecal_util.h
    module.def("enable_loopback", [](bool nb_state_) { return eCAL::Util::EnableLoopback(nb_state_); });
    module.def("pub_share_type", [](bool nb_state_) { return eCAL::Util::PubShareType(nb_state_); });
    module.def("pub_share_description", [](bool nb_state_) { return eCAL::Util::PubShareDescription(nb_state_); });

#if ECAL_CORE_MONITORING
    module.def("shutdown_core", []() { return eCAL::Util::ShutdownCore(); });
    module.def("shutdown_processes", []() { return eCAL::Util::ShutdownProcesses(); });
    module.def("shutdown_process", [](const std::string& nb_process_name_) { return eCAL::Util::ShutdownProcess(nb_process_name_); });
    module.def("shutdown_process", [](const int nb_process_id_) { return eCAL::Util::ShutdownProcess(nb_process_id_); });
#endif // ECAL_CORE_MONITORING

    module.def("get_user_settings_path", []() { return eCAL::Util::GeteCALUserSettingsPath(); });
    module.def("get_log_path", []() { return eCAL::Util::GeteCALLogPath(); });
    module.def("get_config_path", []() { return eCAL::Util::GeteCALConfigPath(); });
    module.def("get_active_ini_file", []() { return eCAL::Util::GeteCALActiveIniFile(); });

    // TODO Ariff: function changed
    //module.def("get_topics", [](std::unordered_map<std::string, eCAL::SDataTypeInformation>& nb_topic_info_map_)
    //    { return eCAL::Util::GetTopics(nb_topic_info_map_); });

    // TODO Ariff: function changed
    //module.def("get_topic_names", [](std::vector<std::string>& nb_topic_names_)
    //    { return eCAL::Util::GetTopicNames(nb_topic_names_); });
    module.def("get_topic_datatype_info", [](const std::string& nb_topic_name_)
        {
            eCAL::SDataTypeInformation nb_topic_info_;
            auto success = eCAL::Util::GetTopicDataTypeInformation(nb_topic_name_, nb_topic_info_);
            auto return_value = nanobind::make_tuple(success, nb_topic_info_);
            return return_value;
        });

    //    m.def("getservices", [](std::map<std::tuple<std::string, std::string>, eCAL::SServiceMethodInformation>& nb_service_info_map_)
    //        { return eCAL::Util::GetServices(nb_service_info_map_); });

    // TODO Ariff: function changed
    //module.def("get_service_names", [](std::vector<std::tuple<std::string, std::string>>& nb_service_method_names_)
    //    { return eCAL::Util::GetServiceNames(nb_service_method_names_); });
    module.def("get_service_type_names", [](const std::string& nb_service_name_, const std::string& nb_method_name_, std::string& nb_req_type_, std::string& nb_resp_type_)
        { return eCAL::Util::GetServiceTypeNames(nb_service_name_, nb_method_name_, nb_req_type_, nb_resp_type_); });
    module.def("get_service_description", [](const std::string& nb_service_name_, const std::string& nb_method_name_, std::string& nb_req_desc_, std::string& nb_resp_desc_)
        { return eCAL::Util::GetServiceDescription(nb_service_name_, nb_method_name_, nb_req_desc_, nb_resp_desc_); });

    module.def("split_combined_topic_type", [](const std::string& nb_combined_topic_type_)
        { return eCAL::Util::SplitCombinedTopicType(nb_combined_topic_type_); });
    module.def("combined_topic_encoding_and_type", [](const std::string& nb_topic_encoding_, const std::string& nb_topic_type_)
        { return eCAL::Util::CombinedTopicEncodingAndType(nb_topic_encoding_, nb_topic_type_); });
}
