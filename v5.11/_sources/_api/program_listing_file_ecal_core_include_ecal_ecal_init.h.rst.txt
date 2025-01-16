
.. _program_listing_file_ecal_core_include_ecal_ecal_init.h:

Program Listing for File ecal_init.h
====================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_init.h>` (``ecal/core/include/ecal/ecal_init.h``)

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
   
   namespace eCAL
   {
     namespace Init
     {
       static const unsigned int Publisher  =    0x001;
       static const unsigned int Subscriber =    0x002;
       static const unsigned int Service    =    0x004;
       static const unsigned int Monitoring =    0x008;
       static const unsigned int Logging    =    0x010;
       static const unsigned int TimeSync   =    0x020;
       static const unsigned int ProcessReg =    0x080;
   
       static const unsigned int All =           Publisher
                                               | Subscriber
                                               | Service
                                               | Monitoring
                                               | Logging
                                               | TimeSync
                                               | ProcessReg;
   
       static const unsigned int Default    =    Publisher
                                               | Subscriber
                                               | Service
                                               | Logging
                                               | TimeSync
                                               | ProcessReg;
   
       static const unsigned int None       =    0x000;
     }
   }
