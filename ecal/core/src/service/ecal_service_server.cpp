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
 * @brief  eCAL service server interface
**/

#include <ecal/ecal.h>

#include "ecal_servicegate.h"
#include "ecal_global_accessors.h"
#include "ecal_service_server_impl.h"

namespace eCAL
{
  /**
   * @brief Service Server class. 
  **/

  /**
  * @brief Constructor. 
  **/
  CServiceServer::CServiceServer() :
                  m_service_server_impl(nullptr),
                  m_created(false)
  {
  }

  /**
   * @brief Constructor. 
   *
   * @param service_name_  Service name.
  **/
  CServiceServer::CServiceServer(const std::string& service_name_) :
                  m_service_server_impl(nullptr),
                  m_created(false)
  {
    Create(service_name_);
  }

  /**
   * @brief Destructor. 
  **/
  CServiceServer::~CServiceServer()
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
  bool CServiceServer::Create(const std::string& service_name_)
  {
    if(m_created) return(false);

    m_service_server_impl = new CServiceServerImpl(service_name_);

    // register this service
    if (g_servicegate() != nullptr) g_servicegate()->Register(m_service_server_impl);

    m_created = true;
    return(true);
  }

  /**
   * @brief Destroys this object. 
   *
   * @return  True if successful. 
  **/
  bool CServiceServer::Destroy()
  {
    if(!m_created) return(false);
    m_created = false;

    // unregister this service
    if (g_servicegate() != nullptr) g_servicegate()->Unregister(m_service_server_impl);

    m_service_server_impl->Destroy();
    delete m_service_server_impl;
    m_service_server_impl = nullptr;

    return(true);
  }

  /**
   * @brief Add method type descriptions.
   *
   * @param method_     Service method name.
   * @param req_type_   Service method request type.
   * @param req_desc_   Service method request description.
   * @param resp_type_  Service method response type.
   * @param resp_desc_  Service method response description.
   *
   * @return  True if successful.
  **/
  bool CServiceServer::AddDescription(const std::string& method_, const std::string& req_type_, const std::string& req_desc_, const std::string& resp_type_, const std::string& resp_desc_)
  {
    if (!m_created) return false;
    SDataTypeInformation request_type_information;
    request_type_information.name = req_type_;
    request_type_information.descriptor = req_desc_;
    SDataTypeInformation response_type_information;
    response_type_information.name = resp_type_;
    response_type_information.descriptor = resp_desc_;
    return m_service_server_impl->AddDescription(method_, request_type_information, response_type_information);
  }

  /**
   * @brief Add client request callback.
   *
     * @param method_     Service method name.
     * @param req_type_   Service method request type.
     * @param resp_type_  Service method response type.
     * @param callback_   Callback function for client request.
   *
   * @return  True if successful.
  **/
  bool CServiceServer::AddMethodCallback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const MethodCallbackT& callback_)
  {
    if (!m_created) return false;
    return m_service_server_impl->AddMethodCallback(method_, req_type_, resp_type_, callback_);
  }

  /**
   * @brief Remove client request callback.
   *
   * @return  True if successful.
  **/
  bool CServiceServer::RemMethodCallback(const std::string& method_)
  {
    if (!m_created) return false;
    return m_service_server_impl->RemMethodCallback(method_);
  }

  /**
   * @brief Add callback function for server events.
   *
   * @param type_      The event type to react on.
   * @param callback_  The callback function to add.
   *
   * @return  True if succeeded, false if not.
  **/
  bool CServiceServer::AddEventCallback(eCAL_Server_Event type_, ServerEventCallbackT callback_)
  {
    if (!m_created) return false;
    return m_service_server_impl->AddEventCallback(type_, callback_);
  }

  /**
   * @brief Remove callback function for server events.
   *
   * @param type_  The event type to remove.
   *
   * @return  True if succeeded, false if not.
  **/
  bool CServiceServer::RemEventCallback(eCAL_Server_Event type_)
  {
    if (!m_created) return false;
    return m_service_server_impl->RemEventCallback(type_);
  }

  /**
   * @brief Retrieve service name.
   *
   * @return  The service name.
  **/
  std::string CServiceServer::GetServiceName()
  {
    if (!m_created) return "";
    return m_service_server_impl->GetServiceName();
  }

  /**
   * @brief Check connection state.
   *
   * @return  True if connected, false if not.
  **/
  bool CServiceServer::IsConnected()
  {
    if (!m_created) return false;
    return m_service_server_impl->IsConnected();
  }
}
