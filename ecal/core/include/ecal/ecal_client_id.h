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

#include <ecal/ecal_deprecate.h>
#include <ecal/ecal_os.h>
#include <ecal/ecal_callback.h>
#include <ecal/ecal_service_info.h>
#include <ecal/ecal_client_instance.h>

#include <iostream>
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
    **/
    ECAL_API CServiceClientID();

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
    ECAL_API explicit CServiceClientID(const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_);

    /**
     * @brief Destructor. 
    **/
    ECAL_API virtual ~CServiceClientID();

    /**
     * @brief CServiceClients are non-copyable
    **/
    CServiceClientID(const CServiceClientID&) = delete;

    /**
     * @brief CServiceClients are non-copyable
    **/
    CServiceClientID& operator=(const CServiceClientID&) = delete;

    /**
     * @brief Creates this object. 
     *
     * @param service_name_  Unique service name.
     *
     * @return  True if successful. 
    **/
    ECAL_API bool Create(const std::string& service_name_);

    /**
     * @brief Creates this object.
     *
     * @param service_name_            Unique service name.
     * @param method_information_map_  Map of method names and corresponding datatype information.
     *
     * @return  True if successful.
    **/
    ECAL_API bool Create(const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_);

    /**
     * @brief Destroys this object. 
     *
     * @return  True if successful. 
    **/
    ECAL_API bool Destroy();

    /**
     * @brief Add client event callback function.
     *
     * @param type_      The event type to react on.
     * @param callback_  The callback function to add.
     *
     * @return  True if succeeded, false if not.
    **/
    ECAL_API bool AddEventCallback(eCAL_Client_Event type_, ClientEventIDCallbackT callback_);

    /**
     * @brief Remove client event callback function.
     *
     * @param type_  The event type to remove.
     *
     * @return  True if succeeded, false if not.
    **/
    ECAL_API bool RemEventCallback(eCAL_Client_Event type_);

    /**
     * @brief Get the client instances for all matching services
     *
     * @return  Vector of client instances
    **/
    ECAL_API std::vector<CServiceClientInstance> GetServiceClientInstances();

    /**
     * @brief Blocking call of a service method for all existing service instances, using callback
     *
     * @param method_name_        Method name.
     * @param request_            Request string.
     * @param timeout_            Maximum time before operation returns (in milliseconds, -1 means infinite).
     * @param response_callback_  Callback function for the service method response.
     *
     * @return  True if successful.
    **/
    ECAL_API void CallWithCallback(const std::string& method_name_, const std::string& request_, int timeout_, const ResponseIDCallbackT& repsonse_callback_);

    /**
     * @brief Retrieve service name.
     *
     * @return  The service name.
    **/
    ECAL_API std::string GetServiceName() const;

    /**
     * @brief Check connection state.
     *
     * @return  True if at least one service client instance is connected.
    **/
    ECAL_API bool IsConnected() const;

  private:
    std::string                                 m_service_name;
    std::shared_ptr<eCAL::CServiceClientIDImpl> m_service_client_impl;
  };
}
