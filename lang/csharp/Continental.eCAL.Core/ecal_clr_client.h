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
 * @file  ecal_clr_client.h
**/

#pragma once
#include <ecal/ecal.h>

using namespace System;
using namespace System::Collections::Generic;

namespace Continental
{
  namespace eCAL
  {
    namespace Core
    {
      /**
       * @brief eCAL service client class.
       *
       * The CServiceClient class is used to call a matching eCAL server.
       *
      **/
      public ref class ServiceClient
      {
      public:
          /**
           * @brief Constructor.
           *
           * @param service_name_   Unique service name.
          **/
          ServiceClient(System::String^ service_name_);

          /**
           * @brief Destructor.
          **/
          ~ServiceClient();

          enum class CallState
          {
            None = 0,    //!< undefined
            Executed,    //!< executed (successfully)
            Failed       //!< failed
          };
          /**
           * @brief structure which contains the data for callback functions
          **/
          ref struct ServiceClientCallbackData
          {
              String^      host_name;      //!< service host name
              String^      service_name;   //!< name of the service
              long long    service_id;     //!< id of the service
              String^      method_name;    //!< name of the service method
              String^      error_msg;      //!< human readable error message
              int          ret_state;      //!< return state of the called service method
              CallState    call_state;     //!< call state (see eCallState)
              array<Byte>^ response;       //!< service response
          };

          /**
           * @brief Call a server.
           *
           * @param method_name_
           * @param request
           * @param rcv_timeout_  Maximum time before receive operation returns (in milliseconds, -1 means infinite).
           *
           * @return  List<ServiceClientCallbackData> or null (if timed out)
          **/
          List<ServiceClientCallbackData^>^ Call(System::String^ method_name_, array<Byte>^ request, const int rcv_timeout_);

      private:
          ::eCAL::CServiceClient* m_client;
      };
   }
  }
}
