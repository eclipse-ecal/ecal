
.. _program_listing_file_ecal_core_include_ecal_ecal_core.h:

Program Listing for File ecal_core.h
====================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_core.h>` (``ecal/core/include/ecal/ecal_core.h``)

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
   
   #include <ecal/ecal_init.h>
   
   #include <string>
   #include <vector>
   #include <utility>
   
   #include <ecal/ecal_os.h>
   
   namespace eCAL
   {
     ECAL_API const char* GetVersionString();
   
     ECAL_API const char* GetVersionDateString();
   
     ECAL_API int GetVersion(int* major_, int* minor_, int* patch_);
   
     ECAL_API int Initialize(int argc_ = 0, char **argv_ = nullptr, const char *unit_name_ = nullptr, unsigned int components_ = Init::Default);
   
     ECAL_API int Initialize(std::vector<std::string> args_, const char *unit_name_ = nullptr, unsigned int components_ = Init::Default);
   
     ECAL_API int Finalize(unsigned int components_ = Init::Default);
   
     ECAL_API int IsInitialized(unsigned int component_ = 0);
   
     ECAL_API int SetUnitName(const char *unit_name_);
   
     ECAL_API bool Ok();
   }
