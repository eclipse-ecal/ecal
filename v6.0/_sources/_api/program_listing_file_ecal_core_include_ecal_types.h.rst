
.. _program_listing_file_ecal_core_include_ecal_types.h:

Program Listing for File types.h
================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_types.h>` (``ecal/core/include/ecal/types.h``)

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
   #include <cstdint>
   #include <iostream>
   #include <string>
   #include <tuple>
   
   namespace eCAL
   {
     struct SVersion
     {
       const int major; 
       const int minor; 
       const int patch; 
     };
   
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
   
       bool operator<(const SDataTypeInformation& rhs) const
       {
           return std::tie(name, encoding, descriptor) < std::tie(rhs.name, rhs.encoding, rhs.descriptor);
       }
   
       void clear()
       {
         name.clear();
         encoding.clear();
         descriptor.clear();
       }
     };
   
     using EntityIdT = uint64_t;
   
     struct SEntityId
     {
       EntityIdT    entity_id  = 0;    // unique id within that process (it should already be unique within the whole system)
       int32_t      process_id = 0;    // process id which produced the sample
       std::string  host_name;         // host which produced the sample
   
       bool operator==(const SEntityId& other) const {
         return entity_id == other.entity_id;
       }
   
       bool operator<(const SEntityId& other) const
       {
         return entity_id < other.entity_id;
       }
     };
   
     // Overload the << operator for SEntityId
     inline std::ostream& operator<<(std::ostream& os, const SEntityId& id)
     {
       os << "SEntityId(entity_id: " << id.entity_id
         << ", process_id: " << id.process_id
         << ", host_name: " << id.host_name << ")";
       return os;
     }
   }
   
   namespace std
   {
     template<>
     class hash<eCAL::SDataTypeInformation> {
     public:
       size_t operator()(const eCAL::SDataTypeInformation& datatype_info) const
       {
         const std::size_t h1 = std::hash<std::string>{}(datatype_info.name);
         const std::size_t h2 = std::hash<std::string>{}(datatype_info.encoding);
         const std::size_t h3 = std::hash<std::string>{}(datatype_info.descriptor);
         return h1 ^ (h2 << 1) ^ (h3 << 2); // basic combination
       }
     };
   }
