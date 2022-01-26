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
   * @brief Call method of this service, responses will be returned by callback.
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
   * @brief Call a method of this service, all responses will be returned in service_response_vec_.
   *
   * @param       method_name_           Method name.
   * @param       request_               Request string.
   * @param       timeout_               Maximum time before operation returns (in milliseconds, -1 means infinite).
   * @param [out] service_response_vec_  Response vector containing service responses from every called service (null pointer == no response).
   *
   * @return  True if successful.
  **/
  bool CServiceClient::Call(const std::string& method_name_, const std::string& request_, int timeout_, ServiceResponseVecT* service_response_vec_)
  {
    if (!m_created) return(false);
    return(m_service_client_impl->Call(method_name_, request_, timeout_, service_response_vec_));
  }

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
  bool CServiceClient::Call(const std::string& host_name_, const std::string& method_name_, const std::string& request_, struct SServiceResponse& service_info_, std::string& response_)
  {
    if (!m_created) return(false);

    m_service_client_impl->SetHostName(host_name_);

    ServiceResponseVecT service_response_vec;
    if (m_service_client_impl->Call(method_name_, request_, -1, &service_response_vec))
    {
      if (!service_response_vec.empty())
      {
        service_info_ = service_response_vec[0];
        response_ = service_info_.response;
        return(true);
      }
    }
    return(false);
  }

  /**
   * @brief Call a method of this service asynchronously, responses will be returned by callback.
   *
   * @param method_name_  Method name.
   * @param request_      Request string.
   * @param timeout_      Maximum time before operation returns (in milliseconds, -1 means infinite) - NOT SUPPORTED YET.
   *
   * @return  True if successful.
  **/
  bool CServiceClient::CallAsync(const std::string& method_name_, const std::string& request_, int timeout_)
  {
    if (!m_created) return(false);
    (void)timeout_; // will be implemented later
    return(m_service_client_impl->CallAsync(method_name_, request_ /*, timeout_*/));
  }

  /**
   * @brief Add server response callback.
   *
   * @param callback_  Callback function for server response.
   *
   * @return  True if successful.
  **/
  bool CServiceClient::AddResponseCallback(const ResponseCallbackT& callback_)
  {
    if (!m_created) return false;
    return(m_service_client_impl->AddResponseCallback(callback_));
  }

  /**
   * @brief Remove server response callback.
   *
   * @return  True if successful.
  **/
  bool CServiceClient::RemResponseCallback()
  {
    if (!m_created) return false;
    return(m_service_client_impl->RemResponseCallback());
  }

  /**
   * @brief Add client event callback function.
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
   * @brief Remove client event callback function.
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
   * @brief Retrieve service name.
   *
   * @return  The service name.
  **/
  std::string CServiceClient::GetServiceName()
  {
    if (!m_created) return "";
    return m_service_client_impl->GetServiceName();
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
