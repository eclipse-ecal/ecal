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
 * @file   service/client_instance.h
 * @brief  eCAL client instance interface
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
  class CServiceClientImpl;

  class ECAL_API_CLASS CClientInstance final
  {
  public:
    ECAL_API_EXPORTED_MEMBER
      static constexpr long long DEFAULT_TIME_ARGUMENT = -1;  /*!< Use DEFAULT_TIME_ARGUMENT in the `CallWithResponse()` and `CallWithCallback()` functions for blocking calls */

    // Constructor
    ECAL_API_EXPORTED_MEMBER
      CClientInstance(const SEntityId& entity_id_, const std::shared_ptr<CServiceClientImpl>& service_client_id_impl_);

    // Defaulted destructor
    ~CClientInstance() = default;

    // Deleted copy constructor and copy assignment operator
    CClientInstance(const CClientInstance&) = delete;
    CClientInstance& operator=(const CClientInstance&) = delete;

    // Defaulted move constructor and move assignment operator
    ECAL_API_EXPORTED_MEMBER CClientInstance(CClientInstance&& rhs) noexcept = default;
    ECAL_API_EXPORTED_MEMBER CClientInstance& operator=(CClientInstance&& rhs) noexcept = default;

    /**
     * @brief Blocking call of a service method, response will be returned as pair<bool, SServiceReponse>
     *
     * @param       method_name_  Method name.
     * @param       request_      Request string.
     * @param       timeout_ms_   Maximum time before operation returns (in milliseconds, DEFAULT_TIME_ARGUMENT means infinite).
     *
     * @return  success state and service response
    **/
    ECAL_API_EXPORTED_MEMBER
      std::pair<bool, SServiceResponse> CallWithResponse(const std::string& method_name_, const std::string& request_, int timeout_ms_ = DEFAULT_TIME_ARGUMENT);

    /**
     * @brief Blocking call of a service method, using callback
     *
     * @param method_name_        Method name.
     * @param request_            Request string.
     * @param response_callback_  Callback function for the service method response.
     * @param timeout_ms_         Maximum time before operation returns (in milliseconds, DEFAULT_TIME_ARGUMENT means infinite).
     *
     * @return  True if successful.
    **/
    ECAL_API_EXPORTED_MEMBER
      bool CallWithCallback(const std::string& method_name_, const std::string& request_, const ResponseCallbackT& response_callback_, int timeout_ms_ = DEFAULT_TIME_ARGUMENT);

    /**
     * @brief Asynchronous call of a service method, using callback
     *
     * @param method_name_        Method name.
     * @param request_            Request string.
     * @param response_callback_  Callback function for the service method response.
     *
     * @return  True if successful.
    **/
    ECAL_API_EXPORTED_MEMBER
      bool CallWithCallbackAsync(const std::string& method_name_, const std::string& request_, const ResponseCallbackT& response_callback_);

    /**
     * @brief Check connection state.
     *
     * @return  True if connected, false if not.
    **/
    ECAL_API_EXPORTED_MEMBER
      bool IsConnected() const;

    /**
     * @brief Get unique client entity id.
     *
     * @return  The client entity id.
    **/
    ECAL_API_EXPORTED_MEMBER
      SEntityId GetClientID() const;

  private:
    SEntityId                                       m_entity_id;
    const std::shared_ptr<eCAL::CServiceClientImpl> m_service_client_impl;
  };
}
