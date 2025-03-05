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
 * @file  ecal_clr_monitoring.h
**/

using namespace System;

namespace Continental {
  namespace eCAL {
    namespace Core {

      // Managed representation of the native monitoring entities bitmask.
      [Flags]
      public enum class MonitoringEntity : unsigned int
      {
        None       = 0x000,
        Publisher  = 0x001,
        Subscriber = 0x002,
        Server     = 0x004,
        Client     = 0x008,
        Process    = 0x010,
        Host       = 0x020,
        All        = Publisher | Subscriber | Server | Client | Process | Host
      };

      /**
       * @brief eCAL monitoring class.
       **/
      public ref class Monitoring
      {
      public:
        /**
         * @brief Initialize eCAL Monitoring API.
         **/
        static void Initialize();

        /**
         * @brief Finalize eCAL Monitoring API.
         **/
        static void Terminate();

        /**
         * @brief Get host, process and topic monitoring as raw message bytes.
         *
         * @param entities Specifies which entities to include (default is All).
         * 
         * @return The monitoring message as a byte array.
         **/
        static array<Byte>^ GetMonitoring(MonitoringEntity entities);

        /**
         * @brief Get global log message as raw message bytes.
         *
         * @return The logging message as a byte array.
         **/
        static array<Byte>^ GetLogging();
      };

    } // namespace Core
  } // namespace eCAL
} // namespace Continental
