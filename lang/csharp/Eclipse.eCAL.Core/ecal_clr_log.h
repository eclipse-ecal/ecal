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
 * @file  ecal_clr_log.h
**/

using namespace System;

namespace Eclipse {
  namespace eCAL {
    namespace Core {

      // Expose a managed log level enum with values matching the native API.
      [System::Flags]
      public enum class LogLevel : unsigned char
      {
        None    = 0,
        Info    = 1,    // native: log_level_info
        Warning = 2,    // native: log_level_warning
        Error   = 4,    // native: log_level_error
        Fatal   = 8,    // native: log_level_fatal
        All     = 255   // native: log_level_all
      };

      /**
       * @brief eCAL logging class.
      **/
      public ref class Logging
      {
      public:
        /**
         * @brief Log a message with a specified log level.
         *
         * @param level    The log level.
         * @param message  The message string to log.
        **/
        static void Log(LogLevel level, System::String^ message);

        /**
         * @brief Get global log message as raw message bytes.
         *
         * @return The logging message as a byte array.
         **/
        static array<Byte>^ GetLogging();
      };

    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse
