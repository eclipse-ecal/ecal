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
 * @file clr_client.h
 * @brief Managed wrapper for the native CServiceClient API.
 *
 * This file provides the managed interface for eCAL service clients,
 * including both blocking calls and callback‐based calls.
 */

#include "service/clr_types.h"

#include <ecal/service/client.h>

using namespace System::Collections::Generic;

namespace Eclipse {
  namespace eCAL {
    namespace Core {

      /**
       * @brief Managed wrapper for the native CServiceClient.
       *
       * The ServiceClient class provides methods to interact with eCAL services
       * from a client perspective.
       */
      public ref class ServiceClient
      {
      public:
        /**
         * @brief Constructor.
         *
         * @param serviceName Unique service name.
         */
        ServiceClient(System::String^ serviceName);

        /**
         * @brief Constructor.
         *
         * @param serviceName           Unique service name.
         * @param methodInformationList List of service method information.
         */
        ServiceClient(System::String^ serviceName, ServiceMethodInformationList^ methodInformationList);

        /**
         * @brief Destructor.
         */
        ~ServiceClient();

        /**
         * @brief Finalizer.
         */
        !ServiceClient();

        /**
         * @brief Blocking call of a service method.
         *
         * @param methodName The method name.
         * @param request    The request as a string.
         * @param timeoutMs  Maximum time before the operation returns (in milliseconds, use DefaultTimeArgument for infinite timeout).
         *
         * @return List of service responses.
         */
        List<ServiceResponse^>^ CallWithResponse(System::String^ methodName,
                                                 array<System::Byte>^ request,
                                                 int timeoutMs);

        /**
         * @brief Blocking call with callback.
         *
         * Calls the service and invokes `callback` for each response.
         *
         * @param methodName The method name.
         * @param request    The request payload as a System::Byte array.
         * @param callback   Managed callback for each response.
         * @param timeoutMs  Maximum time before returning (ms, use DefaultTimeArgument for infinite).
         *
         * @return True if the call succeeded.
         */

        bool CallWithCallback(System::String^ methodName,
                              array<System::Byte>^ request,
                              ResponseCallback^ callback,
                              int timeoutMs);

        /**
         * @brief Retrieve the service name.
         *
         * @return The service name as a System::String^.
         */
        System::String^ GetServiceName();

        /**
         * @brief Retrieve the service ID.
         *
         * @return The service ID as a managed ServiceId.
         */
        ServiceId^ GetServiceId();

        /**
         * @brief Check connection state.
         *
         * @return True if at least one client instance is connected, false otherwise.
         */
        bool IsConnected();

        /**
         * @brief Default time argument for service calls.
         */
        literal long long DefaultTimeArgument = ::eCAL::CServiceClient::DEFAULT_TIME_ARGUMENT;

      private:
        /// Pointer to the native CServiceClient instance.
        ::eCAL::CServiceClient* m_native_service_client;

        /**
         * @brief Helper: wrap a managed ResponseCallback in a native std::function.
         */
        static std::function<void(const ::eCAL::SServiceResponse&)>
          CreateNativeResponseCallback(ResponseCallback^ managed);
      };

    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse
