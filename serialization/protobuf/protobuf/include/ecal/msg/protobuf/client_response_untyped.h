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
 * @file   client_response_untyped.h
**/

#pragma once

#include <ecal/msg/protobuf/client_response_base.h>

#include <google/protobuf/message.h>

#include <string>
#include <utility>

namespace eCAL
{
  namespace protobuf
  {
    /**
     * @brief Client class for untyped synchronous responses.
     *
     * Provides only the CallWithResponse method that returns a vector of SServiceResponse.
     */
    template <typename T>
    class CServiceClientUntypedResponse : public CServiceClientResponseBase<T>
    {
    public:
      using CServiceClientResponseBase<T>::CServiceClientResponseBase; // Inherit constructors

      /**
       * @brief Blocking call returning untyped responses.
       *
       * @param method_name Method name.
       * @param request     Request message.
       * @param timeout     Timeout in milliseconds (DEFAULT_TIME_ARGUMENT for infinite).
       * @return A pair of overall success and a vector of SServiceResponse.
       */
      std::pair<bool, ServiceResponseVecT> CallWithResponse(const std::string& method_name,
                                                            const google::protobuf::Message& request,
                                                            int timeout = DEFAULT_TIME_ARGUMENT) const
      {
        return this->template ProcessInstances<SServiceResponse>(
          [&](auto& instance) {
            return instance.CallWithResponse(method_name, request, timeout);
          }
        );
      }
    };
  }
}
