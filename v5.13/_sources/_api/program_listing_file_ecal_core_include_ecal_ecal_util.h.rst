
.. _program_listing_file_ecal_core_include_ecal_ecal_util.h:

Program Listing for File ecal_util.h
====================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_util.h>` (``ecal/core/include/ecal/ecal_util.h``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   /* ========================= eCAL LICENSE =================================
    *
    * Copyright (C) 2016 - 2019 Continental Corporation
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
   
   #include <ecal/ecal_os.h>
   #include <ecal/ecal_deprecate.h>
   #include <ecal/ecal_types.h>
   
   #include <map>
   #include <string>
   #include <tuple>
   #include <unordered_map>
   #include <utility>
   #include <vector>
   
   namespace eCAL
   {
     namespace Util
     {
       ECAL_DEPRECATE_SINCE_5_10("Please use the method std::string GeteCALConfigPath() instead. This function will be removed in future eCAL versions.")
       ECAL_API std::string GeteCALHomePath();
   
       ECAL_API std::string GeteCALConfigPath();
   
       ECAL_API std::string GeteCALUserSettingsPath();
   
       ECAL_API std::string GeteCALLogPath();
   
       ECAL_API std::string GeteCALActiveIniFile();
   
       ECAL_DEPRECATE_SINCE_5_10("Please use the method std::string GeteCALActiveIniFile() instead. This function will be removed in future eCAL versions.")
       ECAL_API std::string GeteCALDefaultIniFile();
   
       ECAL_API void ShutdownProcess(const std::string& unit_name_);
   
       ECAL_API void ShutdownProcess(int process_id_);
   
       ECAL_API void ShutdownProcesses();
   
       ECAL_API void ShutdownCore();
   
       ECAL_API void EnableLoopback(bool state_);
   
       ECAL_API void PubShareType(bool state_);
   
       ECAL_API void PubShareDescription(bool state_);
   
       ECAL_API void GetTopics(std::unordered_map<std::string, SDataTypeInformation>& topic_info_map_);
   
       ECAL_API void GetTopicNames(std::vector<std::string>& topic_names_);
   
       ECAL_DEPRECATE_SINCE_5_13("Please use the method bool GetTopicDataTypeInformation(const std::string& topic_name_, SDataTypeInformation& topic_info_) instead. You can extract the type information from the members encoding and type of the SDataTypeInformation variable. This function will be removed in future eCAL versions.")
       ECAL_API bool GetTopicTypeName(const std::string& topic_name_, std::string& topic_type_);
   
       ECAL_DEPRECATE_SINCE_5_13("Please use the method bool GetTopicDataTypeInformation(const std::string& topic_name_, SDataTypeInformation& topic_info_) instead. You can extract the type information from the members encoding and type of the SDataTypeInformation variable. This function will be removed in future eCAL versions.")
       ECAL_API std::string GetTopicTypeName(const std::string& topic_name_);
   
       ECAL_DEPRECATE_SINCE_5_13("Please use the method bool GetTopicDataTypeInformation(const std::string& topic_name_, SDataTypeInformation& topic_info_) instead. You can extract the descriptor from the STopicInformation variable. This function will be removed in future eCAL versions.")
       ECAL_API bool GetTopicDescription(const std::string& topic_name_, std::string& topic_desc_);
   
       ECAL_DEPRECATE_SINCE_5_13("Please use the method bool GetTopicDataTypeInformation(const std::string& topic_name_, SDataTypeInformation& topic_info_) instead. You can extract the descriptor from the STopicInformation variable. This function will be removed in future eCAL versions.")
       ECAL_API std::string GetTopicDescription(const std::string& topic_name_);
   
       ECAL_API bool GetTopicDataTypeInformation(const std::string& topic_name_, SDataTypeInformation& topic_info_);
   
       ECAL_API void GetServices(std::map<std::tuple<std::string, std::string>, SServiceMethodInformation>& service_info_map_);
   
       ECAL_API void GetServiceNames(std::vector<std::tuple<std::string, std::string>>& service_method_names_);
   
       ECAL_API bool GetServiceTypeNames(const std::string& service_name_, const std::string& method_name_, std::string& req_type_, std::string& resp_type_);
   
       ECAL_API bool GetServiceDescription(const std::string& service_name_, const std::string& method_name_, std::string& req_desc_, std::string& resp_desc_);
   
       ECAL_DEPRECATE_SINCE_5_10("Please use the method bool GetTopicDataTypeInformation(const std::string& topic_name_, SDataTypeInformation& topic_info_) instead. You can extract the type information from the members encoding and type of the SDataTypeInformation variable. This function will be removed in future eCAL versions.")
       ECAL_API bool GetTypeName(const std::string& topic_name_, std::string& topic_type_);
   
       ECAL_DEPRECATE_SINCE_5_10("Please use the method bool GetTopicDataTypeInformation(const std::string& topic_name_, SDataTypeInformation& topic_info_) instead. You can extract the type information from the members encoding and type of the SDataTypeInformation variable. This function will be removed in future eCAL versions.")
       ECAL_API std::string GetTypeName(const std::string& topic_name_);
   
       ECAL_DEPRECATE_SINCE_5_10("Please use the method bool GetTopicDataTypeInformation(const std::string& topic_name_, SDataTypeInformation& topic_info_) instead. You can extract the descriptor from the SDataTypeInformation variable. This function will be removed in future eCAL versions.")
       ECAL_API bool GetDescription(const std::string& topic_name_, std::string& topic_desc_);
   
       ECAL_DEPRECATE_SINCE_5_13("Please use the method bool GetTopicDataTypeInformation(const std::string& topic_name_, SDataTypeInformation& topic_info_) instead. You can extract the descriptor from the SDataTypeInformation variable. This function will be removed in future eCAL versions.")
       ECAL_API std::string GetDescription(const std::string& topic_name_);
   
       ECAL_API std::pair<std::string, std::string> SplitCombinedTopicType(const std::string& combined_topic_type_);
   
       ECAL_API std::string CombinedTopicEncodingAndType(const std::string& topic_encoding_, const std::string& topic_type_);
     }
   }
