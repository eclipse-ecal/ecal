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
  class CServiceClientImpl;

  /**
   * @brief Service client wrapper class.
  **/
  class CServiceClient
  {
  public:
    /**
     * @brief Constructor. 
    **/
    ECAL_API CServiceClient();

    /**
     * @brief Constructor. 
     *
     * @param service_name_  Unique service name.
    **/
    ECAL_API CServiceClient(const std::string& service_name_);

    /**
     * @brief Destructor. 
    **/
    ECAL_API virtual ~CServiceClient();

    /**
     * @brief CServiceClients are non-copyable
    **/
    ECAL_API CServiceClient(const CServiceClient&) = delete;

    /**
     * @brief CServiceClients are non-copyable
    **/
    ECAL_API CServiceClient& operator=(const CServiceClient&) = delete;

    /**
     * @brief Creates this object. 
     *
     * @param service_name_  Unique service name.
     *
     * @return  True if successful. 
    **/
    ECAL_API bool Create(const std::string& service_name_);

    /**
     * @brief Destroys this object. 
     *
     * @return  True if successful. 
    **/
    ECAL_API bool Destroy();

    /**
     * @brief Change the host name filter for that client instance
     *
     * @param host_name_  Host name filter (empty == all hosts)
     *
     * @return  True if successful.
    **/
    ECAL_API bool SetHostName(const std::string& host_name_);

    /**
     * @brief Call a method of this service, responses will be returned by callback. 
     *
     * @param method_name_  Method name.
     * @param request_      Request string. 
     * @param timeout_      Maximum time before operation returns (in milliseconds, -1 means infinite).
     *
     * @return  True if successful. 
    **/
    ECAL_API bool Call(const std::string& method_name_, const std::string& request_, int timeout_ = -1);

    /**
     * @brief Call a method of this service, all responses will be returned in service_response_vec_. 
     *
     * @param       method_name_           Method name.
     * @param       request_               Request string.
     * @param       timeout_               Maximum time before operation returns (in milliseconds, -1 means infinite).
     * @param [out] service_response_vec_  Response vector containing service responses from every called service (null pointer == no response).
     *
     * @return  True if successful.
    **/
    ECAL_API bool Call(const std::string& method_name_, const std::string& request_, int timeout_, ServiceResponseVecT* service_response_vec_);

    /**
     * @brief Call method of this service, for specific host (deprecated). 
     *
     * @param       host_name_         Host name.
     * @param       method_name_       Method name.
     * @param       request_           Request string.
     * @param [out] service_info_      Service response struct for detailed informations.
     * @param [out] response_          Response string.
     *
     * @return  True if successful.
    **/
    ECAL_DEPRECATE_SINCE_5_10("Please use the create method bool Call(const std::string& method_name_, const std::string& request_, int timeout_, ServiceResponseVecT* service_response_vec_) instead. This function will be removed in eCAL6.")
    ECAL_API bool Call(const std::string& host_name_, const std::string& method_name_, const std::string& request_, struct SServiceResponse& service_info_, std::string& response_);

    /**
     * @brief Call a method of this service asynchronously, responses will be returned by callback. 
     *
     * @param method_name_  Method name.
     * @param request_      Request string. 
     * @param timeout_      Maximum time before operation returns (in milliseconds, -1 means infinite) - NOT SUPPORTED YET.
     *
     * @return  True if successful.
    **/
    ECAL_API bool CallAsync(const std::string& method_name_, const std::string& request_, int timeout_ = -1);

    /**
     * @brief Add server response callback. 
     *
     * @param callback_  Callback function for server response.  
     *
     * @return  True if successful.
    **/
    ECAL_API bool AddResponseCallback(const ResponseCallbackT& callback_);

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
     * @brief Check connection state.
     *
     * @return  True if connected, false if not.
    **/
    ECAL_API bool IsConnected();

  protected:
    std::shared_ptr<eCAL::CServiceClientImpl> m_service_client_impl;
    bool                m_created;
  };
}
