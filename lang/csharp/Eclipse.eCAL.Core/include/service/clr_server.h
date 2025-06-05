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
 * @file  clr_server.h
**/

#include "service/clr_types.h"

#include <ecal/service/server.h>

using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;

namespace Eclipse {
  namespace eCAL {
    namespace Core {

      /**
       * @brief eCAL server class.
       *
       * The ServiceServer class is used to answer calls from matching eCAL clients.
       */
      public ref class ServiceServer
      {
      public:
        /**
         * @brief Constructor.
         *
         * @param serverName Unique server name.
         */
        ServiceServer(System::String^ serverName);

        /**
         * @brief Destructor.
         */
        ~ServiceServer();

        /**
         * @brief Finalizer.
         */
        !ServiceServer();

        /**
         * @brief Delegate definition for method callbacks.
         *
         * @param methodInfo The service method information.
         * @param request The request data as a byte array.
         * 
         * @return The response data as a byte array.
         */
        delegate array<System::Byte>^ MethodCallback(ServiceMethodInformation^ methodInfo, array<System::Byte>^ request);

        /**
         * @brief Add a callback function for an incoming service method call.
         *
         * @param methodInfo The service method information.
         * @param callback The callback function.
         * 
         * @return True if succeeded, false otherwise.
         */
        bool SetMethodCallback(ServiceMethodInformation^ methodInfo, MethodCallback^ callback);

        /**
         * @brief Remove the callback function for the specified service method.
         *
         * @param methodName The name of the service method.
         * 
         * @return True if succeeded, false otherwise.
         */
        bool RemoveMethodCallback(System::String^ methodName);

      private:
        /**
         * @brief Pointer to the native CServiceServer instance.
         */
        ::eCAL::CServiceServer* m_native_service_server;

        /**
         * @brief Dictionary mapping method name to the managed method callback.
         */
        Dictionary<System::String^, MethodCallback^>^ m_managed_callbacks;

        /**
         * @brief Internal native callback delegate definition.
         *
         * This delegate is used to bridge native calls to managed callbacks.
         */
        delegate int servCallback(const ::eCAL::SServiceMethodInformation& methodInfo, const std::string& request, std::string& response);

        /**
         * @brief Dictionary mapping method name to the native callback delegate.
         */
        Dictionary<System::String^, servCallback^>^ m_native_callbacks;

        /**
         * @brief Dictionary mapping method name to the GCHandle for the native callback delegate.
         */
        Dictionary<System::String^, GCHandle>^ m_native_callbacks_handles;

        /**
         * @brief Internal method callback invoked by the native code.
         *
         * This function converts native types to managed types and then invokes the managed callback
         * corresponding to the method name.
         *
         * @param methodInfo The service method information.
         * @param request The request data as a std::string.
         * @param response The response data as a std::string (to be filled).
         * 
         * @return 0 on success.
         */
        int OnMethodCall(const ::eCAL::SServiceMethodInformation& methodInfo, const std::string& request, std::string& response);

        /**
         * @brief stdcall function pointer definition of eCAL::MethodCallbackT.
         */
        typedef int(__stdcall* stdcall_eCAL_MethodCallbackT)(const ::eCAL::SServiceMethodInformation& methodInfo, const std::string& request, std::string& response);
      };

    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse
