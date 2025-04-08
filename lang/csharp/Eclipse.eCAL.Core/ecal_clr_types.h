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
 * @file ecal_clr_types.h
 *
 * @brief Managed type definitions for eCAL topics.
 *
 * This file contains managed wrappers for native type definitions associated
 * with topics (e.g. version information, datatype information, and entity IDs).
 */

using namespace System;

namespace Eclipse {
  namespace eCAL {
    namespace Core {

      /**
       * @brief Managed wrapper for the native SVersion structure.
       *
       * Represents the version of eCAL.
       */
      public ref class Version {
      public:
        /**
         * @brief Major version number.
         */
        property int Major;
        /**
         * @brief Minor version number.
         */
        property int Minor;
        /**
         * @brief Patch version number.
         */
        property int Patch;

        /**
         * @brief Default constructor.
         */
        Version() {}

        /**
         * @brief Parameterized constructor.
         * @param major Major version.
         * @param minor Minor version.
         * @param patch Patch version.
         */
        Version(int major, int minor, int patch) {
          Major = major;
          Minor = minor;
          Patch = patch;
        }
      };

      /**
       * @brief Managed enumeration for eCAL initialization flags.
       *
       * This enumeration mirrors the native eCAL::Init constants.
       */
      [Flags]
      public enum class Init : unsigned int {
        None       = 0x000,
        Publisher  = 0x001,
        Subscriber = 0x002,
        Service    = 0x004,
        Monitoring = 0x008,
        Logging    = 0x010,
        TimeSync   = 0x020,
        Default    = Publisher | Subscriber | Service | Logging | TimeSync,
        All        = Publisher | Subscriber | Service | Monitoring | Logging | TimeSync
      };

      /**
       * @brief Managed wrapper for the native SDataTypeInformation structure.
       *
       * Contains information about a topic's data type, including its name, encoding, and descriptor.
       */
      public ref class DataTypeInformation {
      public:
        /**
         * @brief Gets or sets the data type name.
         */
        property String^ Name;
        /**
         * @brief Gets or sets the encoding of the data type (e.g. protobuf, flatbuffers).
         */
        property String^ Encoding;
        /**
         * @brief Gets or sets the descriptor information of the data type.
         */
        property array<Byte>^ Descriptor;

        /**
         * @brief Default constructor.
         */
        DataTypeInformation() {}

        /**
         * @brief Parameterized constructor.
         * @param name Data type name.
         * @param encoding Encoding of the data type.
         * @param descriptor Descriptor information.
         */
        DataTypeInformation(String^ name, String^ encoding, array<Byte>^ descriptor)
        {
          Name = name;
          Encoding = encoding;
          Descriptor = descriptor;
        }
      };

      /**
       * @brief Managed wrapper for the native SEntityId structure.
       *
       * Represents a unique identifier for an entity (e.g. publisher or subscriber).
       */
      public ref class EntityId {
      public:
        /**
         * @brief Gets or sets the unique entity ID.
         */
        property UInt64 Id;
        /**
         * @brief Gets or sets the process ID that produced the sample.
         */
        property int ProcessId;
        /**
         * @brief Gets or sets the host name that produced the sample.
         */
        property String^ HostName;

        /**
         * @brief Default constructor.
         */
        EntityId() {}

        /**
         * @brief Parameterized constructor.
         * @param id Unique entity ID.
         * @param processId Process ID.
         * @param hostName Host name.
         */
        EntityId(UInt64 id, int processId, String^ hostName)
        {
          Id = id;
          ProcessId = processId;
          HostName = hostName;
        }
      };

    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse
