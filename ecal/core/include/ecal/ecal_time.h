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

/**
 * @file   ecal_time.h
 * @brief  eCAL time interface
**/

#pragma once

#include <ecal/ecal_os.h>
#include <string>
#include <chrono>

namespace eCAL
{
  namespace Time
  {
    /**
     * @brief  Get interface name.
     *
     * @return  Get time sync interface name.
    **/
    ECAL_API std::string GetName();

    /**
     * @brief  Get the current eCAL Time in microseconds.
     * 
     * The timebase depends on the loaded ecaltime plugin.
     *
     * @return  eCAL Time in microseconds
    **/
    ECAL_API long long GetMicroSeconds();

    /**
     * @brief  Get the current eCAL Time in nanoseconds.
     *
     * The timebase depends on the loaded ecaltime plugin.
     *
     * @return  eCAL Time in nanoseconds
    **/
    ECAL_API long long GetNanoSeconds();

    /**
     * @brief  Sets the eCAL Time to the given time in nanoseconds.
     *
     * This function only succeeds, if the host is the time master.
     *
     * @param time_  new eCAL Time in nanoseconds
     *
     * @return  True if succeeded false otherwise.
    **/
    ECAL_API bool SetNanoSeconds(long long time_);

    /**
     * @brief  Returns time synchronization state. 
     *
     * @return  True if process is time synchronized. 
    **/
    ECAL_API bool IsSynchronized();

    /**
     * @brief  Checks if the current host is the time master.
     *
     * @return  True if host is time master, false otherwise
    **/
    ECAL_API bool IsMaster();
    
    /**
     * @brief Blocks for the given amount of nanoseconds.
     *
     * The actual amount of (real-) time is influenced by the current rate at
     * which the time is proceeding.
     * It is not guaranteed, that the precision of this function actually  is in
     * nanoseconds. Limitations of the operating system might reduce the accuracy.
     *
     * @param duration_nsecs_ the duration in nanoseconds
    **/
    ECAL_API void SleepForNanoseconds(long long duration_nsecs_);

    /**
     * @brief Get the current error code and status message
     *
     * An error code of 0 is considered to be OK. Any other error code is
     * considered to indicate a problem. Time Adapters may use a set of error
     * codes to indicate specific problems.
     * The Status message may be a nullpointer.
     *
     * @param error_ [out]          the error code
     * @param status_message_ [out] a human-readable status message. May be nullptr.
    **/
    ECAL_API void GetStatus(int& error_, std::string* status_message_);

    ///////////////////////////////////////////////////////////////////////////
    ////// C++ API with std::chrono                                      //////
    ///////////////////////////////////////////////////////////////////////////

    /**
     * @brief blocks for the given duration
     *
     * The actual amount of (real-) time is influenced by the current rate at
     * which the time is proceeding.
     * The precision of the delay is not guaranteed. It might depend on the
     * actual eCAL Clock that is selected and limitations of the operating
     * system.
     *
     * Example usage:
     *    eCAL::Time::sleep_for(std::chrono::seconds(10))
     *
     * @param _Rel_time the relative duration to sleep
    **/
    template<class _Rep,
             class _Period> inline
      void sleep_for(const std::chrono::duration<_Rep, _Period>& _Rel_time) {
               long long duration_nsecs = (std::chrono::duration_cast<std::chrono::nanoseconds>(_Rel_time)).count();
               SleepForNanoseconds(duration_nsecs);
    }

    /* @cond */
    /**
     * @brief The eCAL Clock which bases it's time on the loaded eCAL Time Adapter.
    **/
    struct ecal_clock
    {
      typedef std::chrono::nanoseconds            duration;
      typedef duration::rep                       rep;
      typedef duration::period                    period;
      typedef std::chrono::time_point<ecal_clock> time_point;
      static const bool                           is_steady = false;

      /**
       * @brief Returns a time point representing the current point in eCAL time
       * @return the current eCAL Time
      **/
      static time_point now()
      {
        return time_point (duration(GetNanoSeconds()));
      }
    };
    /* @endcond */
  }
  /** @example time.cpp
  * This is an example on how to use the eCAL Time Interface.
  */
}
