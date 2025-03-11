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
 * @file   msg/protobuf/client_untyped.h
 * @brief  eCAL Protobuf Untyped Client Interface
 *
 * This header file defines a templated client class for interacting with eCAL services
 * that use Google Protocol Buffers (protobuf) for message serialization.
 *
 * It provides the CServiceClientUntyped class which implements an untyped service client interface.
 * This interface offers synchronous and asynchronous calls that return untyped responses.
 * The call methods are templated on the request type.
 *
 * @note This class inherits from CServiceClientBase specialized for untyped responses.
 */

#pragma once

#include <ecal/service/client.h>
#include <ecal/msg/protobuf/client_base.h>
#include <ecal/msg/protobuf/client_untyped_instance.h>
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
     * @brief Untyped service client for Protobuf services.
     *
     * Implements synchronous (blocking) and asynchronous (callback) calls that return untyped responses.
     * The call methods are templated on the request type.
     *
     * @tparam T The service description type.
     */
    template <typename T>
    class CServiceClientUntyped : public CServiceClientBase<T, CClientInstanceUntyped<T>>
    {
    public:
      using Base = CServiceClientBase<T, CClientInstanceUntyped<T>>;
      using Base::Base;  // Inherit constructors from the base class.

      CServiceClientUntyped() = default;
      ~CServiceClientUntyped() override = default;

      // Non-copyable
      CServiceClientUntyped(const CServiceClientUntyped&) = delete;
      CServiceClientUntyped& operator=(const CServiceClientUntyped&) = delete;

      // Movable
      CServiceClientUntyped(CServiceClientUntyped&& rhs) = default;
      CServiceClientUntyped& operator=(CServiceClientUntyped&& rhs) = default;

      /**
       * @brief Synchronous call returning untyped responses.
       *
       * @tparam RequestT     The type of the request message (must provide SerializeAsString()).
	   *
       * @param  method_name_ Name of the service method.
       * @param  request_     Request message.
       * @param  timeout_ms_  Timeout in milliseconds (use DEFAULT_TIME_ARGUMENT for infinite wait).
	   *
       * @return A pair containing a success flag and a vector of untyped responses.
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

  } // namespace protobuf
} // namespace eCAL
