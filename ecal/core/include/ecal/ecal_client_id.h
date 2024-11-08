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
     * @param service_name_  Unique service name.
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
     * @param service_name_  Unique service name.
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
     * @brief Get the unique service id's for all matching services
     *
     * @return  Service id's of all matching services
    **/
    ECAL_API std::vector<Registration::SEntityId> GetServiceIDs();

    /**
     * @brief Blocking call specific service method, response will be returned as pair<bool, SServiceReponse>
     *
     * @param       entity_id_    Unique service entity (service id, process id, host name).
     * @param       method_name_  Method name.
     * @param       request_      Request string.
     * @param       timeout_      Maximum time before operation returns (in milliseconds, -1 means infinite).
     *
     * @return  success state and service response
    **/
    ECAL_API std::pair<bool, SServiceResponse> CallWithResponse(const Registration::SEntityId& entity_id_, const std::string& method_name_, const std::string& request_, int timeout_ = -1);

    /**
     * @brief Blocking call specific service method, using callback
     *
     * @param entity_id_    Unique service entity (service id, process id, host name).
     * @param method_name_  Method name.
     * @param request_      Request string. 
     * @param timeout_      Maximum time before operation returns (in milliseconds, -1 means infinite).
     *
     * @return  True if successful. 
    **/
    ECAL_API bool CallWithCallback(const Registration::SEntityId& entity_id_, const std::string& method_name_, const std::string& request_, int timeout_ = -1);

    /**
     * @brief Add server response callback. 
     *
     * @param callback_  Callback function for server response.  
     *
     * @return  True if successful.
    **/
    ECAL_API bool AddResponseCallback(const ResponseIDCallbackT& callback_);

    /**
     * @brief Remove server response callback. 
     *
     * @return  True if successful.
    **/
    ECAL_API bool RemResponseCallback();

    /**
     * @brief Add client event callback function.
     *
     * @param type_      The event type to react on.
     * @param callback_  The callback function to add.
     *
     * @return  True if succeeded, false if not.
    **/
    ECAL_API bool AddEventCallback(eCAL_Client_Event type_, ClientEventCallbackT callback_);

    /**
     * @brief Remove client event callback function.
     *
     * @param type_  The event type to remove.
     *
     * @return  True if succeeded, false if not.
    **/
    ECAL_API bool RemEventCallback(eCAL_Client_Event type_);

    /**
     * @brief Retrieve service name.
     *
     * @return  The service name.
    **/
    ECAL_API std::string GetServiceName();

    /**
     * @brief Check connection state of a specific server connection.
     *
     * @param entity_id_  Unique service entity (service id, process id, host name).
     *
     * @return  True if connected, false if not.
    **/
    ECAL_API bool IsConnected(const Registration::SEntityId& entity_id_);

    /**
     * @brief Check connection state.
     *
     * @return  True if connected, false if not.
    **/
    ECAL_API bool IsConnected();

  protected:
    std::shared_ptr<eCAL::CServiceClientIDImpl> m_service_client_impl;
    std::string                                 m_service_name;
  };
}
