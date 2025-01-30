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
 * @brief  eCAL Client interface based on protobuf service description
**/

#pragma once

#include <ecal/deprecate.h>
#include <ecal/service/client.h>
#include <ecal/msg/protobuf/ecal_proto_dyn.h>

// protobuf includes
#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#include <google/protobuf/descriptor.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

// stl includes
#include <string>
#include <functional>
#include <map>

namespace eCAL
{
  namespace protobuf
  {
    /**
     * @brief Google Protobuf Client wrapper class.
    **/
    template <typename T>
    class CServiceClient : public eCAL::CServiceClient
    {
    public:
      /**
       * @brief Constructor (using protobuf-defined service name).
      **/
      CServiceClient(const ClientEventCallbackT& event_callback_ = ClientEventCallbackT())
        : eCAL::CServiceClient(GetServiceNameFromDescriptor(), CreateServiceMethodInformationSet(), event_callback_)
      {
      }

      /**
       * @brief Constructor.
       *
       * @param service_name_  Unique service name.
      **/
      explicit CServiceClient(const std::string& service_name_, const ClientEventCallbackT& event_callback_ = ClientEventCallbackT())
        : eCAL::CServiceClient(service_name_, CreateServiceMethodInformationSet(), event_callback_)
      {
      }

      /**
       * @brief CServiceClients are non-copyable.
      **/
      CServiceClient(const CServiceClient&) = delete;
      CServiceClient& operator=(const CServiceClient&) = delete;

      /**
       * @brief Blocking call of a service method for all existing service instances, response will be returned as ServiceResponseVecT
       *
       * @param       method_name_           Method name.
       * @param       request_               Request message.
       * @param       timeout_               Maximum time before operation returns (in milliseconds, -1 means infinite).
       * @param [out] service_response_vec_  Response vector containing service responses from every called service.
       *
       * @return  True if all calls were successful.
      **/
      bool CallWithResponse(const std::string& method_name_, const google::protobuf::Message& request_, int timeout_, ServiceResponseVecT& service_response_vec_)
      {
        return CallWithResponse(method_name_, request_.SerializeAsString(), timeout_, service_response_vec_);
      }

      /**
       * @brief Blocking call (with timeout) of a service method for all existing service instances, using callback
       *
       * @param method_name_        Method name.
       * @param request_            Request message.
       * @param timeout_            Maximum time before operation returns (in milliseconds, -1 means infinite).
       * @param response_callback_  Callback function for the service method response.
       *
       * @return  True if all calls were successful.
      **/
      bool CallWithCallback(const std::string& method_name_, const google::protobuf::Message& request_, int timeout_, const ResponseCallbackT& response_callback_)
      {
        return CallWithCallback(method_name_, request_.SerializeAsString(), timeout_, response_callback_);
      }

      /**
       * @brief Asynchronous call of a service method for all existing service instances, using callback
       *
       * @param method_name_        Method name.
       * @param request_            Request message.
       * @param response_callback_  Callback function for the service method response.
       *
       * @return  True if all calls were successful.
      **/
      bool CallWithCallbackAsync(const std::string& method_name_, const google::protobuf::Message& request_, const ResponseCallbackT& response_callback_)
      {
        return CallWithCallbackAsync(method_name_, request_.SerializeAsString(), response_callback_);
      }

      using eCAL::CServiceClient::CallWithResponse;
      using eCAL::CServiceClient::CallWithCallback;
      using eCAL::CServiceClient::CallWithCallbackAsync;

    private:
      /**
       * @brief Retrieves the full service name from the Protobuf descriptor.
      **/
      static std::string GetServiceNameFromDescriptor()
      {
        struct U : T {};  // Temporary subclass to access protected descriptor method
        U temp_instance;
        return temp_instance.GetDescriptor()->full_name();
      }

      /**
       * @brief Generates the method information map for the service.
       * @return A set containing service method information.
      **/
      static ServiceMethodInformationSetT CreateServiceMethodInformationSet()
      {
        struct U : T {};  // Temporary subclass to access protected descriptor method
        U temp_instance;
        const google::protobuf::ServiceDescriptor* service_descriptor = temp_instance.GetDescriptor();

        if (!service_descriptor)
        {
          throw std::runtime_error("Failed to retrieve service descriptor.");
        }

        ServiceMethodInformationSetT method_information_map;
        CProtoDynDecoder dyn_decoder;
        std::string error_s;

        for (int i = 0; i < service_descriptor->method_count(); ++i)
        {
          const google::protobuf::MethodDescriptor* method_descriptor = service_descriptor->method(i);
          std::string method_name = method_descriptor->name();

          std::string request_type_name  = method_descriptor->input_type()->name();
          std::string response_type_name = method_descriptor->output_type()->name();

          std::string request_type_descriptor;
          std::string response_type_descriptor;

          dyn_decoder.GetServiceMessageDescFromType(service_descriptor, request_type_name, request_type_descriptor, error_s);
          dyn_decoder.GetServiceMessageDescFromType(service_descriptor, response_type_name, response_type_descriptor, error_s);

          method_information_map.emplace(SServiceMethodInformation{
            method_name,
            SDataTypeInformation{request_type_name,  "proto", request_type_descriptor},
            SDataTypeInformation{response_type_name, "proto", response_type_descriptor}
            });
        }

        return method_information_map;
      }
    };
  }
}
