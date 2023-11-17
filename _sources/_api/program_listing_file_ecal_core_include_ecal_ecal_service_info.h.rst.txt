
.. _program_listing_file_ecal_core_include_ecal_ecal_service_info.h:

Program Listing for File ecal_service_info.h
============================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_service_info.h>` (``ecal/core/include/ecal/ecal_service_info.h``)

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
   
   #include <ecal/cimpl/ecal_service_info_cimpl.h>
   
   #include <functional>
   #include <string>
   #include <vector>
   
   namespace eCAL
   {
     struct SServiceResponse
     {
       SServiceResponse()
       {
         ret_state  = 0;
         call_state = call_state_none;
       };
       std::string  host_name;      
       std::string  service_name;   
       std::string  service_id;     
       std::string  method_name;    
       std::string  error_msg;      
       int          ret_state;      
       eCallState   call_state;     
       std::string  response;       
     };
     typedef std::vector<SServiceResponse> ServiceResponseVecT; 
   
     typedef std::function<int(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_)> MethodCallbackT;
   
     typedef std::function<void(const struct SServiceResponse& service_response_)> ResponseCallbackT;
   }
