
.. _program_listing_file_ecal_core_include_ecal_ecal_process.h:

Program Listing for File ecal_process.h
=======================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_process.h>` (``ecal/core/include/ecal/ecal_process.h``)

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
   
   #include <chrono>
   #include <string>
   
   #include <ecal/ecal_deprecate.h>
   #include <ecal/ecal_os.h>
   #include <ecal/ecal_process_mode.h>
   #include <ecal/ecal_process_severity.h>
   #include <ecal/ecal_callback.h>
   
   namespace eCAL
   {
     namespace Process
     {
       ECAL_API void DumpConfig();
   
       ECAL_API void DumpConfig(std::string& cfg_s_);
   
       ECAL_API std::string GetHostName();
   
       ECAL_API std::string GetHostGroupName();
   
       ECAL_DEPRECATE_SINCE_5_10("Will be removed in future eCAL versions.")
       ECAL_API int GetHostID();
   
       ECAL_API std::string GetUnitName();
   
       ECAL_API std::string GetTaskParameter(const char* sep_);
   
       ECAL_API void SleepMS(long time_ms_);
   
       ECAL_API void SleepNS(const long long time_ns_);
   
       template <typename Rep, typename Period>
       void SleepFor( std::chrono::duration<Rep, Period> time )
       {
           auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time).count();
           SleepNS(ns);
       }
   
       ECAL_API int GetProcessID();
   
       ECAL_API std::string GetProcessIDAsString();
   
       ECAL_API std::string GetProcessName();
   
       ECAL_API std::string GetProcessParameter();
   
       ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
       ECAL_API float GetProcessCpuUsage();
   
       ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
       ECAL_API unsigned long GetProcessMemory();
   
       ECAL_DEPRECATE_SINCE_5_4("Will be removed in future eCAL versions.")
       ECAL_API long long GetSClock();
   
       ECAL_DEPRECATE_SINCE_5_4("Will be removed in future eCAL versions.")
       ECAL_API long long GetSBytes();
   
       ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
       ECAL_API long long GetWClock();
   
       ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
       ECAL_API long long GetWBytes();
   
       ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
       ECAL_API long long GetRClock();
   
       ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
       ECAL_API long long GetRBytes();
   
       ECAL_API void SetState(eCAL_Process_eSeverity severity_, eCAL_Process_eSeverity_Level level_, const char* info_);
   
       ECAL_API int AddRegistrationCallback(enum eCAL_Registration_Event event_, RegistrationCallbackT callback_);
   
       ECAL_API int RemRegistrationCallback(enum eCAL_Registration_Event event_);
   
       ECAL_API int StartProcess(const char* proc_name_, const char* proc_args_, const char* working_dir_, bool create_console_, eCAL_Process_eStartMode process_mode_, bool block_);
   
       ECAL_API bool StopProcess(const char* proc_name_);
   
       ECAL_API bool StopProcess(int proc_id_);
     }
   }
