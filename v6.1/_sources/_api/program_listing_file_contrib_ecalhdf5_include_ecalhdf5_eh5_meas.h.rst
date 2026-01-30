
.. _program_listing_file_contrib_ecalhdf5_include_ecalhdf5_eh5_meas.h:

Program Listing for File eh5_meas.h
===================================

|exhale_lsh| :ref:`Return to documentation for file <file_contrib_ecalhdf5_include_ecalhdf5_eh5_meas.h>` (``contrib/ecalhdf5/include/ecalhdf5/eh5_meas.h``)

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
   
   #include <cstdint>
   #include <functional>
   #include <set>
   #include <string>
   #include <memory>
   
   #include "eh5_types.h"
   
   #if defined(ECAL_EH5_NO_DEPRECATION_WARNINGS)
   #define ECAL_EH5_DEPRECATE(__message__)                             
   #else 
   #define ECAL_EH5_DEPRECATE(__message__) [[deprecated(__message__)]] 
   #endif
   
   #include "eh5_meas_api_v2.h"
   #include "eh5_meas_api_v3.h"
