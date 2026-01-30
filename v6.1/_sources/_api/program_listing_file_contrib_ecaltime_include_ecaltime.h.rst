
.. _program_listing_file_contrib_ecaltime_include_ecaltime.h:

Program Listing for File ecaltime.h
===================================

|exhale_lsh| :ref:`Return to documentation for file <file_contrib_ecaltime_include_ecaltime.h>` (``contrib/ecaltime/include/ecaltime.h``)

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
   
   #ifndef ecaltime_h_included
   #define ecaltime_h_included
   
   #ifdef _MSC_VER
     #ifdef ECAL_TIME_PLUGIN_API_EXPORT
       #define ECALTIME_API __declspec(dllexport)
     #else
       #define ECALTIME_API
     #endif
   #else
     #define ECALTIME_API
   #endif
   
   #ifdef __cplusplus
   extern "C"
   {
   #endif /*__cplusplus*/
     ECALTIME_API int etime_initialize(void);
   
     ECALTIME_API int etime_finalize(void);
   
     ECALTIME_API long long etime_get_nanoseconds();
   
     ECALTIME_API int etime_set_nanoseconds(long long time_);
   
     ECALTIME_API int etime_is_synchronized();
   
     ECALTIME_API int etime_is_master();
   
     ECALTIME_API void etime_sleep_for_nanoseconds(long long duration_nsecs_);
   
     ECALTIME_API void etime_get_status(int* error_, char* status_message_, int max_len_);
   
   #ifdef __cplusplus
   }
   #endif /*__cplusplus*/
   
   #endif /*ecaltime_h_included*/
