
.. _program_listing_file_ecal_core_include_ecal_ecal_callback.h:

Program Listing for File ecal_callback.h
========================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_callback.h>` (``ecal/core/include/ecal/ecal_callback.h``)

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
   
   #include <ecal/ecal_deprecate.h>
   #include <ecal/ecal_types.h>
   
   #include <functional>
   #include <string>
   
   namespace eCAL
   {
     struct SReceiveCallbackData
     {
       void*     buf   = nullptr;  
       long      size  = 0;        
       long long id    = 0;        
       long long time  = 0;        
       long long clock = 0;        
     };
   
     enum class eSubscriberEvent
     {
       none = 0,
       connected = 1,
       disconnected = 2,
       dropped = 3,
       corrupted = 5,
       update_connection = 6,
     };
   
     inline std::string to_string(eSubscriberEvent event_) {
       switch (event_) {
       case eSubscriberEvent::none:                   return "NONE";
       case eSubscriberEvent::connected:              return "CONNECTED";
       case eSubscriberEvent::disconnected:           return "DISCONNECTED";
       case eSubscriberEvent::dropped :               return "DROPPED";
       case eSubscriberEvent::corrupted:              return "CORRUPTED";
       case eSubscriberEvent::update_connection :     return "UPDATED_CONNECTION";
       default:            return "Unknown";
       }
     }
   
     enum class ePublisherEvent
     {
       none = 0,
       connected = 1,
       disconnected = 2,
       dropped = 3,
       update_connection = 4,
     };
   
     inline std::string to_string(ePublisherEvent event_) {
       switch (event_) {
       case ePublisherEvent::none:                   return "NONE";
       case ePublisherEvent::connected:              return "CONNECTED";
       case ePublisherEvent::disconnected:           return "DISCONNECTED";
       case ePublisherEvent::dropped:                return "DROPPED";
       case ePublisherEvent::update_connection:      return "UPDATED_CONNECTION";
       default:            return "Unknown";
       }
     }
   
     enum class eClientEvent
     {
       none = 0,
       connected = 1,
       disconnected = 2,
       timeout = 3,
     };
   
     inline std::string to_string(eClientEvent event_) {
       switch (event_) {
       case eClientEvent::none:                   return "NONE";
       case eClientEvent::connected:              return "CONNECTED";
       case eClientEvent::disconnected:           return "DISCONNECTED";
       default:            return "Unknown";
       }
     }
   
     enum class eServerEvent
     {
       none = 0,
       connected = 1,
       disconnected = 2,
     };
   
     inline std::string to_string(eServerEvent event_) {
       switch (event_) {
       case eServerEvent::none:                   return "NONE";
       case eServerEvent::connected:              return "CONNECTED";
       case eServerEvent::disconnected:           return "DISCONNECTED";
       default:            return "Unknown";
       }
     }
   
     using TimerCallbackT = std::function<void()>;
   
     inline namespace v6
     {
       using ReceiveCallbackT = std::function<void(const Registration::STopicId& topic_id_, const SDataTypeInformation& data_type_info_, const SReceiveCallbackData& data_)>;
   
       struct SPubEventCallbackData
       {
         ePublisherEvent      type{ ePublisherEvent::none };  
         long long            time{ 0 };               
         long long            clock{ 0 };              
         SDataTypeInformation tdatatype;               
       };
   
       using PubEventCallbackT = std::function<void(const Registration::STopicId& topic_id_, const SPubEventCallbackData& data_)>;
   
       struct SSubEventCallbackData
       {
         eSubscriberEvent      type{ eSubscriberEvent::none }; 
         long long             time{ 0 };              
         long long             clock{ 0 };             
         SDataTypeInformation  tdatatype;              
       };
   
       using SubEventCallbackT = std::function<void(const Registration::STopicId& topic_id_, const SSubEventCallbackData& data_)>;
   
       struct SClientEventCallbackData
       {
         eClientEvent      type{ eClientEvent::none };  
         long long         time = 0;                  
       };
   
       using ClientEventCallbackT = std::function<void(const Registration::SServiceMethodId& service_id_, const SClientEventCallbackData& data_)>;
   
       struct SServerEventCallbackData
       {
         eServerEvent      type{ eServerEvent::none };  
         long long         time = 0;                  
       };
   
       using ServerEventCallbackT = std::function<void(const Registration::SServiceMethodId& service_id_, const struct SServerEventCallbackData& data_)>;
     }
   }
