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
 * @file   v5/ecal_client.h
 * @brief  eCAL client interface (deprecated eCAL5 version)
**/

#pragma once

#include <ecal/namespace.h>

#include <ecal/deprecate.h>
#include <ecal/os.h>
#include <ecal/v5/ecal_callback.h>
#include <ecal/service/types.h>

#include <memory>
#include <string>

namespace eCAL
{
  ECAL_CORE_NAMESPACE_V5
  {
    class CServiceClientImpl;

    /**
     * @brief Service client wrapper class.
    **/
    class ECAL_API_CLASS CServiceClient
    {
    public:
      /**
       * @brief Constructor.
      **/
      ECAL_API_EXPORTED_MEMBER
        CServiceClient();

      /**
       * @brief Constructor.
       *
       * @param service_name_  Unique service name.
      **/
      ECAL_API_EXPORTED_MEMBER
        explicit CServiceClient(const std::string& service_name_);

      /**
       * @brief Constructor.
       *
       * @param service_name_  Unique service name.
       * @param method_information_map_  Map of method names and corresponding datatype information.
      **/
      ECAL_API_EXPORTED_MEMBER
        explicit CServiceClient(const std::string& service_name_, const ServiceMethodInformationSetT& method_information_map_);

      /**
       * @brief Destructor.
      **/
      ECAL_API_EXPORTED_MEMBER
        virtual ~CServiceClient();

      /**
       * @brief CServiceClients are non-copyable
      **/
      CServiceClient(const CServiceClient&) = delete;

      /**
       * @brief CServiceClients are non-copyable
      **/
      CServiceClient& operator=(const CServiceClient&) = delete;

      /**
       * @brief Creates this object.
       *
       * @param service_name_  Unique service name.
       *
       * @return  True if successful.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool Create(const std::string& service_name_);

      /**
       * @brief Creates this object.
       *
       * @param service_name_  Unique service name.
       * @param method_information_map_  Map of method names and corresponding datatype information.
       *
       * @return  True if successful.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool Create(const std::string& service_name_, const ServiceMethodInformationSetT& method_information_map_);

      /**
       * @brief Destroys this object.
       *
       * @return  True if successful.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool Destroy();

      /**
       * @brief Change the host name filter for that client instance
       *
       * @param host_name_  Host name filter (empty == all hosts)
       *
       * @return  True if successful.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool SetHostName(const std::string& host_name_);

      /**
       * @brief Call a method of this service, responses will be returned by callback.
       *
       * @param method_name_  Method name.
       * @param request_      Request string.
       * @param timeout_      Maximum time before operation returns (in milliseconds, -1 means infinite).
       *
       * @return  True if successful.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool Call(const std::string& method_name_, const std::string& request_, int timeout_ = -1);

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
      ECAL_API_EXPORTED_MEMBER
        bool Call(const std::string& method_name_, const std::string& request_, int timeout_, ServiceResponseVecT* service_response_vec_);

      /**
       * @brief Call a method of this service asynchronously, responses will be returned by callback.
       *
       * @param method_name_  Method name.
       * @param request_      Request string.
       * @param timeout_      Maximum time before operation returns (in milliseconds, -1 means infinite) - NOT SUPPORTED YET.
       *
       * @return  True if successful.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool CallAsync(const std::string& method_name_, const std::string& request_, int timeout_ = -1);

      /**
       * @brief Add server response callback.
       *
       * @param callback_  Callback function for server response.
       *
       * @return  True if successful.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool AddResponseCallback(const ResponseCallbackT& callback_);

      /**
       * @brief Remove server response callback.
       *
       * @return  True if successful.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool RemResponseCallback();

      /**
       * @brief Add client event callback function.
       *
       * @param type_      The event type to react on.
       * @param callback_  The callback function to add.
       *
       * @return  True if succeeded, false if not.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool AddEventCallback(eClientEvent type_, ClientEventCallbackT callback_);

      /**
       * @brief Remove client event callback function.
       *
       * @param type_  The event type to remove.
       *
       * @return  True if succeeded, false if not.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool RemEventCallback(eClientEvent type_);

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
      std::shared_ptr<CServiceClientImpl> m_service_client_impl;
    };
  }
}
