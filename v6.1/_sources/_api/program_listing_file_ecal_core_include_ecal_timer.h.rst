
.. _program_listing_file_ecal_core_include_ecal_timer.h:

Program Listing for File timer.h
================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_timer.h>` (``ecal/core/include/ecal/timer.h``)

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
   
   #include <ecal/os.h>
   #include <functional>
   #include <memory>
   
   namespace eCAL
   {
     class CTimerImpl;
     using TimerCallbackT = std::function<void ()>;
   
     class CTimer 
     {
     public:
       ECAL_API CTimer();
   
       ECAL_API CTimer(int timeout_, const TimerCallbackT& callback_, int delay_ = 0);
   
       ECAL_API virtual ~CTimer();
   
       // Object not copyable / moveable
       CTimer(const CTimer&) = delete;
       CTimer& operator=(const CTimer&) = delete;
       CTimer(CTimer&& rhs) = delete;
       CTimer& operator=(CTimer&& rhs) = delete;
   
       ECAL_API bool Start(int timeout_, const TimerCallbackT& callback_, int delay_ = 0);
   
       ECAL_API bool Stop();
   
     protected:
       // class members
       std::unique_ptr<CTimerImpl> m_timer;
     };
   }
   
