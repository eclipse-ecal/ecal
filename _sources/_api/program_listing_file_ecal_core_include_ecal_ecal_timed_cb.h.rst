
.. _program_listing_file_ecal_core_include_ecal_ecal_timed_cb.h:

Program Listing for File ecal_timed_cb.h
========================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_timed_cb.h>` (``ecal/core/include/ecal/ecal_timed_cb.h``)

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
   
   #include <atomic>
   #include <chrono>
   #include <functional>
   #include <thread>
   #include <assert.h>
   
   #include <ecal/ecal_deprecate.h>
   
   #include "ecal_process.h"
   
   namespace eCAL
   {
     class CTimedCB;
   
     typedef std::function<void(void)> TimerCallbackT;
   
     class CTimedCB
     {
     public:
       ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
       CTimedCB() : m_stop(false), m_running(false) {}
   
       ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
       CTimedCB(int timeout_, TimerCallbackT callback_, int delay_ = 0) : m_stop(false), m_running(false) { Start(timeout_, callback_, delay_); }
   
       virtual ~CTimedCB() { Stop(); }
   
       CTimedCB(const CTimedCB&) = delete;
       CTimedCB& operator=(const CTimedCB&) = delete;
       CTimedCB(CTimedCB&& rhs) = delete;
       CTimedCB& operator=(CTimedCB&& rhs) = delete;
   
       ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
       bool Start(const int timeout_, TimerCallbackT callback_, const int delay_ = 0)
       {
         assert(m_running == false);
         if (m_running)    return(false);
         if (timeout_ < 0) return(false);
         m_stop = false;
         m_thread = std::thread(&CTimedCB::Thread, this, callback_, timeout_, delay_);
         m_running = true;
         return(true);
       }
   
       ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
       bool Stop()
       {
         if (!m_running) return(false);
         m_stop = true;
         // Wait for the callback thread to finish
         if (m_thread.joinable()) {
           m_thread.join();
         }
         m_running = false;
         return(true);
       }
   
     private:
       void Thread(TimerCallbackT callback_, int timeout_, int delay_)
       {
         assert(callback_ != nullptr);
         if (callback_ == nullptr) return;
         if (delay_ > 0) eCAL::Process::SleepFor(std::chrono::milliseconds(delay_));
         while (!m_stop)
         {
           auto start = std::chrono::steady_clock::now();
           (callback_)();
           if (timeout_ > 0)
           {
             auto now = std::chrono::steady_clock::now();
             auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
             auto sleep_duration = timeout_ - elapsed_time;
             eCAL::Process::SleepFor(std::chrono::milliseconds(sleep_duration));
           }
         }
         m_stop = false;
       }
   
       std::atomic<bool> m_stop;
       std::atomic<bool> m_running;
       std::thread       m_thread;
     };
   }
