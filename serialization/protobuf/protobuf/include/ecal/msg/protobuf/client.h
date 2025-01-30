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
#include <ecal/v5/ecal_client.h>
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
    class CServiceClient : public eCAL::v5::CServiceClient
    {
    public:
      /**
       * @brief Constructor (using protobuf defined service name).
       *
      **/
      CServiceClient()
      {
        // As google::protobuf::Service::GetDescriptor() is defined in a protected class scope
        // we need to inherit public from T in order to make the method accessible in our code.
        struct U : T {};
        std::shared_ptr<U> service = std::make_shared<U>();
        const google::protobuf::ServiceDescriptor* service_descriptor = service->GetDescriptor();

        Create(service_descriptor->full_name(), CreateMethodInformationMap());
      }

      /**
       * @brief Constructor.
       *
       * @param service_name_  Unique service name.
      **/
      explicit CServiceClient(const std::string& service_name_)
      {
        Create(service_name_, CreateMethodInformationMap());
      }

      /**
       * @brief CServiceClients are non-copyable
      **/
      CServiceClient(const CServiceClient&) = delete;

      /**
       * @brief CServiceClients are non-copyable
      **/
      CServiceClient& operator=(const CServiceClient&) = delete;

      /**
       * @brief Call a method of this service, responses will be returned by callback. 
       *
       * @param method_name_  Method name.
       * @param request_      Request message.
       * @param timeout_      Maximum time before operation returns (in milliseconds, -1 means infinite).
       *
       * @return  True if successful.
      **/
      bool Call(const std::string& method_name_, const google::protobuf::Message& request_, const int timeout_ = -1)
      {
        return Call(method_name_, request_.SerializeAsString(), timeout_);
      }

      /**
       * @brief Call a method of this service, all responses will be returned in service_response_vec.
       *
       * @param       method_name_           Method name.
       * @param       request_               Request message.
       * @param       timeout_               Maximum time before operation returns (in milliseconds, -1 means infinite).
       * @param [out] service_response_vec_  Response vector containing service responses from every called service (null pointer == no response).
       *
       * @return  True if successful.
      **/
      bool Call(const std::string& method_name_, const google::protobuf::Message& request_, const int timeout_, v5::ServiceResponseVecT* service_response_vec_)
      {
        return Call(method_name_, request_.SerializeAsString(), timeout_, service_response_vec_);
      }

      /**
       * @brief Call a method of this service asynchronously, responses will be returned by callback.
       *
       * @param method_name_  Method name.
       * @param request_      Request message.
       * @param timeout_      Maximum time before operation returns (in milliseconds, -1 means infinite).
       *
       * @return  True if successful.
      **/
      bool CallAsync(const std::string& method_name_, const google::protobuf::Message& request_, const int timeout_ = -1)
      {
        return CallAsync(method_name_, request_.SerializeAsString(), timeout_);
      }

      using eCAL::v5::CServiceClient::Call;
      using eCAL::v5::CServiceClient::CallAsync;
    private:
      ServiceMethodInformationSetT CreateMethodInformationMap()
      {
        // As google::protobuf::Service::GetDescriptor() is defined in a protected class scope
        // we need to inherit public from T in order to make the method accessible in our code.
        struct U : T {};
        std::shared_ptr<U> service = std::make_shared<U>();
        const google::protobuf::ServiceDescriptor* service_descriptor = service->GetDescriptor();

        std::string error_s;
        ServiceMethodInformationSetT method_information_map;
        CProtoDynDecoder dyn_decoder;
        for (int i = 0; i < service_descriptor->method_count(); ++i)
        {
          // get method name and descriptor
          const google::protobuf::MethodDescriptor* method_descriptor = service_descriptor->method(i);
          const std::string method_name = method_descriptor->name();

          // get message type names
          const std::string request_type_name = method_descriptor->input_type()->name();
          const std::string response_type_name = method_descriptor->output_type()->name();

          // get message type descriptors
          std::string request_type_descriptor;
          std::string response_type_descriptor;

          dyn_decoder.GetServiceMessageDescFromType(service_descriptor, request_type_name, request_type_descriptor, error_s);
          dyn_decoder.GetServiceMessageDescFromType(service_descriptor, response_type_name, response_type_descriptor, error_s);

          method_information_map.emplace(SServiceMethodInformation{ method_name,
            SDataTypeInformation{request_type_name, "proto", request_type_descriptor},
            SDataTypeInformation{response_type_name, "proto", response_type_descriptor}
            }
          );
        }

        return method_information_map;
      }
    };
  }
}
