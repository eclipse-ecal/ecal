
.. _program_listing_file_ecal_core_include_ecal_ecal_qos.h:

Program Listing for File ecal_qos.h
===================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_qos.h>` (``ecal/core/include/ecal/ecal_qos.h``)

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
   
   #include "ecal_os.h"
   
   namespace eCAL
   {
     namespace QOS
     {
       enum eQOSPolicy_HistoryKind
       {
         keep_last_history_qos,          
         keep_all_history_qos,           
       };
   
       enum eQOSPolicy_Reliability
       {
         best_effort_reliability_qos,    
         reliable_reliability_qos,       
       };
   
       struct ECAL_API SWriterQOS
       {
         SWriterQOS()
         {
           history_kind       = keep_last_history_qos;
           history_kind_depth = 8;
           reliability        = reliable_reliability_qos;
         }
         eQOSPolicy_HistoryKind  history_kind;              
         int                     history_kind_depth;        
         eQOSPolicy_Reliability  reliability;               
       };
   
       struct ECAL_API SReaderQOS
       {
         SReaderQOS()
         {
           history_kind       = keep_last_history_qos;
           history_kind_depth = 8;
           reliability        = best_effort_reliability_qos;
         }
         eQOSPolicy_HistoryKind  history_kind;              
         int                     history_kind_depth;        
         eQOSPolicy_Reliability  reliability;               
       };
     }
   }
