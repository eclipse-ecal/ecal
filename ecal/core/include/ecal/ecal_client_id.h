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
 * @file   ecal_client.h
 * @brief  eCAL client interface
**/

#pragma once

#include <ecal/ecal_callback.h>
#include <ecal/ecal_service_info.h>
#include <ecal/ecal_client_instance.h>

#include <string>
#include <vector>
#include <memory>

namespace eCAL
{
  class CServiceClientIDImpl;

  /**
   * @brief Service client wrapper class.
  **/
  class CServiceClientID
  {
  public:
    /**
     * @brief Constructor.
     *
     * @param service_name_  Unique service name.
    **/
    ECAL_API explicit CServiceClientID(const std::string& service_name_);

    /**
     * @brief Constructor.
     *
     * @param service_name_            Unique service name.
     * @param method_information_map_  Map of method names and corresponding datatype information.
    **/
    ECAL_API CServiceClientID(const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_);

    /**
     * @brief Destructor.
    **/
    ECAL_API virtual ~CServiceClientID();

    // Deleted copy constructor and copy assignment operator
    CServiceClientID(const CServiceClientID&) = delete;
    CServiceClientID& operator=(const CServiceClientID&) = delete;

    // Move constructor and move assignment operator
    ECAL_API CServiceClientID(CServiceClientID&& rhs) noexcept;
    ECAL_API CServiceClientID& operator=(CServiceClientID&& rhs) noexcept;

    /**
     * @brief Get the client instances for all matching services
     *
     * @return  Vector of client instances
    **/
    ECAL_API std::vector<CServiceClientInstance> GetServiceClientInstances() const;

    /**
     * @brief Blocking call (with timeout) of a service method for all existing service instances, using callback
     *
     * @param method_name_        Method name.
     * @param request_            Request string.
     * @param timeout_            Maximum time before operation returns (in milliseconds, -1 means infinite).
     * @param response_callback_  Callback function for the service method response.
     *
     * @return  True if all calls were successful.
    **/
    ECAL_API bool CallWithCallback(const std::string& method_name_, const std::string& request_, int timeout_, const ResponseIDCallbackT& response_callback_) const;

    /**
     * @brief Retrieve service name.
     *
     * @return  The service name.
    **/
    ECAL_API std::string GetServiceName() const;

    /**
     * @brief Check connection to at least one service.
     *
     * @return  True if at least one service client instance is connected.
    **/
    ECAL_API bool IsConnected() const;

  private:
    std::string                                 m_service_name;
    std::shared_ptr<eCAL::CServiceClientIDImpl> m_service_client_impl;
  };
}
