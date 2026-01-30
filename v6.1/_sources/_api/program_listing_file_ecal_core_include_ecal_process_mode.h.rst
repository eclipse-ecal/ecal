
.. _program_listing_file_ecal_core_include_ecal_process_mode.h:

Program Listing for File process_mode.h
=======================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_process_mode.h>` (``ecal/core/include/ecal/process_mode.h``)

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
   
   namespace eCAL
   {
     namespace Process
     {
       enum class eStartMode
       {
         normal = 0,     
         hidden = 1,     
         minimized = 2,  
         maximized = 3,  
       };
     }
   }
