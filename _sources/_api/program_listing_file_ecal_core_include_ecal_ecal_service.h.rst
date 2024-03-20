
.. _program_listing_file_ecal_core_include_ecal_ecal_service.h:

Program Listing for File ecal_service.h
=======================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_service.h>` (``ecal/core/include/ecal/ecal_service.h``)

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
   
   #ifdef _MSC_VER
   #pragma message("WARNING: The header file ecal_service.h is deprecated. It will be removed in future eCAL versions. Please include <ecal/ecal_server.h> and / or <ecal/ecal_client.h> instead")
   #endif /*_MSC_VER*/
   #ifdef __GNUC__
   #pragma message "WARNING: The header file ecal_service.h is deprecated. It will be removed in future eCAL versions. Please include <ecal/ecal_server.h> and / or <ecal/ecal_client.h> instead"
   #endif /* __GNUC__ */
   
   #include <ecal/ecal_server.h>
   #include <ecal/ecal_client.h>
