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

/**
 * @file  ecal_clr_receivecallbackdata.h
**/

#pragma once

using namespace System;

namespace Continental
{
  namespace eCAL
  {
    namespace Core
    {
      /**
       * @brief Managed wrapper for the native SReceiveCallbackData structure.
       */
      public ref class ReceiveCallbackData
      {
      public:
        /// @brief Gets the payload buffer as a byte array.
        property array<Byte>^ Buffer;

        /// @brief Gets the publisher send timestamp in microseconds.
        property long long SendTimestamp;

        /// @brief Gets the publisher send clock.
        property long long SendClock;
      };
    }
  }
}
