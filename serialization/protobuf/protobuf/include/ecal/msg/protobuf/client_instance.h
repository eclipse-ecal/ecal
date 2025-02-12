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
 * @brief  eCAL client instance interface based on protobuf service description
 *
 * This header file provides the implementation of client instance interfaces for eCAL services
 * that leverage Google Protocol Buffers (protobuf) for message serialization and deserialization.
 *
 * The file defines two primary templated classes that wrap an underlying eCAL client instance:
 *
 * 1. CClientInstance<T>
 *    - Supports untyped responses.
 *    - Implements blocking and asynchronous service calls that deliver generic responses
 *      (i.e., SServiceResponse) without performing any type conversion.
 *
 *    Key functionalities include:
 *      - **CallWithResponse**: A blocking call that returns a pair consisting of a success flag
 *        and a raw, untyped service response.
 *      - **CallWithCallback**: A callback-based call where the provided callback receives the raw response.
 *      - **CallWithCallbackAsync**: A non-blocking variant that triggers a callback with the untyped response.
 *
 *    The template parameter T should be a service description type that exposes (or allows access to)
 *    its protobuf ServiceDescriptor (typically via a protected GetDescriptor() method).
 *
 * 2. CClientInstanceTyped<T>
 *    - Supports typed responses.
 *    - Implements methods that not only perform the service call but also automatically parse and convert
 *      the raw response into a strongly-typed response object.
 *
 *    Key functionalities include:
 *      - **CallWithResponse**: A templated blocking call that, given an expected response type (ResponseT),
 *        returns a pair of a success flag and a parsed response encapsulated in a SMsgServiceResponse<ResponseT> structure.
 *      - **CallWithCallback**: A templated callback-based call that parses the raw response and delivers it
 *        to a callback expecting a typed response.
 *      - **CallWithCallbackAsync**: A non-blocking variant of the typed callback-based call.
 *
 * Both classes encapsulate an underlying eCAL::CClientInstance that manages the low-level communication
 * with the service endpoints. They provide a higher-level, type-safe abstraction for making service calls,
 * thereby isolating the application developer from the intricacies of message serialization and parsing.
 *
 * Usage Examples:
 *
 *   // Untyped client instance usage:
 *   eCAL::protobuf::CClientInstance<MyService> instance(myBaseInstance);
 *   auto result = instance.CallWithResponse("MyMethod", requestMessage);
 *   if (result.first)
 *   {
 *     // Process the untyped response in result.second.
 *   }
 *
 *   // Typed client instance usage:
 *   eCAL::protobuf::CClientInstanceTyped<MyService> typedInstance(myBaseInstance);
 *   auto typedResult = typedInstance.CallWithResponse<MyResponseType>("MyMethod", requestMessage);
 *   if (typedResult.first)
 *   {
 *     // Process the typed response contained in typedResult.second.
 *   }
 *
 * @note The service description type T must provide access to its protobuf ServiceDescriptor
 *       (typically via a GetDescriptor() method) to enable correct parsing of responses.
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
    ////////////////////////////////////////////////////////////////////////////
    // Untyped Client Instance
    ////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Templated client instance that supports untyped responses.
     *
     * The template parameter T is the service description type that provides a GetDescriptor() method.
     *
     * This class provides:
     *   - Blocking calls (CallWithResponse) that return untyped responses (SServiceResponse).
     *   - Callback-based calls (CallWithCallback and CallWithCallbackAsync) that deliver untyped responses.
     *
     * No type conversion or parsing is performed in these methods.
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
       * @brief Blocking call that returns an untyped response.
       *
       * @param  method_name_  The name of the service method.
       * @param  request_      The request message.
       * @param  timeout_ms_   Timeout in milliseconds.
       *
       * @return A pair consisting of a success flag and an untyped response.
       */
      std::pair<bool, SServiceResponse> CallWithResponse(const std::string& method_name_,
        const google::protobuf::Message& request_,
        int timeout_ms_ = eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT)
      {
        const std::string serialized_request = request_.SerializeAsString();
        auto ret = m_instance.CallWithResponse(method_name_, serialized_request, timeout_ms_);
        return std::make_pair(ret.first, ret.second);
      }

      /**
       * @brief Call with callback that returns an untyped response.
       *
       * @param  method_name_        The name of the service method.
       * @param  request_            The request message.
       * @param  response_callback_  Callback accepting an untyped response.
       * @param  timeout_ms_         Timeout in milliseconds.
       *
       * @return True if the call was successfully initiated.
       */
      bool CallWithCallback(const std::string& method_name_,
        const google::protobuf::Message& request_,
        const ResponseCallbackT& response_callback_,
        int timeout_ms_ = eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT)
      {
        const std::string serialized_request = request_.SerializeAsString();
        return m_instance.CallWithCallback(method_name_, serialized_request, response_callback_, timeout_ms_);
      }

      /**
       * @brief Asynchronous call with callback that returns an untyped response.
       *
       * @param  method_name_        The name of the service method.
       * @param  request_            The request message.
       * @param  response_callback_  Callback accepting an untyped response.
       *
       * @return True if the call was successfully initiated.
       */
      bool CallWithCallbackAsync(const std::string& method_name_,
        const google::protobuf::Message& request_,
        const ResponseCallbackT& response_callback_)
      {
        const std::string serialized_request = request_.SerializeAsString();
        return m_instance.CallWithCallbackAsync(method_name_, serialized_request, response_callback_);
      }

      /**
       * @brief Get unique client entity id.
       *
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

    ////////////////////////////////////////////////////////////////////////////
    // Typed Client Instance
    ////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Templated client instance that supports typed responses.
     *
     * The template parameter T is the service description type that provides a GetDescriptor() method.
     *
     * This class provides:
     *   - Blocking calls (CallWithResponse) that return a parsed, typed response.
     *   - Callback-based calls (CallWithCallback and CallWithCallbackAsync) that deliver a typed response.
     *
     * All typed calls internally perform parsing using the ResponseParser utility.
     */
    template <typename T>
    class CClientInstanceTyped
    {
    public:
      // Constructors
      CClientInstanceTyped(eCAL::CClientInstance&& base_instance_) noexcept
        : m_instance(std::move(base_instance_))
      {
      }

      CClientInstanceTyped(const SEntityId& entity_id_,
        const std::shared_ptr<eCAL::CServiceClientImpl>& service_client_impl_)
        : m_instance(entity_id_, service_client_impl_)
      {
      }

      /**
       * @brief Blocking call that returns a typed response.
       *
       * @tparam ResponseT     Expected protobuf response type.
       *
       * @param  method_name_  The name of the service method.
       * @param  request_      The request message.
       * @param  timeout_ms_   Timeout in milliseconds.
       *
       * @return A pair consisting of a success flag and a typed response.
       */
      template <typename ResponseT>
      std::pair<bool, SMsgServiceResponse<ResponseT>> CallWithResponse(const std::string& method_name_,
        const google::protobuf::Message& request_,
        int timeout_ms_ = eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT)
      {
        const std::string serialized_request = request_.SerializeAsString();

        auto ret = m_instance.CallWithResponse(method_name_, serialized_request, timeout_ms_);
        SMsgServiceResponse<ResponseT> msg_response = ConvertResponse<ResponseT>(ret.second);

        bool success = ret.first && (msg_response.call_state == eCallState::executed);
        return std::make_pair(success, msg_response);
      }

      /**
       * @brief Call with callback that returns a typed response.
       *
       * @tparam ResponseT           Expected protobuf response type.
       *
       * @param  method_name_        The name of the service method.
       * @param  request_            The request message.
       * @param  response_callback_  Callback accepting a typed response.
       * @param  timeout_ms_         Timeout in milliseconds.
       *
       * @return True if the call was successfully initiated.
       */
      template <typename ResponseT>
      bool CallWithCallback(const std::string& method_name_,
        const google::protobuf::Message& request_,
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
       * @brief Asynchronous call with callback that returns a typed response.
       *
       * @tparam ResponseT           Expected protobuf response type.
       *
       * @param  method_name_        The name of the service method.
       * @param  request_            The request message.
       * @param  response_callback_  Callback accepting a typed response.
       *
       * @return True if the call was successfully initiated.
       */
      template <typename ResponseT>
      bool CallWithCallbackAsync(const std::string& method_name_,
        const google::protobuf::Message& request_,
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
      SEntityId GetClientID() const
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
       * @tparam ResponseT The expected protobuf response type.
       * @param service_response The raw service response.
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
