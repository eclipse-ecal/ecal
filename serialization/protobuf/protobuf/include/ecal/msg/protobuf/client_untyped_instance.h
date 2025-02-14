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
 * @file   msg/protobuf/client_untyped_instance.h
 * @brief  eCAL Protobuf Untyped Client Instance Interface
 *
 * This header file implements the client instance interface for eCAL services using
 * Google Protocol Buffers (protobuf) for message serialization.
 *
 * It defines the CClientInstanceUntyped class which supports untyped responses.
 * This class provides methods for synchronous and asynchronous service calls that return
 * untyped responses.
 *
 * The call methods are templated on the request type.
 *
 * This class wraps an underlying eCAL client instance.
 */

#pragma once

#include <ecal/service/client_instance.h>
#include <ecal/msg/protobuf/ecal_proto_dyn.h>
#include <ecal/msg/protobuf/client_protobuf_types.h>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace eCAL
{
  namespace protobuf
  {
    /**
     * @brief Untyped client instance for Protobuf services.
     *
     * Provides blocking and callback-based methods that return untyped responses.
     * The call methods are templated on the request type.
     *
     * @tparam T The service description type.
     */
    template <typename T>
    class CClientInstanceUntyped
    {
    public:
      // Constructors
      CClientInstanceUntyped(eCAL::CClientInstance&& base_instance_) noexcept
        : m_instance(std::move(base_instance_))
      {
      }

      CClientInstanceUntyped(const SEntityId& entity_id_,
                             const std::shared_ptr<eCAL::CServiceClientImpl>& service_client_impl_)
        : m_instance(entity_id_, service_client_impl_)
      {
      }

      /**
       * @brief Blocking call that returns an untyped response.
       *
       * @tparam RequestT      The type of the request message (must provide SerializeAsString()).
	   *
       * @param  method_name_  Name of the service method.
       * @param  request_      Request message.
       * @param  timeout_ms_   Timeout in milliseconds.
	   *
       * @return A pair containing a success flag and an untyped response.
       */
      template <typename RequestT>
      std::pair<bool, SServiceResponse> CallWithResponse(const std::string& method_name_,
                                                         const RequestT& request_,
                                                         int timeout_ms_ = eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT)
      {
        const std::string serialized_request = request_.SerializeAsString();
        auto ret = m_instance.CallWithResponse(method_name_, serialized_request, timeout_ms_);
        return std::make_pair(ret.first, ret.second);
      }

      /**
       * @brief Call with callback that returns an untyped response.
       *
       * @tparam RequestT            The type of the request message (must provide SerializeAsString()).
	   *
       * @param  method_name_        Name of the service method.
       * @param  request_            Request message.
       * @param  response_callback_  Callback accepting an untyped response.
       * @param  timeout_ms_         Timeout in milliseconds.
	   *
       * @return True if the call was successfully initiated.
       */
      template <typename RequestT>
      bool CallWithCallback(const std::string& method_name_,
                            const RequestT& request_,
                            const ResponseCallbackT& response_callback_,
                            int timeout_ms_ = eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT)
      {
        const std::string serialized_request = request_.SerializeAsString();
        return m_instance.CallWithCallback(method_name_, serialized_request, response_callback_, timeout_ms_);
      }

      /**
       * @brief Asynchronous call with callback that returns an untyped response.
       *
       * @tparam RequestT            The type of the request message (must provide SerializeAsString()).
	   *
       * @param  method_name_        Name of the service method.
       * @param  request_            Request message.
       * @param  response_callback_  Callback accepting an untyped response.
	   *
       * @return True if the call was successfully initiated.
       */
      template <typename RequestT>
      bool CallWithCallbackAsync(const std::string& method_name_,
                                 const RequestT& request_,
                                 const ResponseCallbackT& response_callback_)
      {
        const std::string serialized_request = request_.SerializeAsString();
        return m_instance.CallWithCallbackAsync(method_name_, serialized_request, response_callback_);
      }

      /**
       * @brief Get the unique client entity id.
       * @return The client entity id.
       */
      SEntityId GetClientID() const
      {
        return m_instance.GetClientID();
      }

    private:
      // The underlying base client instance.
      eCAL::CClientInstance m_instance;
    };

  } // namespace protobuf
} // namespace eCAL
