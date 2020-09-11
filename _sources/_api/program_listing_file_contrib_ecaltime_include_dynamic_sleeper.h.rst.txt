
.. _program_listing_file_contrib_ecaltime_include_dynamic_sleeper.h:

Program Listing for File dynamic_sleeper.h
==========================================

|exhale_lsh| :ref:`Return to documentation for file <file_contrib_ecaltime_include_dynamic_sleeper.h>` (``contrib/ecaltime/include/dynamic_sleeper.h``)

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
   
   #include <chrono>
   #include <mutex>
   #include <condition_variable>
   #include <atomic>
   
   class CDynamicSleeper
   {
   public:
   
     CDynamicSleeper() :
       rate(1.0),
       lastSimTime(0)
     {
       lastSimTimeLocalTimestamp = std::chrono::steady_clock::now();
     }
   
     void sleepFor(long long durationNsecs_) {
       auto nowReal = std::chrono::steady_clock::now();
       long long nowSim;
       {
         std::unique_lock<std::mutex> modifyTimeLock(modifyTimeMutex);
         nowSim = (long long)((double)((nowReal - lastSimTimeLocalTimestamp).count()) * rate) + lastSimTime;
       }
       sleepUntil(nowSim + durationNsecs_);
     }
   
     void sleepUntil(long long sleepUntilTimeNsecs_) {
       auto      startRealtime = std::chrono::steady_clock::now();
       long long loopStartSimTime;
       double    originalRate;
       long long originalLastSimtime;
   
       {
         std::unique_lock<std::mutex> modifyTimeLock(modifyTimeMutex);
         loopStartSimTime = (long long)((double)((startRealtime - lastSimTimeLocalTimestamp).count()) * rate) + lastSimTime;
         originalRate = rate;
         originalLastSimtime = lastSimTime;
       }
   
       while (loopStartSimTime < sleepUntilTimeNsecs_) {
         std::unique_lock<std::mutex> waitLck(waitMutex);
         if (originalRate > 0) {
           std::chrono::duration<long long, std::nano> realTimeToSleep((long long)((double)(sleepUntilTimeNsecs_ - loopStartSimTime) / originalRate));
           if (waitCv.wait_for(waitLck, realTimeToSleep) == std::cv_status::timeout) {
             return;
           }
         }
         else {
           waitCv.wait(waitLck);
         }
   
         {
           std::unique_lock<std::mutex> modifyTimeLock(modifyTimeMutex);
           if (lastSimTime < originalLastSimtime) {
             return;
           }
           startRealtime = std::chrono::steady_clock::now();
           loopStartSimTime = (long long)((double)((startRealtime - lastSimTimeLocalTimestamp).count()) * rate) + lastSimTime;
           originalLastSimtime = lastSimTime;
           originalRate = rate;
         }
       }
     }
   
     void setRate(double rate_) {
       auto now = std::chrono::steady_clock::now();
       std::unique_lock<std::mutex> modifyTimeLock(modifyTimeMutex);
       long long passedSystemNsecs = (now - lastSimTimeLocalTimestamp).count();
       long long passedSimtimeNsecs = (long long)((double)passedSystemNsecs * rate);
       lastSimTimeLocalTimestamp = now;
       lastSimTime += passedSimtimeNsecs;
       rate = rate_;
       waitCv.notify_all();
     }
   
     void setTime(long long nsecs_) {
       auto now = std::chrono::steady_clock::now();
       std::unique_lock<std::mutex> modifyTimeLock(modifyTimeMutex);
       lastSimTimeLocalTimestamp = now;
       lastSimTime = nsecs_;
       waitCv.notify_all();
     }
   
     void setTimeAndRate(long long time_nsecs_, double rate_) {
       auto now = std::chrono::steady_clock::now();
       std::unique_lock<std::mutex> modifyTimeLock(modifyTimeMutex);
       lastSimTimeLocalTimestamp = now;
       lastSimTime = time_nsecs_;
       rate = rate_;
       waitCv.notify_all();
     }
   
     long long getCurrentInternalSimTime() {
       std::unique_lock<std::mutex> modifyTimeLock(modifyTimeMutex);
       auto now = std::chrono::steady_clock::now();
       return (long long)((double)((now - lastSimTimeLocalTimestamp).count()) * rate) + lastSimTime;
     }
   
     ~CDynamicSleeper() {}
   
   private:
     std::mutex waitMutex;             
     std::condition_variable waitCv;   
     std::mutex modifyTimeMutex;       
     double rate;                      
     long long lastSimTime;            
     std::chrono::time_point<std::chrono::steady_clock> lastSimTimeLocalTimestamp; 
   };
