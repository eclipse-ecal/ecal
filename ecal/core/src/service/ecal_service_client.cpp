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
 * @brief  eCAL service client interface
**/

#include <ecal/ecal.h>
#include "ecal_service_client_impl.h"

namespace eCAL
{
  /**
   * @brief Constructor.
  **/
  CServiceClient::CServiceClient() :
                  m_service_client_impl(nullptr),
                  m_created(false)
  {
  }

  /**
   * @brief Constructor. 
   *
   * @param service_name_  Service name. 
  **/
  CServiceClient::CServiceClient(const std::string& service_name_) :
                   m_service_client_impl(nullptr),
                   m_created(false)
  {
    Create(service_name_);
  }

  /**
   * @brief Destructor. 
  **/
  CServiceClient::~CServiceClient()
  {
    Destroy();
  }

  /**
   * @brief Creates this object. 
   *
   * @param service_name_  Service name. 
   *
   * @return  True if successful. 
  **/
  bool CServiceClient::Create(const std::string& service_name_)
  {
    if(m_created) return(false);

    m_service_client_impl = new CServiceClientImpl;
    m_service_client_impl->Create(service_name_);

    m_created = true;
    return(true);
  }

  /**
   * @brief Destroys this object. 
   *
   * @return  True if successful. 
  **/
  bool CServiceClient::Destroy()
  {
    if(!m_created) return(false);
    m_created = false;

    m_service_client_impl->Destroy();
    delete m_service_client_impl;
    m_service_client_impl = nullptr;

    return(true);
  }

  /**
   * @brief Change the host name filter for that client instance
   *
   * @param host_name_  Host name filter (empty == all hosts)
   *
   * @return  True if successful.
  **/
  bool CServiceClient::SetHostName(const std::string& host_name_)
  {
    if (!m_created) return(false);
    m_service_client_impl->SetHostName(host_name_);
    return(true);
  }

  /**
   * @brief Call method of this service for a specific host, responses will be returned by callback.
   *
   * @param method_name_  Method name.
   * @param request_      Request string.
   * @param timeout_      Maximum time before operation returns (in milliseconds, -1 means infinite).
   *
   * @return  True if successful.
  **/
  bool CServiceClient::Call(const std::string& method_name_, const std::string& request_, int timeout_)
  {
    if(!m_created) return(false);
    return(m_service_client_impl->Call(method_name_, request_, timeout_));
  }

  /**
   * @brief Call method of this service, for specific host, responses will be returned in service_response_vec_.
   *
   * @param       method_name_           Method name.
   * @param       request_               Request string.
   * @param       timeout_               Maximum time before operation returns (in milliseconds, -1 means infinite).
   * @param [out] service_response_vec_  Response vector containing service info and response string from every called service (optional).
   *
   * @return  True if successful.
  **/
  bool CServiceClient::Call(const std::string& method_name_, const std::string& request_, int timeout_, ServiceResponseVecT* service_response_vec_)
  {
    if (!m_created) return(false);
    return(m_service_client_impl->Call(method_name_, request_, timeout_, service_response_vec_));
  }

  /**
   * @brief Asynchronously call of this service, for specific host, response will be returned by callback.
   *
   * @param method_name_  Method name.
   * @param request_      Request string.
   * @param timeout_      Maximum time before operation returns (in milliseconds, -1 means infinite).
  **/
  void CServiceClient::CallAsync(const std::string& method_name_, const std::string& request_, int timeout_)
  {
    if(m_created) m_service_client_impl->CallAsync(method_name_, request_, timeout_);
  }

  /**
   * @brief Add server response callback. 
   *
   * @param callback_  Callback function for server response.  
  **/
  bool CServiceClient::AddResponseCallback(const ResponseCallbackT& callback_)
  {
    if (!m_created) return false;
    return(m_service_client_impl->AddResponseCallback(callback_));
  }

  /**
   * @brief Remove server response callback. 
  **/
  bool CServiceClient::RemResponseCallback()
  {
    if (!m_created) return false;
    return(m_service_client_impl->RemResponseCallback());
  }

  /**
   * @brief Add callback function for client events.
   *
   * @param type_      The event type to react on.
   * @param callback_  The callback function to add.
   *
   * @return  True if succeeded, false if not.
  **/
  bool CServiceClient::AddEventCallback(eCAL_Client_Event type_, ClientEventCallbackT callback_)
  {
    if (!m_created) return false;
    return m_service_client_impl->AddEventCallback(type_, callback_);
  }

  /**
   * @brief Remove callback function for client events.
   *
   * @param type_  The event type to remove.
   *
   * @return  True if succeeded, false if not.
  **/
  bool CServiceClient::RemEventCallback(eCAL_Client_Event type_)
  {
    if (!m_created) return false;
    return m_service_client_impl->RemEventCallback(type_);
  }

  /**
   * @brief Check connection state.
   *
   * @return  True if connected, false if not.
  **/
  bool CServiceClient::IsConnected()
  {
    if (!m_created) return false;
    return m_service_client_impl->IsConnected();
  }
}
