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
 * @file  ecal_clr_servicemethodinformation.h
**/

#pragma once

#include "ecal_clr_datatype.h"

using namespace System;

namespace Continental
{
  namespace eCAL
  {
    namespace Core
    {
      /**
       * @brief Managed wrapper for the native SServiceMethodInformation structure.
       *
       * This class encapsulates the service method name, the request type, and the response type
       * information, and is used to register method callbacks.
       */
      public ref class ServiceMethodInformation
      {
      public:
        /// <summary>
        /// Gets or sets the name of the service method.
        /// </summary>
        property String^ MethodName;

        /// <summary>
        /// Gets or sets the request type information.
        /// </summary>
        property DataTypeInformation^ RequestType;

        /// <summary>
        /// Gets or sets the response type information.
        /// </summary>
        property DataTypeInformation^ ResponseType;

        /**
         * @brief Default constructor.
         */
        ServiceMethodInformation() {}

        /**
         * @brief Parameterized constructor.
         *
         * @param methodName The name of the service method.
         * @param requestType The request type information.
         * @param responseType The response type information.
         */
        ServiceMethodInformation(String^ methodName, DataTypeInformation^ requestType, DataTypeInformation^ responseType)
        {
          MethodName = methodName;
          RequestType = requestType;
          ResponseType = responseType;
        }
      };
    }
  }
}
