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
 * @file   ecal_server.h
 * @brief  eCAL service interface
**/

#pragma once

#include <ecal/ecal_os.h>
#include <ecal/ecal_callback.h>
#include <ecal/ecal_service_info.h>

#include <string>
#include <vector>

namespace eCAL
{
  class CServiceServerImpl;

  /**
   * @brief Service Server wrapper class.
  **/
  class ECAL_API CServiceServer
  {
  public:
    /**
     * @brief Constructor. 
    **/
    CServiceServer();

    /**
     * @brief Constructor. 
     *
     * @param service_name_   Unique service name.
    **/
    CServiceServer(const std::string& service_name_);

    /**
     * @brief Destructor. 
    **/
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
    bool Create(const std::string& service_name_);

    /**
     * @brief Destroys this object. 
     *
     * @return  True if successful. 
    **/
    bool Destroy();

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
    bool AddMethodCallback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const MethodCallbackT& callback_);

    /**
     * @brief Remove method callback.
     *
     * @param method_  Service method name.
     *
     * @return  True if successful.
    **/
    bool RemMethodCallback(const std::string& method_);

    /**
     * @brief Add server event callback function.
     *
     * @param type_      The event type to react on.
     * @param callback_  The callback function to add.
     *
     * @return  True if succeeded, false if not.
    **/
    bool AddEventCallback(eCAL_Server_Event type_, ServerEventCallbackT callback_);

    /**
     * @brief Remove server event callback function.
     *
     * @param type_  The event type to remove.
     *
     * @return  True if succeeded, false if not.
    **/
    bool RemEventCallback(eCAL_Server_Event type_);

    /**
     * @brief Retrieve service name.
     *
     * @return  The service name.
    **/
    std::string GetServiceName();

    /**
     * @brief Check connection state.
     *
     * @return  True if connected, false if not.
    **/
    bool IsConnected();

  protected:
    CServiceServerImpl*  m_service_server_impl;
    bool                 m_created;
  };
} 
