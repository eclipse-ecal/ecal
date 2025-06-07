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
 * @file  clr_process.h
**/

#include <string>
#include <ecal/process.h>
#include <msclr/marshal_cppstd.h>

#include "clr_process_severity.h"
#include "clr_types.h"

namespace Eclipse {
  namespace eCAL {
    namespace Core {
        
      /**
       * @brief Managed wrapper for eCAL::Process static API.
       */
      public ref class Process abstract sealed
      {
      public:
        /**
         * @brief  Get current unit name (defined via eCAL::Initialize). 
         *
         * @return  Unit name or empty string if failed. 
         */
        static System::String^ GetUnitName();

        /**
         * @brief  Set process state info. 
         *
         * @param severity  Severity level.
         * @param level     Severity level.
         * @param info      Info message.
         */
        static void SetState(eProcessSeverity severity, eProcessSeverityLevel level, System::String^ info);
      };

    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse