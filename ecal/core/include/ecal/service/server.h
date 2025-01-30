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
 * @file   service/server.h
 * @brief  eCAL server interface
**/

#pragma once

#include <ecal/deprecate.h>
#include <ecal/namespace.h>
#include <ecal/os.h>

#include <ecal/service/types.h>

#include <memory>
#include <string>

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
     * @param event_callback_ Callback function for server events.
    **/
    ECAL_API_EXPORTED_MEMBER
      CServiceServer(const std::string& service_name_, const ServerEventCallbackT& event_callback_ = ServerEventCallbackT());

    /**
     * @brief Destructor.
    **/
    ECAL_API_EXPORTED_MEMBER
      virtual ~CServiceServer();

    /**
     * @brief CServiceServer are non-copyable
    **/
    CServiceServer(const CServiceServer&) = delete;

    /**
     * @brief CServiceServer are non-copyable
    **/
    CServiceServer& operator=(const CServiceServer&) = delete;

    /**
     * @brief CServiceServer are move-enabled
    **/
    ECAL_API_EXPORTED_MEMBER
      CServiceServer(CServiceServer&& rhs) noexcept;

    /**
     * @brief CServiceServer are move-enabled
    **/
    ECAL_API_EXPORTED_MEMBER
      CServiceServer& operator=(CServiceServer&& rhs) noexcept;

    /**
     * @brief Set/overwrite a method callback, that will be invoked, when a connected client is making a service call.
     *
     * @param method_info_  Service method information (method name, request & response types).
     * @param callback_     Callback function for client request.
     *
     * @return  True if succeeded, false if not.
    **/
    ECAL_API_EXPORTED_MEMBER
      bool SetMethodCallback(const SServiceMethodInformation& method_info_, const ServiceMethodCallbackT& callback_);

    /**
     * @brief Remove method callback.
     *
     * @param method_  Service method name.
     *
     * @return  True if succeeded, false if not.
    **/
    ECAL_API_EXPORTED_MEMBER
      bool RemoveMethodCallback(const std::string& method_);

    /**
     * @brief Retrieve service name.
     *
     * @return  The service name.
    **/
    ECAL_API_EXPORTED_MEMBER
      std::string GetServiceName();

    /**
     * @brief Retrieve the service id.
     *
     * @return  The service id.
    **/
    ECAL_API_EXPORTED_MEMBER
      SServiceId GetServiceId() const;

    /**
     * @brief Check connection state.
     *
     * @return  True if succeeded, false if not.
    **/
    ECAL_API_EXPORTED_MEMBER
      bool IsConnected();

  private:
    std::shared_ptr<CServiceServerImpl> m_service_server_impl;
  };
}
