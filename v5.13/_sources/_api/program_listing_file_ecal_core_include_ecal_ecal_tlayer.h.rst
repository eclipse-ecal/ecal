
.. _program_listing_file_ecal_core_include_ecal_ecal_tlayer.h:

Program Listing for File ecal_tlayer.h
======================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_tlayer.h>` (``ecal/core/include/ecal/ecal_tlayer.h``)

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
   
   namespace eCAL
   {
     namespace TLayer
     {
       enum eTransportLayer
       {
         tlayer_none       = 0,
         tlayer_udp_mc     = 1,
         tlayer_shm        = 4,
         tlayer_tcp        = 5,
         tlayer_inproc     = 42,  
         tlayer_all        = 255
       };
   
       enum eSendMode
       {
         smode_none = -1,
         smode_off  = 0,
         smode_on,
         smode_auto
       };
   
       struct ECAL_API STLayer
       {
         STLayer()
         {
           sm_udp_mc  = smode_none;
           sm_shm     = smode_none;
           sm_inproc  = smode_none;
           sm_tcp     = smode_none;
         }
         eSendMode sm_udp_mc;  
         eSendMode sm_shm;     
         eSendMode sm_inproc;  
         eSendMode sm_tcp;     
       };
     }
   }
