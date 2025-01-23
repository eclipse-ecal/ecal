
.. _program_listing_file_ecal_core_include_ecal_log.h:

Program Listing for File log.h
==============================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_log.h>` (``ecal/core/include/ecal/log.h``)

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
   #include <ecal/log_level.h>
   #include <ecal/types/logging.h>
   
   #include <list>
   #include <string>
   
   namespace eCAL
   {  
     namespace Logging
     {
       ECAL_API void Log(eLogLevel level_, const std::string& msg_);
   
       ECAL_API bool GetLogging(std::string& log_);
   
       ECAL_API bool GetLogging(Logging::SLogging& log_);
     }
   }
   
