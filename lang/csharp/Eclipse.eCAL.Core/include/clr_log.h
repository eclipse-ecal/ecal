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
 * @file  clr_log.h
**/

#include "clr_log_level.h"
#include "types/clr_logging.h"

namespace Eclipse {
  namespace eCAL {
    namespace Core {
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
        static void Log(eLoggingLogLevel level, System::String^ message);

        /**
         * @brief Get global log message as raw message bytes.
         *
         * @return The logging message as a byte array.
         *         Can return nullptr if the call was not successful.
         **/
        static array<System::Byte>^ GetSerializedLogging();

        /**
         * @brief Get a list of Log messages.
         *
         * @return The logging message as SLogging struct.
         *         Can return nullptr if the call was not successful.
         **/
        static SLogging^ GetLogging();
      };

    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse
