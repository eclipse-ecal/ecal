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

/**
 * @file  ecal_clr_time.h
**/

using namespace System;

namespace Eclipse {
  namespace eCAL {
    namespace Time {

      /**
       * @brief Managed wrapper for the eCAL Time API.
       *
       * This class provides a managed interface to the native eCAL Time API,
       * enabling .NET applications to interact with eCAL time synchronization
       * and management functionality.
       */
      public ref class TimeAdapter
      {
      public:
        /**
         * @brief Gets the time synchronization interface name.
         *
         * @return The interface name as a System::String^.
         */
        static String^ GetName();

        /**
         * @brief Gets the current eCAL time in microseconds.
         *
         * @return The eCAL time in microseconds.
         */
        static long long GetMicroSeconds();

        /**
         * @brief Gets the current eCAL time in nanoseconds.
         *
         * @return The eCAL time in nanoseconds.
         */
        static long long GetNanoSeconds();

        /**
         * @brief Sets the eCAL time to the specified value in nanoseconds.
         *
         * This function only succeeds if the host is the time master.
         *
         * @param time New eCAL time in nanoseconds.
         * 
         * @return True if the time was successfully set, false otherwise.
         */
        static bool SetNanoSeconds(long long time);

        /**
         * @brief Checks whether the process is time synchronized.
         *
         * @return True if the process is synchronized, false otherwise.
         */
        static bool IsSynchronized();

        /**
         * @brief Checks whether the current host is the time master.
         *
         * @return True if the host is the time master, false otherwise.
         */
        static bool IsMaster();

        /**
         * @brief Suspends execution for the specified duration.
         *
         * The actual sleep duration may be affected by the current rate at which
         * eCAL time is progressing.
         *
         * @param duration_nsecs Duration to sleep in nanoseconds.
         */
        static void SleepForNanoseconds(long long duration_nsecs);

        /**
         * @brief Retrieves the current error code and status message.
         *
         * An error code of 0 indicates no error. The status message provides
         * additional context about any error that occurred.
         *
         * @param error [out] The error code.
         * @param statusMessage [out] A human-readable status message. May be empty if no message is provided.
         */
        static void GetStatus([System::Runtime::InteropServices::Out] int% error, [System::Runtime::InteropServices::Out] String^% statusMessage);
      };

    } // namespace Time
  } // namespace eCAL
} // namespace Eclipse
