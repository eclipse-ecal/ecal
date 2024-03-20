
.. _program_listing_file_ecal_core_include_ecal_ecal_types.h:

Program Listing for File ecal_types.h
=====================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_types.h>` (``ecal/core/include/ecal/ecal_types.h``)

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
   #include <string>
   
   namespace eCAL
   {
       struct SDataTypeInformation
       {
         std::string name;          
         std::string encoding;      
         std::string descriptor;    
   
         bool operator==(const SDataTypeInformation& other) const
         {
           return name == other.name && encoding == other.encoding && descriptor == other.descriptor;
         }
   
         bool operator!=(const SDataTypeInformation& other) const
         {
           return !(*this == other);
         }
       };
   
       struct STopicInformation
       {
         SDataTypeInformation topic_type; 
   
         bool operator==(const STopicInformation& other) const
         {
           return topic_type == other.topic_type;
         }
   
         bool operator!=(const STopicInformation& other) const
         {
           return !(*this == other);
         }
       };
   
       struct SServiceMethodInformation
       {
         SDataTypeInformation request_type;   
         SDataTypeInformation response_type;  
   
         bool operator==(const SServiceMethodInformation& other) const
         {
           return request_type == other.request_type && response_type == other.response_type;
         }
   
         bool operator!=(const SServiceMethodInformation& other) const
         {
           return !(*this == other);
         }
       };
   
   }
