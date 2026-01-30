
.. _program_listing_file_ecal_core_include_ecal_util.h:

Program Listing for File util.h
===============================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_util.h>` (``ecal/core/include/ecal/util.h``)

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
   
   #include <ecal/os.h>
   #include <ecal/deprecate.h>
   
   #include <string>
   #include <utility>
   
   namespace eCAL
   {
     namespace Util
     {
       ECAL_API std::string GeteCALDataDir();
   
       ECAL_API std::string GeteCALLogDir();
   
       ECAL_API void ShutdownProcess(const std::string& unit_name_);
   
       ECAL_API void ShutdownProcess(int process_id_);
   
       ECAL_API void ShutdownProcesses();
   
       ECAL_DEPRECATE_SINCE_6_0("Please refactor your code to use SDatatypeInformation. This function will be removed with eCAL 7")
       ECAL_API std::pair<std::string, std::string> SplitCombinedTopicType(const std::string& combined_topic_type_);
   
       ECAL_DEPRECATE_SINCE_6_0("Please refactor your code to use SDatatypeInformation. This function will be removed with eCAL 7")
       ECAL_API std::string CombinedTopicEncodingAndType(const std::string& topic_encoding_, const std::string& topic_type_);
     }
   }
   
