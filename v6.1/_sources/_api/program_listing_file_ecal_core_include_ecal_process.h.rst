
.. _program_listing_file_ecal_core_include_ecal_process.h:

Program Listing for File process.h
==================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_process.h>` (``ecal/core/include/ecal/process.h``)

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
   
   #include <chrono>
   #include <string>
   
   #include <ecal/os.h>
   #include <ecal/process_mode.h>
   #include <ecal/process_severity.h>
   #include <ecal/deprecate.h>
   
   namespace eCAL
   {
     namespace Process
     {
       ECAL_API void DumpConfig();
   
       ECAL_API void DumpConfig(std::string& cfg_s_);
   
       ECAL_API const std::string& GetHostName();
   
       ECAL_API const std::string& GetShmTransportDomain();
   
       ECAL_API const std::string& GetUnitName();
   
       ECAL_API void SleepMS(long time_ms_);
   
       ECAL_API void SleepNS(long long time_ns_);
   
       template <typename Rep, typename Period>
       void SleepFor( std::chrono::duration<Rep, Period> time )
       {
           auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time).count();
           SleepNS(ns);
       }
   
       ECAL_API int GetProcessID();
   
       ECAL_API const std::string& GetProcessIDAsString();
   
       ECAL_API const std::string& GetProcessName();
   
       ECAL_API const std::string& GetProcessParameter();
   
       ECAL_API void SetState(eCAL::Process::eSeverity severity_, eCAL::Process::eSeverityLevel level_, const std::string& info_);
   
       ECAL_DEPRECATE_SINCE_6_0("This function is deprecated and will be removed in future eCAL versions.")
       ECAL_API int StartProcess(const char* proc_name_, const char* proc_args_, const char* working_dir_, bool create_console_, eCAL::Process::eStartMode process_mode_, bool block_);
   
       ECAL_DEPRECATE_SINCE_6_0("This function is deprecated and will be removed in future eCAL versions.")
       ECAL_API bool StopProcess(const char* proc_name_);
   
       ECAL_DEPRECATE_SINCE_6_0("This function is deprecated and will be removed in future eCAL versions.")
       ECAL_API bool StopProcess(int proc_id_);
     }
   }
   
