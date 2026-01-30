
.. _program_listing_file_ecal_core_include_ecal_core.h:

Program Listing for File core.h
===============================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_core.h>` (``ecal/core/include/ecal/core.h``)

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
   
   #include <ecal/init.h>
   
   #include <string>
   #include <vector>
   #include <utility>
   
   #include <ecal/os.h>
   #include <ecal/config/configuration.h>
   #include <ecal/types.h>
   
   namespace eCAL
   {
     ECAL_API const std::string& GetVersionString();
   
     ECAL_API const std::string& GetVersionDateString();
   
     ECAL_API SVersion GetVersion();
   
     ECAL_API bool Initialize(const std::string& unit_name_ = "", unsigned int components_ = Init::Default);
   
     ECAL_API bool Initialize(eCAL::Configuration& config_, const std::string& unit_name_ = "", unsigned int components_ = Init::Default);
   
     ECAL_API bool Finalize();
   
     ECAL_API bool IsInitialized();
   
     ECAL_API bool IsInitialized(unsigned int component_);
   
     ECAL_API bool Ok();
   }
   
