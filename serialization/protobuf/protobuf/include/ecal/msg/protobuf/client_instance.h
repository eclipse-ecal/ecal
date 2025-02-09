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
        bool success = ResponseParser::ParseResponse<ResponseT>(GetServiceDescriptor(), method_name_, ret.second, msg_response);

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
            if (ResponseParser::ParseResponse<ResponseT>(GetServiceDescriptor(), method_name_, service_response, msg_response))
            {
              response_callback_(msg_response);
            }
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
            if (ResponseParser::ParseResponse<ResponseT>(GetServiceDescriptor(), method_name_, service_response, msg_response))
            {
              response_callback_(msg_response);
            }
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
