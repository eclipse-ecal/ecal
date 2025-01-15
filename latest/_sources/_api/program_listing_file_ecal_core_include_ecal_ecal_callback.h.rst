
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
   #include <ecal/cimpl/ecal_callback_cimpl.h>
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
   
     using TimerCallbackT = std::function<void()>;
   
     inline namespace v6
     {
       using ReceiveCallbackT = std::function<void(const Registration::STopicId& topic_id_, const SDataTypeInformation& data_type_info_, const SReceiveCallbackData& data_)>;
   
       struct SPubEventCallbackData
       {
         eCAL_Publisher_Event type{ pub_event_none };  
         long long            time{ 0 };               
         long long            clock{ 0 };              
         SDataTypeInformation tdatatype;               
       };
   
       using PubEventCallbackT = std::function<void(const Registration::STopicId& topic_id_, const SPubEventCallbackData& data_)>;
   
       struct SSubEventCallbackData
       {
         eCAL_Subscriber_Event type{ sub_event_none }; 
         long long             time{ 0 };              
         long long             clock{ 0 };             
         SDataTypeInformation  tdatatype;              
       };
   
       using SubEventCallbackT = std::function<void(const Registration::STopicId& topic_id_, const SSubEventCallbackData& data_)>;
   
       struct SClientEventCallbackData
       {
         eCAL_Client_Event type = client_event_none;  
         long long         time = 0;                  
       };
   
       using ClientEventCallbackT = std::function<void(const Registration::SServiceMethodId& service_id_, const SClientEventCallbackData& data_)>;
   
       struct SServerEventCallbackData
       {
         eCAL_Server_Event type = server_event_none;  
         long long         time = 0;                  
       };
   
       using ServerEventCallbackT = std::function<void(const Registration::SServiceMethodId& service_id_, const struct SServerEventCallbackData& data_)>;
     }
   }
