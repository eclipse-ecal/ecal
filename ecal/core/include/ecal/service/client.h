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
 * @file   service/client.h
 * @brief  eCAL client interface
**/

#pragma once

#include <ecal/deprecate.h>
#include <ecal/namespace.h>
#include <ecal/os.h>

#include <ecal/service/client_instance.h>
#include <ecal/service/types.h>

#include <memory>
#include <string>
#include <vector>

namespace eCAL
{
  class CServiceClientImpl;

  /**
   * @brief Service client wrapper class.
  **/
  class ECAL_API_CLASS CServiceClient
  {
  public:
    ECAL_API_EXPORTED_MEMBER
      static constexpr long long DEFAULT_TIME_ARGUMENT = -1;  /*!< Use DEFAULT_TIME_ARGUMENT in the `CallWithResponse()` and `CallWithCallback()` functions for blocking calls */

    /**
     * @brief Constructor.
     *
     * @param service_name_            Unique service name.
     * @param method_information_set_  Set of method names and corresponding datatype information.
     * @param event_callback_          The client event callback funtion.
    **/
    ECAL_API_EXPORTED_MEMBER
      CServiceClient(const std::string& service_name_, const ServiceMethodInformationSetT& method_information_set_ = ServiceMethodInformationSetT(), const ClientEventCallbackT& event_callback_ = ClientEventCallbackT());

    /**
     * @brief Destructor.
    **/
    ECAL_API_EXPORTED_MEMBER
      virtual ~CServiceClient();

    /**
     * @brief CServiceClient are non-copyable
    **/
    CServiceClient(const CServiceClient&) = delete;

    /**
     * @brief CServiceClient are non-copyable
    **/
    CServiceClient& operator=(const CServiceClient&) = delete;

    /**
     * @brief CServiceClient are move-enabled
    **/
    ECAL_API_EXPORTED_MEMBER
      CServiceClient(CServiceClient&& rhs) noexcept;

    /**
     * @brief CServiceClient are move-enabled
    **/
    ECAL_API_EXPORTED_MEMBER
      CServiceClient& operator=(CServiceClient&& rhs) noexcept;

    /**
     * @brief Get the client instances for all matching services
     *
     * @return  Vector of client instances
    **/
    ECAL_API_EXPORTED_MEMBER
      std::vector<CClientInstance> GetClientInstances() const;

    /**
     * @brief Blocking call of a service method for all existing service instances, response will be returned as ServiceResponseVecT
     *
     * @param       method_name_           Method name.
     * @param       request_               Request string.
     * @param [out] service_response_vec_  Response vector containing service responses from every called service (null pointer == no response).
     * @param       timeout_ms_            Maximum time before operation returns (in milliseconds, DEFAULT_TIME_ARGUMENT means infinite).
     *
     * @return  True if all calls were successful.
    **/
    ECAL_API_EXPORTED_MEMBER
      bool CallWithResponse(const std::string& method_name_, const std::string& request_, ServiceResponseVecT& service_response_vec_, int timeout_ms_ = DEFAULT_TIME_ARGUMENT) const;

    /**
     * @brief Blocking call (with timeout) of a service method for all existing service instances, using callback
     *
     * @param method_name_        Method name.
     * @param request_            Request string.
     * @param response_callback_  Callback function for the service method response.
     * @param timeout_ms_         Maximum time before operation returns (in milliseconds, DEFAULT_TIME_ARGUMENT means infinite).
     *
     * @return  True if all calls were successful.
    **/
    ECAL_API_EXPORTED_MEMBER
      bool CallWithCallback(const std::string& method_name_, const std::string& request_, const ResponseCallbackT& response_callback_, int timeout_ms_ = DEFAULT_TIME_ARGUMENT) const;

    /**
     * @brief Asynchronous call of a service method for all existing service instances, using callback
     *
     * @param method_name_        Method name.
     * @param request_            Request string.
     * @param response_callback_  Callback function for the service method response.
     *
     * @return  True if all calls were successful.
    **/
    ECAL_API_EXPORTED_MEMBER
      bool CallWithCallbackAsync(const std::string& method_name_, const std::string& request_, const ResponseCallbackT& response_callback_) const;

    /**
     * @brief Retrieve service name.
     *
     * @return  The service name.
    **/
    ECAL_API_EXPORTED_MEMBER
      std::string GetServiceName() const;

    /**
     * @brief Retrieve the service id.
     *
     * @return  The service id.
    **/
    ECAL_API_EXPORTED_MEMBER
      SServiceId GetServiceId() const;

    /**
     * @brief Check connection to at least one service.
     *
     * @return  True if at least one service client instance is connected.
    **/
    ECAL_API_EXPORTED_MEMBER
      bool IsConnected() const;

  private:
    std::shared_ptr<eCAL::CServiceClientImpl> m_service_client_impl;
  };
}
