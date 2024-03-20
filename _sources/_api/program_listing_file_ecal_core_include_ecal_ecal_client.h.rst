
.. _program_listing_file_ecal_core_include_ecal_ecal_client.h:

Program Listing for File ecal_client.h
======================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_client.h>` (``ecal/core/include/ecal/ecal_client.h``)

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
   
   #include <ecal/ecal_deprecate.h>
   #include <ecal/ecal_os.h>
   #include <ecal/ecal_callback.h>
   #include <ecal/ecal_service_info.h>
   
   #include <iostream>
   #include <string>
   #include <vector>
   #include <memory>
   
   namespace eCAL
   {
     class CServiceClientImpl;
   
     class CServiceClient
     {
     public:
       ECAL_API CServiceClient();
   
       ECAL_API CServiceClient(const std::string& service_name_);
   
       ECAL_API virtual ~CServiceClient();
   
       ECAL_API CServiceClient(const CServiceClient&) = delete;
   
       ECAL_API CServiceClient& operator=(const CServiceClient&) = delete;
   
       ECAL_API bool Create(const std::string& service_name_);
   
       ECAL_API bool Destroy();
   
       ECAL_API bool SetHostName(const std::string& host_name_);
   
       ECAL_API bool Call(const std::string& method_name_, const std::string& request_, int timeout_ = -1);
   
       ECAL_API bool Call(const std::string& method_name_, const std::string& request_, int timeout_, ServiceResponseVecT* service_response_vec_);
   
       ECAL_DEPRECATE_SINCE_5_10("Please use the create method bool Call(const std::string& method_name_, const std::string& request_, int timeout_, ServiceResponseVecT* service_response_vec_) instead. This function will be removed in future eCAL versions.")
       ECAL_API bool Call(const std::string& host_name_, const std::string& method_name_, const std::string& request_, struct SServiceResponse& service_info_, std::string& response_);
   
       ECAL_API bool CallAsync(const std::string& method_name_, const std::string& request_, int timeout_ = -1);
   
       ECAL_API bool AddResponseCallback(const ResponseCallbackT& callback_);
   
       ECAL_API bool RemResponseCallback();
   
       ECAL_API bool AddEventCallback(eCAL_Client_Event type_, ClientEventCallbackT callback_);
   
       ECAL_API bool RemEventCallback(eCAL_Client_Event type_);
   
       ECAL_API std::string GetServiceName();
   
       ECAL_API bool IsConnected();
   
     protected:
       std::shared_ptr<eCAL::CServiceClientImpl> m_service_client_impl;
       bool                m_created;
     };
   }
