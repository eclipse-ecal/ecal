/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * @file   ecal_server_v5.h
 * @brief  eCAL server interface (deprecated eCAL5 version)
**/

#pragma once

#include <ecal/ecal_deprecate.h>
#include <ecal/ecal_os.h>
#include <ecal/ecal_callback.h>
#include <ecal/ecal_service_info.h>

#include <string>
#include <vector>
#include <memory>

namespace eCAL
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
     *
     * @param service_name_   Unique service name.
    **/
    ECAL_API_EXPORTED_MEMBER
      explicit CServiceServer(const std::string& service_name_, const ServerEventCallbackT callback_ = ServerEventCallbackT());

    /**
     * @brief Destructor.
    **/
    ECAL_API_EXPORTED_MEMBER
      virtual ~CServiceServer();

    // Deleted copy constructor and copy assignment operator
    CServiceServer(const CServiceServer&) = delete;
    CServiceServer& operator=(const CServiceServer&) = delete;

    // Move constructor and move assignment operator
    ECAL_API_EXPORTED_MEMBER CServiceServer(CServiceServer&& rhs) noexcept;
    ECAL_API_EXPORTED_MEMBER CServiceServer& operator=(CServiceServer&& rhs) noexcept;

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
      bool AddMethodCallback(const std::string& method_, const SServiceMethodInformation& method_info_, const MethodCallbackT& callback_);

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
