
.. _program_listing_file_ecal_core_include_ecal_ecal_log.h:

Program Listing for File ecal_log.h
===================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_log.h>` (``ecal/core/include/ecal/ecal_log.h``)

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
   
   #include <ecal/ecal_os.h>
   #include <ecal/ecal_log_level.h>
   #include <ecal/ecal_deprecate.h>
   
   #include <string>
   
   namespace eCAL
   {  
     namespace Logging
     {
       ECAL_API void SetLogLevel(eCAL_Logging_eLogLevel level_);
     
       ECAL_API eCAL_Logging_eLogLevel GetLogLevel();
   
       ECAL_API void Log(const std::string& msg_);
   
       inline void Log(eCAL_Logging_eLogLevel level_, const std::string& msg_)
       {
         SetLogLevel(level_);
         Log(msg_);
       }
   
       ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
       ECAL_API void StartCoreTimer();
   
       ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
       ECAL_API void StopCoreTimer();
   
       ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
       ECAL_API void SetCoreTime(double time_);
   
       ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
       ECAL_API double GetCoreTime();
     }
   }
