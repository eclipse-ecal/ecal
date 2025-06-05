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

#include "ecal/log_level.h"

namespace Eclipse {
  namespace eCAL {
    namespace Core {
      /**
       * @brief Managed enum for eCAL log levels, matching native eCAL::Logging::eLogLevel.
       */
      [System::Flags]
      public enum class eLoggingLogLevel : System::Byte
      {
        None    = ::eCAL::Logging::log_level_none,
        All     = ::eCAL::Logging::log_level_all,
        Info    = ::eCAL::Logging::log_level_info,
        Warning = ::eCAL::Logging::log_level_warning,
        Error   = ::eCAL::Logging::log_level_error,
        Fatal   = ::eCAL::Logging::log_level_fatal,
        Debug1  = ::eCAL::Logging::log_level_debug1,
        Debug2  = ::eCAL::Logging::log_level_debug2,
        Debug3  = ::eCAL::Logging::log_level_debug3,
        Debug4  = ::eCAL::Logging::log_level_debug4
      };
     
      using LoggingFilter = System::Byte;
     
       /**
        * @brief Helper class for converting between managed LogLevel and native eLogLevel.
        */
      public ref class eLoggingLogLevelHelper abstract sealed
      {
      public:
        static eLoggingLogLevel FromNative(::eCAL::Logging::eLogLevel nativeLevel)
        {
          return static_cast<eLoggingLogLevel>(nativeLevel);
        }
       
        static ::eCAL::Logging::eLogLevel ToNative(eLoggingLogLevel managedLevel)
        {
          return static_cast<::eCAL::Logging::eLogLevel>(managedLevel);
        }
        
        static LoggingFilter FromNative(::eCAL::Logging::Filter nativeFilter)
        {
          return static_cast<LoggingFilter>(nativeFilter);
        }
        
        static ::eCAL::Logging::Filter ToNative(LoggingFilter managedFilter)
        {
          return static_cast<::eCAL::Logging::Filter>(managedFilter);
        }
      };
      
    }
  }
}
