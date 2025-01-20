
.. _program_listing_file_ecal_core_include_ecal_ecal_registration.h:

Program Listing for File ecal_registration.h
============================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_registration.h>` (``ecal/core/include/ecal/ecal_registration.h``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

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
   
   #include <ecal/ecal_os.h>
   #include <ecal/ecal_types.h>
   
   #include <cstdint>
   #include <functional>
   #include <map>
   #include <set>
   #include <string>
   #include <vector>
   
   namespace eCAL
   {
     namespace Registration
     {
       struct SServiceMethod
       {
         std::string service_name;
         std::string method_name;
   
         bool operator<(const SServiceMethod& other) const
         {
           return std::tie(service_name, method_name) < std::tie(other.service_name, other.method_name);
         }
       };
   
       using CallbackToken = std::size_t;
   
       enum class RegistrationEventType
       {
         new_entity,     
         deleted_entity  
       };
   
       /* @brief Event callback, when a topic related entity (publisher / subscriber) has been created or deleted */
       using TopicEventCallbackT = std::function<void(const STopicId&, RegistrationEventType)>;
   
       ECAL_API std::set<STopicId> GetPublisherIDs();
   
       ECAL_API bool GetPublisherInfo(const STopicId& id_, SDataTypeInformation& topic_info_);
   
       ECAL_API CallbackToken AddPublisherEventCallback(const TopicEventCallbackT& callback_);
   
       ECAL_API void RemPublisherEventCallback(CallbackToken token_);
   
       ECAL_API std::set<STopicId> GetSubscriberIDs();
   
       ECAL_API bool GetSubscriberInfo(const STopicId& id_, SDataTypeInformation& topic_info_);
   
       ECAL_API CallbackToken AddSubscriberEventCallback(const TopicEventCallbackT& callback_);
   
       ECAL_API void RemSubscriberEventCallback(CallbackToken token_);
   
       ECAL_API std::set<SServiceMethodId> GetServiceIDs();
   
       ECAL_API bool GetServiceInfo(const SServiceMethodId& id_, SServiceMethodInformation& service_method_info_);
   
       ECAL_API std::set<SServiceMethodId> GetClientIDs();
   
       ECAL_API bool GetClientInfo(const SServiceMethodId& id_, SServiceMethodInformation& service_method_info_);
   
       ECAL_API void GetTopicNames(std::set<std::string>& topic_names_);
   
       ECAL_API void GetServiceMethodNames(std::set<SServiceMethod>& service_method_names_);
   
       ECAL_API void GetClientMethodNames(std::set<SServiceMethod>& client_method_names_);
     }
   }
