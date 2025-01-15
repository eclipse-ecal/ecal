
.. _program_listing_file_ecal_core_include_ecal_ecal_client_instance.h:

Program Listing for File ecal_client_instance.h
===============================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_client_instance.h>` (``ecal/core/include/ecal/ecal_client_instance.h``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

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
   
   #pragma once
   
   #include <ecal/ecal_callback.h>
   #include <ecal/ecal_deprecate.h>
   #include <ecal/ecal_os.h>
   #include <ecal/ecal_types.h>
   #include <ecal/ecal_service_info.h>
   
   #include <memory>
   #include <string>
   
   namespace eCAL
   {
     class CServiceClientImpl;
   
     class ECAL_API_CLASS CClientInstance final
     {
     public:
       // Constructor
       ECAL_API_EXPORTED_MEMBER
         CClientInstance(const Registration::SEntityId& entity_id_, const std::shared_ptr<CServiceClientImpl>& service_client_id_impl_);
   
       // Defaulted destructor
       ~CClientInstance() = default;
   
       // Deleted copy constructor and copy assignment operator
       CClientInstance(const CClientInstance&) = delete;
       CClientInstance& operator=(const CClientInstance&) = delete;
   
       // Defaulted move constructor and move assignment operator
       ECAL_API_EXPORTED_MEMBER CClientInstance(CClientInstance&& rhs) noexcept = default;
       ECAL_API_EXPORTED_MEMBER CClientInstance& operator=(CClientInstance&& rhs) noexcept = default;
   
       ECAL_API_EXPORTED_MEMBER
         std::pair<bool, SServiceIDResponse> CallWithResponse(const std::string& method_name_, const std::string& request_, int timeout_ = -1);
   
       ECAL_API_EXPORTED_MEMBER
         bool CallWithCallback(const std::string& method_name_, const std::string& request_, int timeout_, const ResponseIDCallbackT& response_callback_);
   
       ECAL_API_EXPORTED_MEMBER
         bool CallWithCallbackAsync(const std::string& method_name_, const std::string& request_, const ResponseIDCallbackT& response_callback_);
   
       ECAL_API_EXPORTED_MEMBER
         bool IsConnected() const;
   
       ECAL_API_EXPORTED_MEMBER
         Registration::SEntityId GetClientID() const;
   
     private:
       Registration::SEntityId                         m_entity_id;
       const std::shared_ptr<eCAL::CServiceClientImpl> m_service_client_impl;
     };
   }
