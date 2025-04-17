
.. _program_listing_file_ecal_core_include_ecal_deprecate.h:

Program Listing for File deprecate.h
====================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_deprecate.h>` (``ecal/core/include/ecal/deprecate.h``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   /* ========================= eCAL LICENSE =================================
    *
    * Copyright (C) 2016 - 2024 Continental Corporation
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
   
   #include <ecal/defs.h>
   
   //uncomment this line if you do want to get deprecation warnings inside eCAL core
   //#undef ECAL_NO_DEPRECATION_WARNINGS
   
   #if !defined(ECAL_NO_DEPRECATION_WARNINGS) && ECAL_VERSION_INTEGER >= ECAL_VERSION_CALCULATE(6, 0, 0)
   #define ECAL_DEPRECATE_SINCE_6_0(__message__) [[deprecated(__message__)]] 
   #else 
   #define ECAL_DEPRECATE_SINCE_6_0(__message__)                             
   #endif
