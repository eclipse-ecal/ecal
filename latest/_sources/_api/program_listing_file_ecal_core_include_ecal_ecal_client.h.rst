
.. _program_listing_file_ecal_core_include_ecal_ecal_client.h:

Program Listing for File ecal_client.h
======================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_client.h>` (``ecal/core/include/ecal/ecal_client.h``)

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
   #include <ecal/ecal_namespace.h>
   #include <ecal/ecal_os.h>
   
   #include <ecal/ecal_callback.h>
   #include <ecal/ecal_client_instance.h>
   #include <ecal/ecal_service_info.h>
   #include <ecal/ecal_types.h>
   
   #include <memory>
   #include <string>
   #include <vector>
   
   namespace eCAL
   {
     class CServiceClientImpl;
   
     ECAL_CORE_NAMESPACE_V6
     {
       class ECAL_API_CLASS CServiceClient
       {
       public:
         ECAL_API_EXPORTED_MEMBER
           CServiceClient(const std::string& service_name_, const ServiceMethodInformationMapT method_information_map_ = ServiceMethodInformationMapT(), const ClientEventCallbackT event_callback_ = ClientEventCallbackT());
   
         ECAL_API_EXPORTED_MEMBER
           virtual ~CServiceClient();
   
         CServiceClient(const CServiceClient&) = delete;
         
         CServiceClient& operator=(const CServiceClient&) = delete;
   
         ECAL_API_EXPORTED_MEMBER
           CServiceClient(CServiceClient&& rhs) noexcept;
   
         ECAL_API_EXPORTED_MEMBER
           CServiceClient& operator=(CServiceClient&& rhs) noexcept;
   
         ECAL_API_EXPORTED_MEMBER
           std::vector<CClientInstance> GetClientInstances() const;
   
         ECAL_API_EXPORTED_MEMBER
           bool CallWithResponse(const std::string& method_name_, const std::string& request_, int timeout_, ServiceIDResponseVecT& service_response_vec_) const;
   
         ECAL_API_EXPORTED_MEMBER
           bool CallWithCallback(const std::string& method_name_, const std::string& request_, int timeout_, const ResponseIDCallbackT& response_callback_) const;
   
         ECAL_API_EXPORTED_MEMBER
           bool CallWithCallbackAsync(const std::string& method_name_, const std::string& request_, const ResponseIDCallbackT& response_callback_) const;
   
         ECAL_API_EXPORTED_MEMBER
           std::string GetServiceName() const;
   
         ECAL_API_EXPORTED_MEMBER
           Registration::SServiceId GetServiceId() const;
   
         ECAL_API_EXPORTED_MEMBER
           bool IsConnected() const;
   
       private:
         std::shared_ptr<eCAL::v6::CServiceClientImpl> m_service_client_impl;
       };
     }
   }
