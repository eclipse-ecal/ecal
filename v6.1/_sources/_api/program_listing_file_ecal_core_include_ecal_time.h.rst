
.. _program_listing_file_ecal_core_include_ecal_time.h:

Program Listing for File time.h
===============================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_time.h>` (``ecal/core/include/ecal/time.h``)

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
   
   #include <ecal/os.h>
   #include <string>
   #include <chrono>
   
   namespace eCAL
   {
     namespace Time
     {
       ECAL_API const std::string& GetName();
   
       ECAL_API long long GetMicroSeconds();
   
       ECAL_API long long GetNanoSeconds();
   
       ECAL_API bool SetNanoSeconds(long long time_);
   
       ECAL_API bool IsSynchronized();
   
       ECAL_API bool IsMaster();
       
       ECAL_API void SleepForNanoseconds(long long duration_nsecs_);
   
       ECAL_API void GetStatus(int& error_, std::string* status_message_);
   
   
       template<class _Rep,
                class _Period> inline
         void sleep_for(const std::chrono::duration<_Rep, _Period>& _Rel_time) {
                  long long duration_nsecs = (std::chrono::duration_cast<std::chrono::nanoseconds>(_Rel_time)).count();
                  SleepForNanoseconds(duration_nsecs);
       }
   
       /* @cond */
       struct ecal_clock
       {
         using duration   = std::chrono::nanoseconds;
         using rep        = duration::rep;
         using period     = duration::period;
         using time_point = std::chrono::time_point<ecal_clock>;
         static const bool is_steady = false;
   
         static time_point now()
         {
           return time_point (duration(GetNanoSeconds()));
         }
       };
       /* @endcond */
     }
   }
   
