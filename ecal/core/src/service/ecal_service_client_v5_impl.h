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
 * @brief  eCAL service client implementation (deprecated eCAL5 version)
**/

#pragma once

#include <ecal/ecal_deprecate.h>
#include <ecal/ecal_os.h>
#include <ecal/ecal_client.h>
#include <ecal/ecal_callback_v5.h>

#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace eCAL
{
  namespace v5
  {
    /**
     * @brief Service client implementation class.
    **/
    class CServiceClientImpl
    {
    public:
      // Default constructor for creating an empty service client instance.
      CServiceClientImpl();

      // Constructor for creating a service client instance with a specific service name.
      explicit CServiceClientImpl(const std::string& service_name_);

      // Constructor for creating a service client instance with a service name and method information map.
      explicit CServiceClientImpl(const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_);

      // Destructor to clean up resources.
      virtual ~CServiceClientImpl();

      // Creates a service client with a specific service name.
      bool Create(const std::string& service_name_);

      // Creates a service client with a specific service name and method information map.
      bool Create(const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_);

      // Destroys the service client instance and releases resources.
      bool Destroy();

      // Sets the host name filter for the service client instance.
      bool SetHostName(const std::string& host_name_);

      // Calls a method of the service and retrieves responses via callback.
      bool Call(const std::string& method_name_, const std::string& request_, int timeout_ = -1);

      // Calls a method of the service and retrieves all responses in a vector.
      bool Call(const std::string& method_name_, const std::string& request_, int timeout_, ServiceResponseVecT* service_response_vec_);

      // Makes an asynchronous call to a method of the service, responses are received via callback.
      bool CallAsync(const std::string& method_name_, const std::string& request_, int timeout_ = -1);

      // Registers a callback function for server responses.
      bool AddResponseCallback(const ResponseCallbackT& callback_);

      // Removes the registered server response callback function.
      bool RemResponseCallback();

      // Registers a callback function for a specific client event.
      bool AddEventCallback(eCAL_Client_Event type_, ClientEventCallbackT callback_);

      // Removes the registered callback function for a specific client event.
      bool RemEventCallback(eCAL_Client_Event type_);

      // Retrieves the name of the associated service.
      std::string GetServiceName();

      // Checks the connection state of the service client.
      bool IsConnected();

      // Prevent copy and move operations
      CServiceClientImpl(const CServiceClientImpl&) = delete;
      CServiceClientImpl& operator=(const CServiceClientImpl&) = delete;
      CServiceClientImpl(CServiceClientImpl&&) = delete;
      CServiceClientImpl& operator=(CServiceClientImpl&&) = delete;

    private:
      // Pointer to the underlying service client implementation
      std::shared_ptr<eCAL::CServiceClient> m_service_client_impl;

      // Host name filter for the service client
      std::string m_host_name;

      // Mutex and map for managing event callbacks
      std::mutex m_event_callback_map_mutex;
      std::map<eCAL_Client_Event, ClientEventCallbackT> m_event_callback_map;

      // Mutex and callback for managing server responses
      std::mutex m_response_callback_mutex;
      ResponseCallbackT m_response_callback;
    };
  }
}
