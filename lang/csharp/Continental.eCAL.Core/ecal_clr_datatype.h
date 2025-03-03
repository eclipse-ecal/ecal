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
 * @file  ecal_clr_datatype.h
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
       * @brief Managed wrapper for the native SDataTypeInformation structure.
       *
       * This class contains information about a topic's data type, including its name,
       * encoding (e.g. protobuf, flatbuffers), and descriptor.
       */
      public ref class DataTypeInformation
      {
      public:
        /**
         * @brief Gets or sets the data type name.
         **/
        property String^ Name;

        /**
         * @brief Gets or sets the encoding of the data type.
         **/
        property String^ Encoding;

        /**
         * @brief Gets or sets the descriptor of the data type.
         **/
        property array<Byte>^ Descriptor;

        /**
         * @brief Default constructor.
         **/
        DataTypeInformation() {}

        /**
         * @brief Initializes a new instance with the specified values.
         * 
         * @param name Data type name.
         * @param encoding Encoding of the data type.
         * @param descriptor Descriptor information.
         **/
        DataTypeInformation(String^ name, String^ encoding, array<Byte>^ descriptor)
        {
          Name = name;
          Encoding = encoding;
          Descriptor = descriptor;
        }
      };
    }
  }
}
