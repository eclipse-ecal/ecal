
.. _program_listing_file_ecal_core_include_ecal_ecal_log_level.h:

Program Listing for File ecal_log_level.h
=========================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_log_level.h>` (``ecal/core/include/ecal/ecal_log_level.h``)

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
   
   enum eCAL_Logging_eLogLevel
   {
     log_level_none    =   0,
     log_level_all     = 255,
     log_level_info    =   1,
     log_level_warning =   2,
     log_level_error   =   4,
     log_level_fatal   =   8,
     log_level_debug1  =  16,
     log_level_debug2  =  32,
     log_level_debug3  =  64,
     log_level_debug4  = 128,
   };
   
   typedef char eCAL_Logging_Filter;  //!< This type is to be used as a bitmask for the activated logging levels
