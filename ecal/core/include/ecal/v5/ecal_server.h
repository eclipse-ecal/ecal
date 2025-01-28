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
 * @file   v5/ecal_server.h
 * @brief  eCAL server interface (deprecated eCAL5 version)
**/

#pragma once

#include <ecal/namespace.h>

#include <ecal/deprecate.h>
#include <ecal/os.h>
#include <ecal/v5/ecal_callback.h>
#include <ecal/service/types.h>

#include <memory>
#include <string>
#include <vector>

namespace eCAL
{
  ECAL_CORE_NAMESPACE_V5
  {
    class CServiceServerImpl;

    /**
     * @brief Service Server wrapper class.
    **/
    class ECAL_API_CLASS CServiceServer
    {
    public:
      /**
       * @brief Constructor. 
      **/
      ECAL_API_EXPORTED_MEMBER
        CServiceServer();

      /**
       * @brief Constructor. 
       *
       * @param service_name_   Unique service name.
      **/
      ECAL_API_EXPORTED_MEMBER
        explicit CServiceServer(const std::string& service_name_);

      /**
       * @brief Destructor. 
      **/
      ECAL_API_EXPORTED_MEMBER
        virtual ~CServiceServer();

      /**
       * @brief CServiceServers are non-copyable
      **/
      CServiceServer(const CServiceServer&) = delete;

      /**
       * @brief CServiceServers are non-copyable
      **/
      CServiceServer& operator=(const CServiceServer&) = delete;

      /**
       * @brief Creates this object. 
       *
       * @param service_name_   Unique service name.
       *
       * @return  True if successful. 
      **/
      ECAL_API_EXPORTED_MEMBER
        bool Create(const std::string& service_name_);

      /**
       * @brief Destroys this object. 
       *
       * @return  True if successful. 
      **/
      ECAL_API_EXPORTED_MEMBER
        bool Destroy();

      /**
       * @brief Add method type descriptions.
       *
       * @param method_     Service method name.
       * @param req_type_   Service method request type.
       * @param req_desc_   Service method request description.
       * @param resp_type_  Service method response type.
       * @param resp_desc_  Service method response description.
       *
       * @return  True if successful.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool AddDescription(const std::string& method_, const std::string& req_type_, const std::string& req_desc_, const std::string& resp_type_, const std::string& resp_desc_);

      /**
       * @brief Add method callback.
       *
       * @param method_     Service method name.
       * @param req_type_   Service method request type.
       * @param resp_type_  Service method response type.
       * @param callback_   Callback function for client request.
       *
       * @return  True if successful.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool AddMethodCallback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const MethodCallbackT& callback_);

      /**
       * @brief Remove method callback.
       *
       * @param method_  Service method name.
       *
       * @return  True if successful.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool RemMethodCallback(const std::string& method_);

      /**
       * @brief Add server event callback function.
       *
       * @param type_      The event type to react on.
       * @param callback_  The callback function to add.
       *
       * @return  True if succeeded, false if not.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool AddEventCallback(eServerEvent type_, ServerEventCallbackT callback_);

      /**
       * @brief Remove server event callback function.
       *
       * @param type_  The event type to remove.
       *
       * @return  True if succeeded, false if not.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool RemEventCallback(eServerEvent type_);

      /**
       * @brief Retrieve service name.
       *
       * @return  The service name.
      **/
      ECAL_API_EXPORTED_MEMBER
        std::string GetServiceName();

      /**
       * @brief Check connection state.
       *
       * @return  True if connected, false if not.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool IsConnected();

    private:
      std::shared_ptr<CServiceServerImpl> m_service_server_impl;
    };
  }
} 
