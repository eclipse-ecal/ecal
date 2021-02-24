/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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

#include <ecal/ecal_client.h>
#include <ecal/protobuf/ecal_proto_hlp.h>

// protobuf includes
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4800) // disable proto warnings
#endif
#include <google/protobuf/descriptor.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

// stl includes
#include <string>

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
       * @brief Constructor.
       *
      **/
      CServiceClient()
      {
        Create(T::descriptor()->full_name());
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
       * @brief Call method of this service, for all hosts, responses will be returned by callback. 
       *
       * @param method_name_  Method name.
       * @param request_      Request message.
       *
       * @return  True if successful.
      **/
      bool Call(const std::string& method_name_, const google::protobuf::Message& request_)
      {
        return Call(method_name_, request_.SerializeAsString());
      }

      /**
       * @brief Call method of this service, for specific host.
       *
       * @param       host_name_     Host name.
       * @param       method_name_   Method name.
       * @param       request_       Request message.
       * @param [out] service_info_  Service info struct for detailed informations.
       * @param [out] response_      Response string.
       *
      * @return  True if successful.
      **/
      bool Call(const std::string& host_name_, const std::string& method_name_, const google::protobuf::Message& request_, struct SServiceInfo& service_info_, google::protobuf::Message& response_)
      {
        std::string response_s;
        bool success = Call(host_name_, method_name_, request_.SerializeAsString(), service_info_, response_s);
        if (success)
        {
          response_.ParseFromString(response_s);
        }
        return success;
      }

      /**
       * @brief Asynchronously call method of this service, for all hosts, responses will be returned by callback. 
       *
       * @param method_name_  Method name. 
       * @param request_      Request message. 
      **/
      void CallAsync(const std::string& method_name_, const google::protobuf::Message& request_)
      {
        CallAsync(method_name_, request_.SerializeAsString());
      }

      /**
       * @brief Asynchronously call method of this service asynchronously, for specific host, response will be returned by callback. 
       *
       * @param       host_name_     Host name.
       * @param       method_name_   Method name.
       * @param       request_       Request message.
      **/
      void CallAsync(const std::string& host_name_, const std::string& method_name_, const google::protobuf::Message& request_)
      {
        CallAsync(host_name_, method_name_, request_.SerializeAsString());
      }

      using eCAL::CServiceClient::Call;
      using eCAL::CServiceClient::CallAsync;
    };
  }
}
