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
 * @file   client.h
 * @brief  eCAL Protobuf Client Interface
 *
 * This header file defines two primary templated client classes for interacting with
 * eCAL services that use Google Protocol Buffers (protobuf) for message serialization.
 *
 * The file provides:
 *
 * 1. CServiceClient (Untyped Interface)
 *    - Implements an untyped service client interface for protobuf-based services.
 *    - Provides methods for both synchronous (blocking) and asynchronous (callback)
 *      service invocations where responses are delivered in a generic (untyped) format
 *      (i.e., SServiceResponse).
 *    - Suitable for scenarios where the user prefers to handle raw service responses or
 *      perform custom parsing.
 *
 * 2. CServiceClientTyped (Typed Interface)
 *    - Implements a typed service client interface for protobuf-based services.
 *    - Offers methods for synchronous and asynchronous service invocations that support
 *      automatic parsing of the responses into user-defined protobuf message types.
 *    - Uses templated call methods to ensure that responses are converted to the expected
 *      type (TMsgServiceResponse) as specified by the caller.
 *
 * Both classes inherit from the common base class, CServiceClientBase, which now is parameterized
 * on both the service description type and the client instance wrapper type. This base class encapsulates
 * core functionality such as:
 *    - Converting the underlying base client instances into templated client instance wrappers.
 *    - Providing helper functions (like ProcessInstances) to uniformly process all available client instances.
 *
 * The header also brings in necessary utility functions from client_protobuf_utils.h,
 * which extract service descriptors and method information from the provided protobuf
 * service descriptions. This enables proper registration, serialization, and deserialization
 * of service messages.
 *
 * Key Features:
 *    - Blocking calls using CallWithResponse:
 *         * Untyped version returns a vector of SServiceResponse.
 *         * Typed version returns a vector of TMsgServiceResponse<ResponseT>.
 *
 *    - Asynchronous calls using CallWithCallback and CallWithCallbackAsync:
 *         * Untyped version uses ResponseCallbackT for generic response handling.
 *         * Typed version uses TMsgResponseCallbackT<ResponseT> to automatically parse and
 *           deliver typed responses.
 *
 * Usage Examples:
 *
 *    // Using the untyped service client:
 *    eCAL::protobuf::CServiceClient<MyService> client;
 *    auto [success, responses] = client.CallWithResponse("MyMethod", requestMessage);
 *
 *    // Using the typed service client:
 *    eCAL::protobuf::CServiceClientTyped<MyService> typedClient;
 *    auto [success, typedResponses] = typedClient.CallWithResponse<MyResponseType>("MyMethod", requestMessage);
 *
 * @note The template parameter T for both classes must be a protobuf service description
 *       type that provides access to the service descriptor (e.g., via a GetDescriptor() method).
 */

#pragma once

#include <ecal/service/client.h>
#include <ecal/msg/protobuf/client_base.h>
#include <ecal/msg/protobuf/client_instance.h>
#include <ecal/msg/protobuf/client_protobuf_types.h>
#include <ecal/msg/protobuf/client_protobuf_utils.h>

#include <google/protobuf/message.h>

#include <string>
#include <vector>
#include <utility>

namespace eCAL
{
  namespace protobuf
  {
    /**
     * @brief (Untyped) service client for Protobuf services.
     *
     * Implements synchronous (blocking) and asynchronous (callback) calls with
     * untyped responses. The following methods are provided:
     *   - CallWithResponse: synchronous call returning untyped responses.
     *   - CallWithCallback: asynchronous call with callback returning untyped responses.
     *   - CallWithCallbackAsync: asynchronous (non-blocking) call with callback returning untyped responses.
     *
     * @tparam T The service description type.
     */
    template <typename T>
    class CServiceClient : public CServiceClientBase<T, CClientInstance<T>>
    {
    public:
      using Base = CServiceClientBase<T, CClientInstance<T>>;
      using Base::Base;  // Inherit constructors from the base class.
      virtual ~CServiceClient() override = default;

      /**
       * @brief Synchronous call returning untyped responses.
       *
       * @param method_name_  Name of the service method.
       * @param request_      Request message.
       * @param timeout_ms_   Timeout in milliseconds (use DEFAULT_TIME_ARGUMENT for infinite wait).
       * 
       * @return A pair containing an overall success flag and a vector of untyped responses (SServiceResponse).
       */
      std::pair<bool, std::vector<SServiceResponse>> CallWithResponse(const std::string& method_name_,
        const google::protobuf::Message& request_,
        int timeout_ms_ = DEFAULT_TIME_ARGUMENT) const
      {
        return this->template ProcessInstances<SServiceResponse>(
          [&](auto& instance) {
            return instance.CallWithResponse(method_name_, request_, timeout_ms_);
          }
        );
      }

      /**
       * @brief Asynchronous call with callback returning untyped responses.
       *
       * @param method_name_        Name of the service method.
       * @param request_            Request message.
       * @param response_callback_  Callback to receive the untyped response.
       * @param timeout_ms_         Timeout in milliseconds (use DEFAULT_TIME_ARGUMENT for infinite wait).
       * 
       * @return True if the call was successfully initiated.
       */
      bool CallWithCallback(const std::string& method_name_,
        const google::protobuf::Message& request_,
        const ResponseCallbackT& response_callback_,
        int timeout_ms_ = DEFAULT_TIME_ARGUMENT) const
      {
        bool overall_success = true;
        for (auto& instance : this->GetClientInstances())
        {
          overall_success &= instance.CallWithCallback(method_name_, request_, response_callback_, timeout_ms_);
        }
        return overall_success;
      }

      /**
       * @brief Asynchronous call with callback (async variant) returning untyped responses.
       *
       * @param method_name_        Name of the service method.
       * @param request_            Request message.
       * @param response_callback_  Callback to receive the untyped response.
       * 
       * @return True if the call was successfully initiated.
       */
      bool CallWithCallbackAsync(const std::string& method_name_,
        const google::protobuf::Message& request_,
        const ResponseCallbackT& response_callback_) const
      {
        bool overall_success = true;
        for (auto& instance : this->GetClientInstances())
        {
          overall_success &= instance.CallWithCallbackAsync(method_name_, request_, response_callback_);
        }
        return overall_success;
      }
    };

    /**
     * @brief Typed service client for Protobuf services.
     *
     * Implements synchronous (blocking) and asynchronous (callback) calls with
     * typed responses. The following methods are provided:
     *   - CallWithResponse: synchronous call returning typed responses.
     *   - CallWithCallback: asynchronous call with callback returning typed responses.
     *   - CallWithCallbackAsync: asynchronous (non-blocking) call with callback returning typed responses.
     *
     * @tparam T The service description type.
     */
    template <typename T>
    class CServiceClientTyped : public CServiceClientBase<T, CClientInstanceTyped<T>>
    {
    public:
      using Base = CServiceClientBase<T, CClientInstanceTyped<T>>;
      using Base::Base;  // Inherit constructors from the base class.
      virtual ~CServiceClientTyped() override = default;

      /**
       * @brief Synchronous call returning typed responses.
       *
       * @tparam ResponseT     Expected Protobuf response type.
       * 
       * @param  method_name_  Name of the service method.
       * @param  request_      Request message.
       * @param  timeout_ms_   Timeout in milliseconds (use DEFAULT_TIME_ARGUMENT for infinite wait).
       * 
       * @return A pair containing an overall success flag and a vector of typed responses.
       */
      template <typename ResponseT>
      std::pair<bool, TMsgServiceResponseVecT<ResponseT>> CallWithResponse(const std::string& method_name_,
        const google::protobuf::Message& request_,
        int timeout_ms_ = DEFAULT_TIME_ARGUMENT) const
      {
        return this->template ProcessInstances<TMsgServiceResponse<ResponseT>>(
          [&](auto& instance) {
            return instance.template CallWithResponse<ResponseT>(method_name_, request_, timeout_ms_);
          }
        );
      }

      /**
       * @brief Asynchronous call with callback returning typed responses.
       *
       * @tparam ResponseT           Expected Protobuf response type.
       * 
       * @param  method_name_        Name of the service method.
       * @param  request_            Request message.
       * @param  response_callback_  Callback to receive the typed response.
       * @param  timeout_ms_         Timeout in milliseconds (use DEFAULT_TIME_ARGUMENT for infinite wait).
       * 
       * @return True if the call was successfully initiated.
       */
      template <typename ResponseT>
      bool CallWithCallback(const std::string& method_name_,
        const google::protobuf::Message& request_,
        const TMsgResponseCallbackT<ResponseT>& response_callback_,
        int timeout_ms_ = DEFAULT_TIME_ARGUMENT) const
      {
        bool overall_success = true;
        for (auto& instance : this->GetClientInstances())
        {
          overall_success &= instance.template CallWithCallback<ResponseT>(method_name_, request_, response_callback_, timeout_ms_);
        }
        return overall_success;
      }

      /**
       * @brief Asynchronous call with callback (async variant) returning typed responses.
       *
       * @tparam ResponseT          Expected Protobuf response type.
       * 
       * @param  method_name_       Name of the service method.
       * @param  request_           Request message.
       * @param  response_callback_ Callback to receive the typed response.
       * 
       * @return True if the call was successfully initiated.
       */
      template <typename ResponseT>
      bool CallWithCallbackAsync(const std::string& method_name_,
        const google::protobuf::Message& request_,
        const TMsgResponseCallbackT<ResponseT>& response_callback_) const
      {
        bool overall_success = true;
        for (auto& instance : this->GetClientInstances())
        {
          overall_success &= instance.template CallWithCallbackAsync<ResponseT>(method_name_, request_, response_callback_);
        }
        return overall_success;
      }
    };

  } // namespace protobuf
} // namespace eCAL
