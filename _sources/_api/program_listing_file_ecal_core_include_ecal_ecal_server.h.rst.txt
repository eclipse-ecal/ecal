
.. _program_listing_file_ecal_core_include_ecal_ecal_server.h:

Program Listing for File ecal_server.h
======================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_server.h>` (``ecal/core/include/ecal/ecal_server.h``)

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
   
   #include <string>
   #include <vector>
   #include <memory>
   
   namespace eCAL
   {
     class CServiceServerImpl;
   
     class CServiceServer
     {
     public:
       ECAL_API CServiceServer();
   
       ECAL_API CServiceServer(const std::string& service_name_);
   
       ECAL_API virtual ~CServiceServer();
   
       ECAL_API CServiceServer(const CServiceServer&) = delete;
   
       ECAL_API CServiceServer& operator=(const CServiceServer&) = delete;
   
       ECAL_API bool Create(const std::string& service_name_);
   
       ECAL_API bool Destroy();
   
       ECAL_API bool AddDescription(const std::string& method_, const std::string& req_type_, const std::string& req_desc_, const std::string& resp_type_, const std::string& resp_desc_);
   
       ECAL_API bool AddMethodCallback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const MethodCallbackT& callback_);
   
       ECAL_API bool RemMethodCallback(const std::string& method_);
   
       ECAL_API bool AddEventCallback(eCAL_Server_Event type_, ServerEventCallbackT callback_);
   
       ECAL_API bool RemEventCallback(eCAL_Server_Event type_);
   
       ECAL_API std::string GetServiceName();
   
       ECAL_API bool IsConnected();
   
     private:
       std::shared_ptr<CServiceServerImpl> m_service_server_impl;
       bool                                m_created;
     };
   } 
