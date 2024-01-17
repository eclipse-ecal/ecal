
.. _program_listing_file_contrib_ecalhdf5_include_ecalhdf5_eh5_types.h:

Program Listing for File eh5_types.h
====================================

|exhale_lsh| :ref:`Return to documentation for file <file_contrib_ecalhdf5_include_ecalhdf5_eh5_types.h>` (``contrib/ecalhdf5/include/ecalhdf5/eh5_types.h``)

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
   
   #include <set>
   #include <string>
   #include <vector>
   
   #include <ecal/measurement/base/types.h>
   
   namespace eCAL
   {
     namespace eh5
     {
       const std::string kChnNameAttribTitle ("Channel Name");
       const std::string kChnDescAttrTitle   ("Channel Description");
       const std::string kChnTypeAttrTitle   ("Channel Type");
       const std::string kFileVerAttrTitle   ("Version");
       const std::string kTimestampAttrTitle ("Timestamps");
       const std::string kChnAttrTitle       ("Channels");
   
       // Remove @eCAL6 -> backwards compatibility with old interface!
       using SEntryInfo = eCAL::experimental::measurement::base::EntryInfo;
       using EntryInfoSet = eCAL::experimental::measurement::base::EntryInfoSet;
       using EntryInfoVect = eCAL::experimental::measurement::base::EntryInfoVect;
       using eAccessType = eCAL::experimental::measurement::base::AccessType;
       using eCAL::experimental::measurement::base::RDONLY;
       using eCAL::experimental::measurement::base::CREATE;
     }  // namespace eh5
   }  // namespace eCAL
