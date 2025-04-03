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
 * @file  ecal_clr_core.h
**/

using namespace System;

namespace Continental {
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
        static System::String^ GetVersion();

        /**
         * @brief Retrieve eCAL release date string.
         *
         * @return  The date string.
        **/
        static System::String^ GetDate();

        /**
        * @brief Initialize eCAL API.
        *
        * @param unit_name_ The instance unit (node) name.
        **/
        static void Initialize(System::String^ unit_name_);

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
} // namespace Continental
