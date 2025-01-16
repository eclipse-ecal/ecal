
.. _program_listing_file_ecal_core_include_ecal_ecal_client_v5.h:

Program Listing for File ecal_client_v5.h
=========================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_client_v5.h>` (``ecal/core/include/ecal/ecal_client_v5.h``)

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
   #include <ecal/ecal_os.h>
   #include <ecal/ecal_callback_v5.h>
   #include <ecal/ecal_service_info.h>
   
   #include <memory>
   #include <string>
   
   namespace eCAL
   {
     namespace v5
     {
       class CServiceClientImpl;
   
       class ECAL_API_CLASS CServiceClient
       {
       public:
         ECAL_API_EXPORTED_MEMBER
           CServiceClient();
   
         ECAL_API_EXPORTED_MEMBER
           explicit CServiceClient(const std::string& service_name_);
   
         ECAL_API_EXPORTED_MEMBER
           explicit CServiceClient(const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_);
   
         ECAL_API_EXPORTED_MEMBER
           virtual ~CServiceClient();
   
         CServiceClient(const CServiceClient&) = delete;
   
         CServiceClient& operator=(const CServiceClient&) = delete;
   
         ECAL_API_EXPORTED_MEMBER
           bool Create(const std::string& service_name_);
   
         ECAL_API_EXPORTED_MEMBER
           bool Create(const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_);
   
         ECAL_API_EXPORTED_MEMBER
           bool Destroy();
   
         ECAL_API_EXPORTED_MEMBER
           bool SetHostName(const std::string& host_name_);
   
         ECAL_API_EXPORTED_MEMBER
           bool Call(const std::string& method_name_, const std::string& request_, int timeout_ = -1);
   
         ECAL_API_EXPORTED_MEMBER
           bool Call(const std::string& method_name_, const std::string& request_, int timeout_, ServiceResponseVecT* service_response_vec_);
   
         ECAL_API_EXPORTED_MEMBER
           bool CallAsync(const std::string& method_name_, const std::string& request_, int timeout_ = -1);
   
         ECAL_API_EXPORTED_MEMBER
           bool AddResponseCallback(const ResponseCallbackT& callback_);
   
         ECAL_API_EXPORTED_MEMBER
           bool RemResponseCallback();
   
         ECAL_API_EXPORTED_MEMBER
           bool AddEventCallback(eClientEvent type_, ClientEventCallbackT callback_);
   
         ECAL_API_EXPORTED_MEMBER
           bool RemEventCallback(eClientEvent type_);
   
         ECAL_API_EXPORTED_MEMBER
           std::string GetServiceName();
   
         ECAL_API_EXPORTED_MEMBER
           bool IsConnected();
   
       private:
         std::shared_ptr<CServiceClientImpl> m_service_client_impl;
       };
     }
   }
