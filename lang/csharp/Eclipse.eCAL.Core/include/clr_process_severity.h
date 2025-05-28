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
 * @file  clr_process_severity.h
**/

#include <ecal/process_severity.h>

namespace Eclipse {
  namespace eCAL {
    namespace Core {

      /**
       * @brief Managed wrapper for the native eCAL::Process::eSeverity enum.
       */
      public enum class eProcessSeverity
      {
        Unknown  = ::eCAL::Process::eSeverity::unknown,
        Healthy  = ::eCAL::Process::eSeverity::healthy,
        Warning  = ::eCAL::Process::eSeverity::warning,
        Critical = ::eCAL::Process::eSeverity::critical,
        Failed   = ::eCAL::Process::eSeverity::failed
      };

      /**
       * @brief Managed wrapper for the native eCAL::Process::eSeverityLevel enum.
       */
      public enum class eProcessSeverityLevel
      {
        Level1 = ::eCAL::Process::eSeverityLevel::level1,
        Level2 = ::eCAL::Process::eSeverityLevel::level2,
        Level3 = ::eCAL::Process::eSeverityLevel::level3,
        Level4 = ::eCAL::Process::eSeverityLevel::level4,
        Level5 = ::eCAL::Process::eSeverityLevel::level5
      };

      /**
       * @brief Helper class for converting between managed and native enums.
       */
      public ref class ProcessSeverityHelper abstract sealed
      {
      public:
        static eProcessSeverity FromNative(::eCAL::Process::eSeverity nativeSeverity)
        {
          return static_cast<eProcessSeverity>(nativeSeverity);
        }
        static ::eCAL::Process::eSeverity ToNative(eProcessSeverity managedSeverity)
        {
          return static_cast<::eCAL::Process::eSeverity>(managedSeverity);
        }
      };

      public ref class ProcessSeverityLevelHelper abstract sealed
      {
      public:
        static eProcessSeverityLevel FromNative(::eCAL::Process::eSeverityLevel nativeLevel)
        {
          return static_cast<eProcessSeverityLevel>(nativeLevel);
        }
        static ::eCAL::Process::eSeverityLevel ToNative(eProcessSeverityLevel managedLevel)
        {
          return static_cast<::eCAL::Process::eSeverityLevel>(managedLevel);
        }
      };

    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse