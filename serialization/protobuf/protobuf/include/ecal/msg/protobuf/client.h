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
 * @file   msg/protobuf/client.h
 * @brief  eCAL Protobuf Client Interface (adapted for templated request types)
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
 *    - The call methods are now templated on the request type.
 *
 * 2. CServiceClientTyped (Typed Interface)
 *    - Implements a typed service client interface for protobuf-based services.
 *    - Offers methods for synchronous and asynchronous service invocations that support
 *      automatic parsing of the responses into user-defined protobuf message types.
 *    - The call methods are now templated on both the request and response types.
 *
 * Both classes inherit from the common base class, CServiceClientBase, which is parameterized
 * on both the service description type and the client instance wrapper type.
 *
 * The header also brings in necessary utility functions from client_protobuf_utils.h,
 * which extract service descriptors and method information from the provided protobuf
 * service descriptions.
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
     * The call methods are now templated on the request type.
     *
     * @tparam T The service description type.
     */
    template <typename T>
    class CServiceClient : public CServiceClientBase<T, CClientInstance<T>>
    {
    public:
      using Base = CServiceClientBase<T, CClientInstance<T>>;
      using Base::Base;  // Inherit constructors from the base class.

      CServiceClient() = default;
      ~CServiceClient() override = default;

      /**
       * @brief CServiceClients are non-copyable.
       */
      CServiceClient(const CServiceClient&) = delete;
      CServiceClient& operator=(const CServiceClient&) = delete;

      /**
       * @brief CServiceClients are movable.
       */
      CServiceClient(CServiceClient&& rhs) = default;
      CServiceClient& operator=(CServiceClient&& rhs) = default;

      /**
       * @brief Synchronous call returning untyped responses.
       *
       * @tparam RequestT      The type of the request message (must provide SerializeAsString()).
       * 
       * @param  method_name_  Name of the service method.
       * @param  request_      Request message.
       * @param  timeout_ms_   Timeout in milliseconds (use DEFAULT_TIME_ARGUMENT for infinite wait).
       *
       * @return A pair containing an overall success flag and a vector of untyped responses (SServiceResponse).
       */
      template <typename RequestT>
      std::pair<bool, std::vector<SServiceResponse>> CallWithResponse(const std::string& method_name_,
        const RequestT& request_,
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
       * @tparam RequestT            The type of the request message (must provide SerializeAsString()).
       * 
       * @param  method_name_        Name of the service method.
       * @param  request_            Request message.
       * @param  response_callback_  Callback to receive the untyped response.
       * @param  timeout_ms_         Timeout in milliseconds (use DEFAULT_TIME_ARGUMENT for infinite wait).
       *
       * @return True if the call was successfully initiated.
       */
      template <typename RequestT>
      bool CallWithCallback(const std::string& method_name_,
        const RequestT& request_,
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
       * @tparam RequestT            The type of the request message (must provide SerializeAsString()).
       * 
       * @param  method_name_        Name of the service method.
       * @param  request_            Request message.
       * @param  response_callback_  Callback to receive the untyped response.
       *
       * @return True if the call was successfully initiated.
       */
      template <typename RequestT>
      bool CallWithCallbackAsync(const std::string& method_name_,
        const RequestT& request_,
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
     * The call methods are now templated on both the request and response types.
     *
     * @tparam T The service description type.
     */
    template <typename T>
    class CServiceClientTyped : public CServiceClientBase<T, CClientInstanceTyped<T>>
    {
    public:
      using Base = CServiceClientBase<T, CClientInstanceTyped<T>>;
      using Base::Base;  // Inherit constructors from the base class.

      CServiceClientTyped() = default;
      ~CServiceClientTyped() override = default;

      /**
       * @brief Typed CServiceClients are non-copyable.
       */
      CServiceClientTyped(const CServiceClientTyped&) = delete;
      CServiceClientTyped& operator=(const CServiceClientTyped&) = delete;

      /**
       * @brief Typed CServiceClients are movable.
       */
      CServiceClientTyped(CServiceClientTyped&& rhs) = default;
      CServiceClientTyped& operator=(CServiceClientTyped&& rhs) = default;

      /**
       * @brief Synchronous call returning typed responses.
       *
       * @tparam RequestT      The type of the request message (must provide SerializeAsString()).
       * @tparam ResponseT     Expected Protobuf response type.
       *
       * @param  method_name_  Name of the service method.
       * @param  request_      Request message.
       * @param  timeout_ms_   Timeout in milliseconds (use DEFAULT_TIME_ARGUMENT for infinite wait).
       *
       * @return A pair containing an overall success flag and a vector of typed responses.
       */
      template <typename RequestT, typename ResponseT>
      std::pair<bool, SMsgServiceResponseVecT<ResponseT>> CallWithResponse(const std::string& method_name_,
        const RequestT& request_,
        int timeout_ms_ = DEFAULT_TIME_ARGUMENT) const
      {
        return this->template ProcessInstances<SMsgServiceResponse<ResponseT>>(
          [&](auto& instance) {
            return instance.template CallWithResponse<RequestT, ResponseT>(method_name_, request_, timeout_ms_);
          }
        );
      }

      /**
       * @brief Asynchronous call with callback returning typed responses.
       *
       * @tparam RequestT            The type of the request message (must provide SerializeAsString()).
       * @tparam ResponseT           Expected Protobuf response type.
       *
       * @param  method_name_        Name of the service method.
       * @param  request_            Request message.
       * @param  response_callback_  Callback to receive the typed response.
       * @param  timeout_ms_         Timeout in milliseconds (use DEFAULT_TIME_ARGUMENT for infinite wait).
       *
       * @return True if the call was successfully initiated.
       */
      template <typename RequestT, typename ResponseT>
      bool CallWithCallback(const std::string& method_name_,
        const RequestT& request_,
        const SMsgResponseCallbackT<ResponseT>& response_callback_,
        int timeout_ms_ = DEFAULT_TIME_ARGUMENT) const
      {
        bool overall_success = true;
        for (auto& instance : this->GetClientInstances())
        {
          overall_success &= instance.template CallWithCallback<RequestT, ResponseT>(method_name_, request_, response_callback_, timeout_ms_);
        }
        return overall_success;
      }

      /**
       * @brief Asynchronous call with callback (async variant) returning typed responses.
       *
       * @tparam RequestT            The type of the request message (must provide SerializeAsString()).
       * @tparam ResponseT           Expected Protobuf response type.
       *
       * @param  method_name_        Name of the service method.
       * @param  request_            Request message.
       * @param  response_callback_  Callback to receive the typed response.
       *
       * @return True if the call was successfully initiated.
       */
      template <typename RequestT, typename ResponseT>
      bool CallWithCallbackAsync(const std::string& method_name_,
        const RequestT& request_,
        const SMsgResponseCallbackT<ResponseT>& response_callback_) const
      {
        bool overall_success = true;
        for (auto& instance : this->GetClientInstances())
        {
          overall_success &= instance.template CallWithCallbackAsync<RequestT, ResponseT>(method_name_, request_, response_callback_);
        }
        return overall_success;
      }
    };

  } // namespace protobuf
} // namespace eCAL
