
.. _program_listing_file_ecal_core_include_ecal_ecal_payload_writer.h:

Program Listing for File ecal_payload_writer.h
==============================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_payload_writer.h>` (``ecal/core/include/ecal/ecal_payload_writer.h``)

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
   
   #include <cstddef>
   
   namespace eCAL
   {
     class CPayloadWriter
     {
     public:
       CPayloadWriter() = default;
   
       virtual ~CPayloadWriter() = default;
   
       CPayloadWriter(const CPayloadWriter&) = default;
   
       CPayloadWriter(CPayloadWriter&&) = default;
   
       CPayloadWriter& operator=(const CPayloadWriter&) = default;
   
       CPayloadWriter& operator=(CPayloadWriter&&) = default;
   
       virtual bool WriteFull(void* buffer_, size_t size_) = 0;
   
       virtual bool WriteModified(void* buffer_, size_t size_) { return WriteFull(buffer_, size_); };
   
       virtual size_t GetSize() = 0;
     };
   
   } // namespace eCAL
