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
 * @file   ecal_server.h
 * @brief  eCAL service interface
**/

#pragma once

#include <ecal/ecal_os.h>
#include <ecal/ecal_callback.h>
#include <ecal/ecal_service_info.h>

#include <string>
#include <vector>

#ifndef ECAL_C_DLL

namespace eCAL
{
  class CServiceServerImpl;

  /**
   * @brief Service Server wrapper class.
  **/
  class ECAL_API CServiceServer
  {
  public:
    /**
     * @brief Constructor. 
    **/
    CServiceServer();

    /**
     * @brief Constructor. 
     *
     * @param service_name_   Unique service name.
    **/
    CServiceServer(const std::string& service_name_);

    /**
     * @brief Destructor. 
    **/
    virtual ~CServiceServer();

    /**
     * @brief CServiceServers are non-copyable
    **/
    CServiceServer(const CServiceServer&) = delete;

    /**
     * @brief CServiceServers are non-copyable
    **/
    CServiceServer& operator=(const CServiceServer&) = delete;

    /**
     * @brief Creates this object. 
     *
     * @param service_name_   Unique service name.
     *
     * @return  True if successful. 
    **/
    bool Create(const std::string& service_name_);

    /**
     * @brief Destroys this object. 
     *
     * @return  True if successful. 
    **/
    bool Destroy();

    /**
     * @brief Add method callback.
     *
     * @param method_     Service method name.
     * @param req_type_   Service method request type.
     * @param resp_type_  Service method response type.
     * @param callback_   Callback function for client request.
     *
     * @return  True if successful.
    **/
    bool AddMethodCallback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const MethodCallbackT& callback_);

    /**
     * @brief Remove method callback.
     *
     * @param method_  Service method name.
     *
     * @return  True if successful.
    **/
    bool RemMethodCallback(const std::string& method_);

    /**
     * @brief Retrieve service name.
     *
     * @return  The service name.
    **/
    std::string GetServiceName();

    /**
     * @brief Add server event callback function.
     *
     * @param type_      The event type to react on.
     * @param callback_  The callback function to add.
     *
     * @return  True if succeeded, false if not.
    **/
    bool AddEventCallback(eCAL_Server_Event type_, ServerEventCallbackT callback_);

    /**
     * @brief Remove server event callback function.
     *
     * @param type_  The event type to remove.
     *
     * @return  True if succeeded, false if not.
    **/
    bool RemEventCallback(eCAL_Server_Event type_);

    /**
     * @brief Check connection state.
     *
     * @return  True if connected, false if not.
    **/
    bool IsConnected();

  protected:
    CServiceServerImpl*  m_service_server_impl;
    bool                 m_created;
  };
} 

#else /* ! ECAL_C_DLL */

#include <ecal/ecalc.h>

namespace eCAL
{
  class CServiceServer
  {
  public:
    CServiceServer() :
      m_service(nullptr),
      m_created(false)
    {
    }

    CServiceServer(const std::string& service_name_) :
      m_service(nullptr),
      m_created(false)
    {
      Create(service_name_);
    }

    virtual ~CServiceServer()
    {
      Destroy();
    }

    // this object must not be copied.
    CServiceServer(const CServiceServer&) = delete;
    CServiceServer& operator=(const CServiceServer&) = delete;

    bool Create(const std::string& service_name_)
    {
      if(m_created) return(false);
      m_service = eCAL_Server_Create(service_name_.c_str());
      if(!m_service) return(false);

      m_created = true;
      return(true);
    }

    bool Destroy()
    {
      if(!m_created) return(false);
      if(m_service) eCAL_Server_Destroy(m_service);

      m_service = nullptr;
      m_created = false;
      return(true);
    }

    static int MethodCallback(const char* method_, const char* req_type_, const char* resp_type_, const char* request_, int request_len_, void** response_, int* response_len_, void* par_)
    {
      if (par_          == nullptr) return 0;
      if (response_     == nullptr) return 0;;
      if (response_len_ == nullptr) return 0;;
      CServiceServer* server = static_cast<CServiceServer*>(par_);

      std::string response;
      int ret_state = server->m_callback(method_, req_type_, resp_type_, std::string(request_, request_len_), response);
      if (!response.empty())
      {
        int response_size = static_cast<int>(response.size());

        if (*response_len_ == ECAL_ALLOCATE_4ME)
        {
          std::vector<char>& response_buf = server->GetResponseBuffer();
          response_buf.resize(response_size);
          *response_     = response_buf.data();
          *response_len_ = response_size;
        }

        if ((*response_ != nullptr) && (*response_len_ >= response_size))
        {
          memcpy(*response_, response.data(), response.size());
        }
      }
      return ret_state;
    }

    bool AddMethodCallback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const MethodCallbackT& callback_)
    {
      if (!m_created) return false;
      m_callback = callback_;
      return eCAL_Server_AddMethodCallbackC(m_service, method_.c_str(), req_type_.c_str(), resp_type_.c_str(), &CServiceServer::MethodCallback, this) != 0;
    }

    bool RemMethodCallback(const std::string& method_)
    {
      if (!m_created) return false;
      m_callback = nullptr;
      return eCAL_Server_RemMethodCallbackC(m_service, method_.c_str()) != 0;
    }

    std::string GetServiceName()
    {
      if (!m_created) return "";
      std::string service_name;
      void* buf = nullptr;
      size_t buf_len = eCAL_Server_GetServiceName(m_service, &buf, ECAL_ALLOCATE_4ME);
      if (buf_len > 0)
      {
        service_name = std::string(static_cast<char*>(buf), buf_len);
        eCAL_FreeMem(buf);
      }
      return(service_name);
    }


    std::vector<char>& GetResponseBuffer()
    {
      return m_response_buf;
    }

  protected:
    ECAL_HANDLE        m_service;
    MethodCallbackT    m_callback;
    std::vector<char>  m_response_buf;

    bool               m_created;
  };
} 

#endif /* ! ECAL_C_DLL */
