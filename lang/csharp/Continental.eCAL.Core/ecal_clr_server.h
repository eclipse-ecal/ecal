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
 * @file
 * @brief
**/

#pragma once
#include <ecal/ecal.h>

using namespace System;
using namespace System::Runtime::InteropServices;

namespace Continental
{
  namespace eCAL
  {
    namespace Core
    {
      /**
       * @brief eCAL server class.
       *
       * The CServiceServer class is used to answer calls from matching eCAL clients.
       *
      **/
      public ref class ServiceServer
      {
      public:
          /**
           * @brief Constructor.
           *
           * @param topic_name_   Unique server name.
          **/
          ServiceServer(System::String^ server_name_);

          /**
           * @brief Destructor.
          **/
          ~ServiceServer();

        /**
         * @brief delegate definition for callback functions
        **/
        delegate array<Byte>^ MethodCallback(String^ methodName, String^ reqType, String^ responseType, array<Byte>^ request);

        /**
         * @brief Add callback function for incoming calls.
         *
         * @param callback_  The callback function set to connect.
         *
         * @return  True if succeeded, false if not.
        **/
        bool SetMethodCallback(String^ methodName, String^ reqType, String^ responseType, MethodCallback^ callback_);

        /**
         * @brief Remove callback function for incoming calls.
         *
         * @param callback_  The callback function set to disconnect.
         *
         * @return  True if succeeded, false if not.
        **/
        bool RemoveMethodCallback(String^ methodName, MethodCallback^ callback_);

      private:
        ::eCAL::CServiceServer* m_serv;
        /**
         * @brief managed callbacks that will get executed during the eCAL method callback
        **/
        MethodCallback^ m_callbacks;

        /**
         * @brief private member which holds the pointer to OnMethodCall, to avoid function relocation
        **/
        GCHandle m_gch;

        /**
         * @brief The callback of the subscriber, that is registered with the unmanaged code
        **/
        delegate int servCallback(const ::eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_);
        servCallback^ m_serv_callback;
        int OnMethodCall(const ::eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_);

        /**
         * @brief stdcall function pointer definition of eCAL::MethodCallbackT
        **/
        typedef int(__stdcall * stdcall_eCAL_MethodCallbackT)(const ::eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_);
      };
    }
  }
}
