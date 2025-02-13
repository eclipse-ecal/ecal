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
 * @file   msg/protobuf/server.h
 * @brief  eCAL Server interface based on protobuf service description
 *
 * This header defines the CServiceServer class, a Google Protocol Buffers server wrapper
 * for eCAL. The class automatically extracts service method information from a provided
 * protobuf service instance and registers all available methods with the underlying eCAL
 * service server infrastructure.
 *
 * Key Features:
 *  - Automatic extraction of the full service name from the protobuf descriptor.
 *  - Dynamic generation of method information (input/output types and their descriptors)
 *    using the CProtoDynDecoder.
 *  - Registration of all protobuf-defined methods as callbacks with the underlying
 *    eCAL::CServiceServer.
 *  - Handling of incoming requests by deserializing the request message, invoking
 *    the appropriate service method, and serializing the response.
 *
 * Usage Example:
 *   // Create a protobuf service implementation instance (e.g., MathServiceImpl)
 *   std::shared_ptr<MathServiceImpl> service_impl = std::make_shared<MathServiceImpl>();
 *
 *   // Create a server for the protobuf service
 *   eCAL::protobuf::CServiceServer<MathService> server(service_impl);
 *
 *   // The server automatically registers all methods and is ready to process incoming requests.
 *
 * @note The service instance must provide access to its protobuf ServiceDescriptor (typically
 *       via a GetDescriptor() method) so that the server can correctly extract method information.
 */

#pragma once

#include <ecal/service/server.h>
#include <ecal/msg/protobuf/ecal_proto_dyn.h>
#include <functional>

// protobuf includes
#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#include <google/protobuf/descriptor.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

// stl includes
#include <iostream>
#include <map>
#include <memory>
#include <string>

namespace eCAL
{
  namespace protobuf
  {
    /**
     * @brief Google Protobuf Server wrapper class.
    **/
    template <typename T>
    class CServiceServer : public eCAL::CServiceServer
    {
    public:
      /**
       * @brief Constructor.
       *
       * @param service_        Google protobuf service instance.
       * @param event_callback_ Callback function for server events.
      **/
      CServiceServer(std::shared_ptr<T> service_, const ServerEventCallbackT& event_callback_ = ServerEventCallbackT())
        : eCAL::CServiceServer(GetServiceNameFromDescriptor(service_), event_callback_), m_service(service_)
      {
        RegisterMethods();
      }

      /**
       * @brief Constructor.
       *
       * @param service_        Google protobuf service instance.
       * @param service_name_   Unique service name.
       * @param event_callback_ Callback function for server events.
      **/
      CServiceServer(std::shared_ptr<T> service_, const std::string& service_name_, const ServerEventCallbackT& event_callback_ = ServerEventCallbackT())
        : eCAL::CServiceServer(service_name_.empty() ? GetServiceNameFromDescriptor(service_) : service_name_, event_callback_),
        m_service(service_)
      {
        RegisterMethods();
      }

      /**
       * @brief Destructor.
      **/
      ~CServiceServer() override = default;

      /**
       * @brief CServiceServers are non-movable.
      **/
      CServiceServer(CServiceServer&& other) = delete;
      CServiceServer& operator=(CServiceServer&& other) = delete;

      /**
       * @brief CServiceServers are non-copyable.
      **/
      CServiceServer(const CServiceServer&) = delete;
      CServiceServer& operator=(const CServiceServer&) = delete;

    private:
      using eCAL::CServiceServer::SetMethodCallback;
      using eCAL::CServiceServer::RemoveMethodCallback;
      using eCAL::CServiceServer::GetServiceName;

      /**
       * @brief Retrieves the full service name from the Protobuf descriptor.
      **/
      static std::string GetServiceNameFromDescriptor(std::shared_ptr<T> service_)
      {
        if (!service_)
          throw std::runtime_error("CServiceServer: Null service instance.");

        return service_->GetDescriptor()->full_name();
      }

      /**
       * @brief Registers all Protobuf methods as service callbacks.
      **/
      void RegisterMethods()
      {
        if (!m_service)
          return;

        const google::protobuf::ServiceDescriptor* service_descriptor = m_service->GetDescriptor();
        CProtoDynDecoder dyn_decoder;
        std::string error_s;

        for (int i = 0; i < service_descriptor->method_count(); ++i)
        {
          const google::protobuf::MethodDescriptor* method_descriptor = service_descriptor->method(i);
          std::string method_name = method_descriptor->name();
          m_methods[method_name] = method_descriptor;

          std::string input_type_desc, output_type_desc;
          dyn_decoder.GetServiceMessageDescFromType(service_descriptor, method_descriptor->input_type()->name(), input_type_desc, error_s);
          dyn_decoder.GetServiceMessageDescFromType(service_descriptor, method_descriptor->output_type()->name(), output_type_desc, error_s);

          SServiceMethodInformation method_info{
            method_name,
            method_descriptor->input_type()->name(),
            method_descriptor->output_type()->name(),
            input_type_desc,
            output_type_desc
          };

          SetMethodCallback(method_info,
            [this, method_name](const SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_)
            {
              return this->RequestCallback(method_info_, request_, response_);
            });
        }
      }

      /**
       * @brief Handles service requests.
      **/
      int RequestCallback(const SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_)
      {
        if (!m_service)
          return -1;

        auto iter = m_methods.find(method_info_.method_name);
        if (iter == m_methods.end())
          return -1;

        const google::protobuf::MethodDescriptor* method_descriptor = iter->second;

        // Use smart pointers to ensure proper cleanup
        std::unique_ptr<google::protobuf::Message> request(m_service->GetRequestPrototype(method_descriptor).New());
        if (!request->ParseFromString(request_))
        {
          std::cerr << "Protobuf Service " << GetServiceName() << " failed to parse request message!" << std::endl;
          return -1;
        }

        std::unique_ptr<google::protobuf::Message> response(m_service->GetResponsePrototype(method_descriptor).New());

        // Call the actual method
        m_service->CallMethod(method_descriptor, nullptr, request.get(), response.get(), nullptr);
        response_ = response->SerializeAsString();

        return 0;
      }

      std::shared_ptr<T>                                               m_service;
      std::map<std::string, const google::protobuf::MethodDescriptor*> m_methods;
    };

  } // namespace protobuf
} // namespace eCAL
