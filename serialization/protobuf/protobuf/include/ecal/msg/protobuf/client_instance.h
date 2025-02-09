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
 * @file   client_instance.h
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
 *        returns a pair of a success flag and a parsed response encapsulated in a TMsgServiceResponse<ResponseT> structure.
 *      - **CallWithCallback**: A templated callback-based call that parses the raw response and delivers it
 *        to a callback expecting a typed response.
 *      - **CallWithCallbackAsync**: A non-blocking variant of the typed callback-based call.
 *
 *    Internally, these methods use the ResponseParser::ParseResponse utility to convert the generic
 *    SServiceResponse into a TMsgServiceResponse<ResponseT>. To obtain the proper service descriptor
 *    required for parsing, a temporary subclass of T is instantiated via the helper function GetServiceDescriptor().
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
#include <ecal/msg/protobuf/client_response_parser.h>
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

      CClientInstance(const SEntityId& entity_id_, const std::shared_ptr<eCAL::CServiceClientImpl>& service_client_impl_)
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
        auto ret = m_instance.CallWithResponse(method_name_, request_.SerializeAsString(), timeout_ms_);
        return std::pair<bool, SServiceResponse>(ret.first, ret.second);
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
        return m_instance.CallWithCallback(method_name_, request_.SerializeAsString(), response_callback_, timeout_ms_);
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
        return m_instance.CallWithCallbackAsync(method_name_, request_.SerializeAsString(), response_callback_);
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

      CClientInstanceTyped(const SEntityId& entity_id_, const std::shared_ptr<eCAL::CServiceClientImpl>& service_client_impl_)
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
      std::pair<bool, TMsgServiceResponse<ResponseT>> CallWithResponse(const std::string& method_name_,
        const google::protobuf::Message& request_,
        int timeout_ms_ = eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT)
      {
        auto ret = m_instance.CallWithResponse(method_name_, request_.SerializeAsString(), timeout_ms_);
        TMsgServiceResponse<ResponseT> msg_response;
        bool success = ResponseParser::ParseResponse<ResponseT>(GetServiceDescriptor(), method_name_, ret.second, msg_response);

        return std::pair<bool, TMsgServiceResponse<ResponseT>>(ret.first && success, msg_response);
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
        const TMsgResponseCallbackT<ResponseT>& response_callback_,
        int timeout_ms_ = eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT)
      {
        auto wrapper = [this, method_name_, response_callback_](const SServiceResponse& service_response)
          {
            TMsgServiceResponse<ResponseT> msg_response;
            if (ResponseParser::ParseResponse<ResponseT>(GetServiceDescriptor(), method_name_, service_response, msg_response))
            {
              response_callback_(msg_response);
            }
          };

        return m_instance.CallWithCallback(method_name_, request_.SerializeAsString(), wrapper, timeout_ms_);
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
        const TMsgResponseCallbackT<ResponseT>& response_callback_)
      {
        auto wrapper = [this, method_name_, response_callback_](const SServiceResponse& service_response)
          {
            TMsgServiceResponse<ResponseT> msg_response;
            if (ResponseParser::ParseResponse<ResponseT>(GetServiceDescriptor(), method_name_, service_response, msg_response))
            {
              response_callback_(msg_response);
            }
          };

        return m_instance.CallWithCallbackAsync(method_name_, request_.SerializeAsString(), wrapper);
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
       * @brief Helper to retrieve the service descriptor from the templated service type.
       *
       * This instantiates a temporary subclass of T to expose its protected GetDescriptor() method.
       *
       * @return Pointer to the service descriptor.
       */
      static const google::protobuf::ServiceDescriptor* GetServiceDescriptor()
      {
        struct U : T {};
        U temp_instance;
        return temp_instance.GetDescriptor();
      }
    };

  } // namespace protobuf
} // namespace eCAL
