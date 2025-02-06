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
 * @brief  eCAL client interface based on protobuf service description
**/

#pragma once

#include <ecal/deprecate.h>
#include <ecal/service/client.h>
#include <ecal/msg/protobuf/ecal_proto_dyn.h>
#include <ecal/msg/protobuf/client_instance.h>  // our new templated CClientInstance<T>

#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#include <google/protobuf/descriptor.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <string>
#include <functional>
#include <map>
#include <vector>

namespace eCAL
{
  namespace protobuf
  {
    /**
     * @brief Google Protobuf Client wrapper class.
     *
     * This class is templated on the service type T
     * and automatically generates method information from the service descriptor.
     *
     * In addition to its base functionality, it now provides typed call overloads
     * so that the responses can be obtained in a type‐safe way (using TMsgServiceResponse<ResponseT>).
     */
    template <typename T>
    class CServiceClient : public eCAL::CServiceClient
    {
    public:
      /**
       * @brief Constructor (using protobuf‐defined service name).
       */
      CServiceClient(const ClientEventCallbackT& event_callback_ = ClientEventCallbackT())
        : eCAL::CServiceClient(GetServiceNameFromDescriptor(), CreateServiceMethodInformationSet(), event_callback_)
      {
      }

      /**
       * @brief Constructor.
       *
       * @param service_name_ Unique service name.
       */
      explicit CServiceClient(const std::string& service_name_, const ClientEventCallbackT& event_callback_ = ClientEventCallbackT())
        : eCAL::CServiceClient(service_name_, CreateServiceMethodInformationSet(), event_callback_)
      {
      }

      /**
       * @brief Destructor.
      **/
      ~CServiceClient() override = default;

      /**
       * @brief CServiceClients are non-movable.
      **/
      CServiceClient(CServiceClient&& other) = delete;
      CServiceClient& operator=(CServiceClient&& other) = delete;

      /**
       * @brief CServiceClients are non-copyable.
      **/
      CServiceClient(const CServiceClient&) = delete;
      CServiceClient& operator=(const CServiceClient&) = delete;

      /**
       * @brief Get the client instances for all matching services as our templated, typed instances.
       *
       * @return A vector of CClientInstance<T>.
       */
      std::vector<CClientInstance<T>> GetClientInstances() const
      {
        // Get the base client instances.
        std::vector<eCAL::CClientInstance> base_instances = eCAL::CServiceClient::GetClientInstances();

        std::vector<CClientInstance<T>> proto_instances;
        proto_instances.reserve(base_instances.size());

        // Wrap each base instance into our protobuf-enhanced instance.
        for (auto& inst : base_instances)
        {
          // Use the conversion constructor (which moves the base instance).
          proto_instances.push_back(CClientInstance<T>(std::move(inst)));
        }
        return proto_instances;
      }

      /**
       * @brief Blocking call of a service method for all matching service instances, returning typed responses.
       *
       * @tparam ResponseT     Expected protobuf response type (e.g. SFloat).
       * 
       * @param  method_name_  Method name.
       * @param  request_      Request message.
       * @param  timeout_      Timeout in milliseconds (DEFAULT_TIME_ARGUMENT for infinite).
       *
       * @return A pair where the first element is true if all calls succeeded and the second element
       *         is a vector of typed responses.
       */
      template <typename ResponseT>
      std::pair<bool, TMsgServiceResponseVecT<ResponseT>> CallWithResponse( const std::string& method_name_, const google::protobuf::Message& request_,
        int timeout_ = DEFAULT_TIME_ARGUMENT) const
      {
        bool overall_success = true;
        TMsgServiceResponseVecT<ResponseT> responses;

        // Get all client instances.
        auto instances = GetClientInstances();
        for (auto& instance : instances)
        {
          auto ret = instance.template CallWithResponse<ResponseT>(method_name_, request_, timeout_);
          responses.push_back(std::move(ret.second));
          if (!ret.first)
          {
            overall_success = false;
          }
        }
        return std::pair<bool, TMsgServiceResponseVecT<ResponseT>>(overall_success, responses);
      }

      /**
       * @brief Blocking call of a service method for all matching service instances.
       *
       * @param  method_name_  Method name.
       * @param  request_      Request message.
       * @param  timeout_      Timeout in milliseconds (DEFAULT_TIME_ARGUMENT for infinite).
       *
       * @return A pair where the first element is true if all calls succeeded and the second element
       *         is a vector of responses.
       */
      std::pair<bool, ServiceResponseVecT> CallWithResponse(const std::string& method_name_, const google::protobuf::Message& request_,
        int timeout_ = DEFAULT_TIME_ARGUMENT) const
      {
        bool overall_success = true;
        ServiceResponseVecT responses;

        // Get all client instances.
        auto instances = GetClientInstances();
        for (auto& instance : instances)
        {
          auto ret = instance.CallWithResponse(method_name_, request_, timeout_);
          responses.push_back(std::move(ret.second));
          if (!ret.first)
          {
            overall_success = false;
          }
        }
        return std::pair<bool, ServiceResponseVecT>(overall_success, responses);
      }

      /**
       * @brief Blocking call with callback for all matching service instances, providing typed responses.
       *
       * @tparam ResponseT           Expected protobuf response type.
       * 
       * @param  method_name_        Method name.
       * @param  request_            Request message.
       * @param  response_callback_  Callback that accepts a typed response.
       * @param  timeout_            Timeout in milliseconds.
       *
       * @return True if the call was successfully initiated for all instances.
       */
      template <typename ResponseT>
      bool CallWithCallback(const std::string& method_name_, const google::protobuf::Message& request_, const TMsgResponseCallbackT<ResponseT>& response_callback_,
        int timeout_ = DEFAULT_TIME_ARGUMENT) const
      {
        bool overall_success = true;
        auto instances = GetClientInstances();
        for (auto& instance : instances)
        {
          bool success = instance.template CallWithCallback<ResponseT>(method_name_, request_, response_callback_, timeout_);
          if (!success) overall_success = false;
        }
        return overall_success;
      }

      /**
       * @brief Blocking call with callback for all matching service instances.
       *
       * @param  method_name_        Method name.
       * @param  request_            Request message.
       * @param  response_callback_  Callback that accepts a typed response.
       * @param  timeout_            Timeout in milliseconds.
       *
       * @return True if the call was successfully initiated for all instances.
       */
      bool CallWithCallback(const std::string& method_name_, const google::protobuf::Message& request_, const ResponseCallbackT& response_callback_,
        int timeout_ = DEFAULT_TIME_ARGUMENT) const
      {
        bool overall_success = true;
        auto instances = GetClientInstances();
        for (auto& instance : instances)
        {
          bool success = instance.CallWithCallback(method_name_, request_, response_callback_, timeout_);
          if (!success) overall_success = false;
        }
        return overall_success;
      }

      /**
       * @brief Asynchronous call with callback for all matching service instances, providing typed responses.
       *
       * @tparam ResponseT           Expected protobuf response type.
       * 
       * @param  method_name_        Method name.
       * @param  request_            Request message.
       * @param  response_callback_  Callback that accepts a typed response.
       *
       * @return True if the call was successfully initiated for all instances.
       */
      template <typename ResponseT>
      bool CallWithCallbackAsync(const std::string& method_name_,
        const google::protobuf::Message& request_,
        const TMsgResponseCallbackT<ResponseT>& response_callback_) const
      {
        bool overall_success = true;
        auto instances = GetClientInstances();
        for (auto& instance : instances)
        {
          bool success = instance.template CallWithCallbackAsync<ResponseT>(method_name_, request_, response_callback_);
          if (!success) overall_success = false;
        }
        return overall_success;
      }

      // Optionally, also expose the base class overloads.
      using eCAL::CServiceClient::CallWithResponse;
      using eCAL::CServiceClient::CallWithCallback;
      using eCAL::CServiceClient::CallWithCallbackAsync;

    private:
      /**
       * @brief Retrieves the full service name from the Protobuf descriptor.
       */
      static std::string GetServiceNameFromDescriptor()
      {
        struct U : T {};  // Temporary subclass to access protected GetDescriptor()
        U temp_instance;
        return temp_instance.GetDescriptor()->full_name();
      }

      /**
       * @brief Generates the method information set for the service.
       *
       * @return A set containing method information.
       */
      static ServiceMethodInformationSetT CreateServiceMethodInformationSet()
      {
        struct U : T {};  // Temporary subclass to access protected GetDescriptor()
        U temp_instance;
        const google::protobuf::ServiceDescriptor* service_descriptor = temp_instance.GetDescriptor();

        if (service_descriptor == nullptr)
        {
          throw std::runtime_error("Failed to retrieve service descriptor.");
        }

        ServiceMethodInformationSetT method_information_map;
        CProtoDynDecoder dyn_decoder;
        std::string error_s;

        for (int i = 0; i < service_descriptor->method_count(); ++i)
        {
          const google::protobuf::MethodDescriptor* method_descriptor = service_descriptor->method(i);
          const std::string& method_name = method_descriptor->name();

          const std::string& request_type_name = method_descriptor->input_type()->name();
          const std::string& response_type_name = method_descriptor->output_type()->name();

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
  } // namespace protobuf
} // namespace eCAL
