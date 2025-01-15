
.. _program_listing_file_ecal_core_include_ecal_ecal_event.h:

Program Listing for File ecal_event.h
=====================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_event.h>` (``ecal/core/include/ecal/ecal_event.h``)

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
   #include <ecal/ecal_eventhandle.h>
   #include <ecal/ecal_deprecate.h>
   
   #include <string>
   
   namespace eCAL
   {
     ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
     ECAL_API bool gOpenEvent(eCAL::EventHandleT* event_, const std::string& event_name_ = "");
   
     ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
     ECAL_API bool gCloseEvent(const EventHandleT& event_);
   
     ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
     ECAL_API bool gSetEvent(const EventHandleT& event_);
   
     ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
     ECAL_API bool gWaitForEvent(const EventHandleT& event_, long timeout_);
   
     ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
     ECAL_API bool gInvalidateEvent(EventHandleT* event_);
   
     ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
     ECAL_API bool gEventIsValid(const EventHandleT& event_);
   }
