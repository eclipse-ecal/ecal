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
 * @file   msg/protobuf/client_instance.h
 * @brief  eCAL Protobuf Typed Client Instance Interface
 *
 * This header file provides the implementation of the client instance interface for eCAL services
 * using Google Protocol Buffers (protobuf) for message serialization and deserialization.
 *
 * It defines the CClientInstance class which supports typed responses.
 * This class provides methods for synchronous and asynchronous service calls that automatically
 * parse raw responses into strongly-typed response objects.
 *
 * @note The service description type T must provide access to its protobuf ServiceDescriptor
 *       to enable correct parsing of responses.
 */

#pragma once

#include <ecal/service/client_instance.h>
#include <ecal/msg/protobuf/ecal_proto_dyn.h>
#include <ecal/msg/protobuf/client_protobuf_types.h>
#include <ecal/msg/protobuf/client_protobuf_utils.h>

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
     * @brief Templated client instance for Protobuf services with typed responses.
     *
     * Provides blocking and callback-based methods that automatically parse raw responses
     * into strongly-typed response objects.
     *
     * @tparam T The service description type.
     */
    template <typename T>
    class CClientInstance
    {
    public:
      // Constructors
      CClientInstance(eCAL::CClientInstance&& base_instance_) noexcept
        : m_instance(std::move(base_instance_))
      {
      }

      CClientInstance(const SEntityId& entity_id_,
                      const std::shared_ptr<eCAL::CServiceClientImpl>& service_client_impl_)
        : m_instance(entity_id_, service_client_impl_)
      {
      }

      /**
       * @brief Blocking call that returns a typed response.
       *
       * @tparam RequestT     The type of the request message (must provide SerializeAsString()).
       * @tparam ResponseT    Expected protobuf response type.
       * 
       * @param  method_name_ The name of the service method.
       * @param  request_     The request message.
       * @param  timeout_ms_  Timeout in milliseconds.
       * 
       * @return A pair consisting of a success flag and a typed response.
       */
      template <typename RequestT, typename ResponseT>
      std::pair<bool, SMsgServiceResponse<ResponseT>> CallWithResponse(const std::string& method_name_,
                                                                       const RequestT& request_,
                                                                       int timeout_ms_ = eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT)
      {
        const std::string serialized_request = request_.SerializeAsString();
        auto ret = m_instance.CallWithResponse(method_name_, serialized_request, timeout_ms_);
        SMsgServiceResponse<ResponseT> msg_response = ConvertResponse<ResponseT>(ret.second);
        bool success = ret.first && (msg_response.call_state == eCallState::executed);
        return std::make_pair(success, msg_response);
      }

      /**
       * @brief Call with callback returning a typed response.
       *
       * @tparam RequestT            The type of the request message (must provide SerializeAsString()).
       * @tparam ResponseT           Expected protobuf response type.
       * 
       * @param  method_name_        The name of the service method.
       * @param  request_            The request message.
       * @param  response_callback_  Callback accepting a typed response.
       * @param  timeout_ms_         Timeout in milliseconds.
       * 
       * @return True if the call was successfully initiated.
       */
      template <typename RequestT, typename ResponseT>
      bool CallWithCallback(const std::string& method_name_,
                            const RequestT& request_,
                            const SMsgResponseCallbackT<ResponseT>& response_callback_,
                            int timeout_ms_ = eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT)
      {
        const std::string serialized_request = request_.SerializeAsString();
        auto wrapper = [this, method_name_, response_callback_](const SServiceResponse& service_response)
        {
          SMsgServiceResponse<ResponseT> msg_response = ConvertResponse<ResponseT>(service_response);
          response_callback_(msg_response);
        };
        return m_instance.CallWithCallback(method_name_, serialized_request, wrapper, timeout_ms_);
      }

      /**
       * @brief Asynchronous call with callback returning a typed response.
       *
       * @tparam RequestT            The type of the request message (must provide SerializeAsString()).
       * @tparam ResponseT           Expected protobuf response type.
       * 
       * @param  method_name_        The name of the service method.
       * @param  request_            The request message.
       * @param  response_callback_  Callback accepting a typed response.
       * 
       * @return True if the call was successfully initiated.
       */
      template <typename RequestT, typename ResponseT>
      bool CallWithCallbackAsync(const std::string& method_name_,
                                 const RequestT& request_,
                                 const SMsgResponseCallbackT<ResponseT>& response_callback_)
      {
        const std::string serialized_request = request_.SerializeAsString();
        auto wrapper = [this, method_name_, response_callback_](const SServiceResponse& service_response)
        {
          SMsgServiceResponse<ResponseT> msg_response = ConvertResponse<ResponseT>(service_response);
          response_callback_(msg_response);
        };
        return m_instance.CallWithCallbackAsync(method_name_, serialized_request, wrapper);
      }

      /**
       * @brief Get unique client entity id.
       *
       * @return The client entity id.
       */
      const SEntityId& GetClientID() const
      {
        return m_instance.GetClientID();
      }

    private:
      // The underlying base client instance.
      eCAL::CClientInstance m_instance;

      /**
       * @brief Helper conversion function: converts an untyped service response into a typed response.
       *
       * This function attempts to parse the raw response string into a ResponseT object. If parsing
       * fails, an error message is set and the call state is updated accordingly.
       *
       * @tparam ResponseT         The expected protobuf response type.
       * 
       * @param  service_response  The raw service response.
       * 
       * @return A SMsgServiceResponse<ResponseT> structure containing the parsed response and metadata.
       */
      template <typename ResponseT>
      static SMsgServiceResponse<ResponseT> ConvertResponse(const SServiceResponse& service_response)
      {
        SMsgServiceResponse<ResponseT> msg_response;
        // Attempt to parse the raw response into a ResponseT object.
        if (!msg_response.response.ParseFromString(service_response.response))
        {
          msg_response.error_msg = "Failed to parse response";
          msg_response.call_state = eCallState::failed;
        }
        else
        {
          msg_response.call_state                 = service_response.call_state;
          msg_response.server_id                  = service_response.server_id;
          msg_response.service_method_information = service_response.service_method_information;
          msg_response.ret_state                  = service_response.ret_state;
          msg_response.error_msg                  = service_response.error_msg;
        }
        return msg_response;
      }
    };

  } // namespace protobuf
} // namespace eCAL
