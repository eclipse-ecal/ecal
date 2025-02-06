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
**/

#pragma once

#include <ecal/service/client_instance.h>
#include <ecal/msg/protobuf/ecal_proto_dyn.h>
#include <ecal/msg/protobuf/client_typed_response.h>

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
    // A typed callback for responses.
    template <typename ResponseT>
    using TMsgResponseCallbackT = std::function<void(const TMsgServiceResponse<ResponseT>&)>;

    /**
     * @brief Templated client instance that supports typed responses.
     *
     * The template parameter T is the service description type that provides a GetDescriptor() method.
     */
    template <typename T>
    class CClientInstance
    {
    public:
      // Constructor
      CClientInstance(eCAL::CClientInstance&& base_instance_) noexcept
        : m_instance(std::move(base_instance_))
      {
      }

      CClientInstance(const SEntityId& entity_id_, const std::shared_ptr<eCAL::CServiceClientImpl>& service_client_impl_)
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
       * @return  A pair of success flag and a typed response.
       */
      template <typename ResponseT>
      std::pair<bool, TMsgServiceResponse<ResponseT>> CallWithResponse(const std::string& method_name_, const google::protobuf::Message& request_,
        int timeout_ms_ = eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT)
      {
        auto ret = m_instance.CallWithResponse(method_name_, request_.SerializeAsString(), timeout_ms_);
        TMsgServiceResponse<ResponseT> msg_response;
        bool success = ProcessResponse<ResponseT>(method_name_, ret.second, msg_response);

        return std::pair<bool, TMsgServiceResponse<ResponseT>>(ret.first && success, msg_response);
      }

      /**
       * @brief Blocking call that returns a typed response.
       *
       * @param  method_name_  The name of the service method.
       * @param  request_      The request message.
       * @param  timeout_ms_   Timeout in milliseconds.
       *
       * @return  A pair of success flag and a response.
       */
      std::pair<bool, SServiceResponse> CallWithResponse(const std::string& method_name_, const google::protobuf::Message& request_,
        int timeout_ms_ = eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT)
      {
        auto ret = m_instance.CallWithResponse(method_name_, request_.SerializeAsString(), timeout_ms_);
        return std::pair<bool, SServiceResponse>(ret.first, ret.second);
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
       * @return  True if the call was successfully initiated.
       */
      template <typename ResponseT>
      bool CallWithCallback(const std::string& method_name_, const google::protobuf::Message& request_, const TMsgResponseCallbackT<ResponseT>& response_callback_,
        int timeout_ms_ = eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT)
      {
        auto wrapper = [this, method_name_, response_callback_](const SServiceResponse& service_response)
          {
            TMsgServiceResponse<ResponseT> msg_response;
            ProcessResponse<ResponseT>(method_name_, service_response, msg_response);
            response_callback_(msg_response);
          };

        return m_instance.CallWithCallback(method_name_, request_.SerializeAsString(), wrapper, timeout_ms_);
      }

      /**
       * @brief Call with callback.
       *
       * @param  method_name_        The name of the service method.
       * @param  request_            The request message.
       * @param  response_callback_  Callback accepting a typed response.
       * @param  timeout_ms_         Timeout in milliseconds.
       *
       * @return  True if the call was successfully initiated.
       */
      bool CallWithCallback(const std::string& method_name_, const google::protobuf::Message& request_, const ResponseCallbackT& response_callback_,
        int timeout_ms_ = eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT)
      {
        return m_instance.CallWithCallback(method_name_, request_.SerializeAsString(), response_callback_, timeout_ms_);
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
       * @return  True if the call was successfully initiated.
       */
      template <typename ResponseT>
      bool CallWithCallbackAsync(const std::string& method_name_, const google::protobuf::Message& request_, const TMsgResponseCallbackT<ResponseT>& response_callback_)
      {
        auto wrapper = [this, method_name_, response_callback_](const SServiceResponse& service_response)
          {
            TMsgServiceResponse<ResponseT> msg_response;
            ProcessResponse<ResponseT>(method_name_, service_response, msg_response);
            response_callback_(msg_response);
          };

        return m_instance.CallWithCallbackAsync(method_name_, request_.SerializeAsString(), wrapper);
      }

      /**
       * @brief Asynchronous call with callback.
       *
       * @param  method_name_        The name of the service method.
       * @param  request_            The request message.
       * @param  response_callback_  Callback accepting a typed response.
       *
       * @return  True if the call was successfully initiated.
       */
      bool CallWithCallbackAsync(const std::string& method_name_, const google::protobuf::Message& request_, const ResponseCallbackT& response_callback_)
      {
        return m_instance.CallWithCallbackAsync(method_name_, request_.SerializeAsString(), response_callback_);
      }

      /**
       * @brief Get unique client entity id.
       *
       * @return  The client entity id.
       **/
      SEntityId GetClientID() const
      {
        return m_instance.GetClientID();
      }

    private:
      // The underlying base client instance.
      eCAL::CClientInstance m_instance;

      /**
       * @brief Helper function to process a generic SServiceResponse into a typed TMsgServiceResponse.
       *
       * This function encapsulates the common code for:
       *   - Retrieving the service descriptor.
       *   - Finding the method descriptor.
       *   - Obtaining the prototype.
       *   - Creating a new message and parsing it.
       *   - Converting the generic response into the expected type via dynamic_cast.
       * 
       *
       * @tparam ResponseT              Expected protobuf response type.
       * 
       * @param  method_name_           The name of the method that was called.
       * @param  generic_response_      The generic SServiceResponse returned by the underlying call.
       * @param  typed_response_ (out)  The typed response to fill.
       * 
       * @return  True if processing succeeded, false otherwise.
       */
      template <typename ResponseT>
      bool ProcessResponse(const std::string& method_name_, const SServiceResponse& generic_response_, TMsgServiceResponse<ResponseT>& typed_response_)
      {
        // Retrieve the service descriptor from the templated service type.
        const google::protobuf::ServiceDescriptor* service_descriptor = GetServiceDescriptor();
        if (!service_descriptor)
        {
          typed_response_.error_msg = "Failed to get service descriptor";
          return false;
        }

        // Find the method descriptor.
        const google::protobuf::MethodDescriptor* method_descriptor = service_descriptor->FindMethodByName(method_name_);
        if (!method_descriptor)
        {
          typed_response_.error_msg = "Method not found in service descriptor";
          return false;
        }

        // Get the prototype for the response.
        const google::protobuf::Message* prototype =
          google::protobuf::MessageFactory::generated_factory()->GetPrototype(method_descriptor->output_type());
        if (!prototype)
        {
          typed_response_.error_msg = "Failed to get prototype for response";
          return false;
        }

        // Create a new mutable message instance.
        std::unique_ptr<google::protobuf::Message> generic_msg(prototype->New());
        if (!generic_msg || !generic_msg->ParseFromString(generic_response_.response))
        {
          typed_response_.error_msg = "Failed to parse response";
          return false;
        }

        // Attempt to convert to the expected type.
        ResponseT* typed_ptr = dynamic_cast<ResponseT*>(generic_msg.get());
        if (!typed_ptr)
        {
          typed_response_.error_msg = "Response type conversion failed";
          return false;
        }

        // Transfer ownership to the typed response.
        generic_msg.release();
        typed_response_.response.reset(typed_ptr);

        // Fill the typed response attributes.
        typed_response_.server_id                  = generic_response_.server_id;
        typed_response_.service_method_information = generic_response_.service_method_information;
        typed_response_.call_state                 = generic_response_.call_state;
        typed_response_.ret_state                  = generic_response_.ret_state;
        typed_response_.error_msg                  = generic_response_.error_msg;
        return true;
      }

      /**
       * @brief Helper to retrieve the service descriptor from the templated service type.
       *
       * This instantiates a temporary subclass of T to expose its protected GetDescriptor() method.
       *
       * @return  Pointer to the service descriptor.
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
