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
    if(!m_created)   return(false);

    m_service_client_impl->Destroy();
    delete m_service_client_impl;
    m_service_client_impl = nullptr;

    m_created = false;
    return(true);
  }

  /**
   * @brief Change the host name filter for that client instance
   *
   * @param host_name_  Host name filter (empty or "*" == all hosts) 
   *
   * @return  True if successful. 
  **/
  bool CServiceClient::SetHostName(const std::string& host_name_)
  {
    if(!m_created) return(false);
    m_service_client_impl->SetHostName(host_name_);
    return(true);
  }

  /**
   * @brief Call method of this service (asynchronously method with callback). 
   *
   * @param method_name_  Method name. 
   * @param request_      Request string. 
   *
   * @return  True if successful. 
  **/
  bool CServiceClient::Call(const std::string& method_name_, const std::string& request_)
  {
    if(!m_created) return(false);
    return(m_service_client_impl->Call(method_name_, request_));
  }

  /**
   * @brief Call method of this service (blocking variant). 
   *
   * @param host_name_     Host name.
   * @param method_name_   Method name.
   * @param request_       Request string. 
   * @param service_info_  Service info struct for detailed informations.
   * @param response_      Response string.
   *
   * @return  True if successful. 
  **/
  bool CServiceClient::Call(const std::string& host_name_, const std::string& method_name_, const std::string& request_, struct SServiceInfo& service_info_, std::string& response_)
  {
    if(!m_created) return(false);
    return(m_service_client_impl->Call(host_name_, method_name_, request_, service_info_, response_));
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
}
