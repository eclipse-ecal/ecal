/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 * Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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
     * This method calls all existing service instances of the service with the
     * given method name and request string.
     * 
     * This method will block until all service calls have returned. In case
     * that a timeout is specified, this method will wait for a service call
     * response until the timeout is reached.
     * 
     * If a service call times out, it WILL NOT be cancelled (i.e. it may
     * continue to execute in the background), but any responses that arrive
     * will be ignored.
     * Instead, the response vector will contain a service response element with:
     *    service_response.call_state == eCallState::timeouted.
     * 
     * Changes with eCAL 6.1: In eCAL 6.0 and earlier, the response vector did
     * not contain responses for timeouted calls. Instead, a timeout event was
     * called. Beginning with eCAL 6.1, this function DOES include responses for
     * timeouted calls in the response vector.
     * 
     * @param       method_name_           Method name.
     * @param       request_               Request string.
     * @param [out] service_response_vec_  Response vector containing service responses from every called service (null pointer == no response).
     * @param       timeout_ms_            Maximum time before operation returns (in milliseconds. 0 or negative values mean infinite).
     *
     * @return  True if all calls were successful and minimum one instance was connected, otherwise false.
    **/
    ECAL_API_EXPORTED_MEMBER
      bool CallWithResponse(const std::string& method_name_, const std::string& request_, ServiceResponseVecT& service_response_vec_, int timeout_ms_ = DEFAULT_TIME_ARGUMENT) const;

    /**
     * @brief Blocking call (with timeout) of a service method for all existing service instances, using callback
     * 
     * This method calls all existing service instances of the service with the
     * given method name and request string.
     * 
     * This method will block until all service calls have returned, AND their
     * response callbacks have been executed. In case that a timeout is
     * specified, this method will wait for a service call response until the
     * timeout is reached. This method MAY block longer than the specified
     * timeout, as it also waits for the response callbacks to be executed.
     * 
     * If a service call times out, it WILL NOT be cancelled (i.e. it will
     * continue to execute in the background), but any responses that arrive
     * will be ignored.
     * Instead, the response callback will be called with an
     * eCAL::SServiceResponse, where
     *    service_response.call_state == eCallState::timeouted.
     * 
     * Changes with eCAL 6.1: In eCAL 6.0 and earlier, the response callback was
     * NOT called in case of timeouts. Instead, a timeout event was called.
     * Beginning with eCAL 6.1, the response callback IS called in case of
     * timeouts
     * 
     * @param method_name_        Method name.
     * @param request_            Request string.
     * @param response_callback_  Callback function for the service method response.
     * @param timeout_ms_         Maximum time before operation returns (in milliseconds. 0 or negative values mean infinite).
     *
     * @return  True if all calls were successful and minimum one instance was connected, otherwise false.
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
     * @return  True if all calls were successful and minimum one instance was connected, otherwise false.
    **/
    ECAL_API_EXPORTED_MEMBER
      bool CallWithCallbackAsync(const std::string& method_name_, const std::string& request_, const ResponseCallbackT& response_callback_) const;

    /**
     * @brief Retrieve service name.
     *
     * @return  The service name.
    **/
    ECAL_API_EXPORTED_MEMBER
      const std::string& GetServiceName() const;

    /**
     * @brief Retrieve the service id.
     *
     * @return  The service id.
    **/
    ECAL_API_EXPORTED_MEMBER
      const SServiceId& GetServiceId() const;

    /**
     * @brief Check connection to at least one service.
     *
     * @return  True if at least one service client instance is connected.
    **/
    ECAL_API_EXPORTED_MEMBER
      bool IsConnected() const;

  private:
    std::weak_ptr<eCAL::CServiceClientImpl> m_service_client_impl;
  };
}
