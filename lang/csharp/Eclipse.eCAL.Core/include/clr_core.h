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
 * @file  clr_core.h
**/

#include "clr_init.h"
#include "clr_types.h"
#include "config/clr_configuration.h"

namespace Eclipse {
  namespace eCAL {
    namespace Core {

      /**
       * @brief eCAL core class.
      **/
      public ref class Core
      {
      public:
        /**
         * @brief Retrieve eCAL release version string.
         *
         * @return  The version string.
        **/
        static System::String^ GetVersionString();

        /**
         * @brief Retrieve eCAL release date string.
         *
         * @return  The date string.
        **/
        static System::String^ GetVersionDateString();

        /**
        * @brief Initialize eCAL API.
        *
        * @param unitName The instance unit (node) name.
        **/
        static void Initialize(System::String^ unitName);

        /**
        * @brief Initialize eCAL API.
        *
        * @param unitName The instance unit (node) name.
        * @param componentFlags The components to initialize.
        **/
        static void Initialize(System::String^ unitName, Init::Flags componentFlags);
        
        /**
        * @brief Initialize eCAL API.
        *
        * @param config The configuration to use.
        * @param unitName The instance unit (node) name.
        * @param componentFlags The components to initialize.
        **/
        static void Initialize(Config::Configuration^ config, System::String^ unitName, Init::Flags componentFlags);

        /**
        * @brief Finalize eCAL API.
        **/
        static void Terminate();

        /**
         * @brief Check eCAL initialize state.
         *
         * @return True if eCAL is initialized.
        **/
        static bool IsInitialized();

        /**
         * @brief Return the eCAL process state.
         *
         * @return  True if eCAL is in proper state.
        **/
        static bool Ok();
      };

    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse
