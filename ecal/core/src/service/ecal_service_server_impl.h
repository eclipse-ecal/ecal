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

#pragma once

#include <ecal/ecal.h>
#include <ecal/ecal_callback.h>

#include "ecal_tcpserver.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <ecal/pb/ecal.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <map>
#include <mutex>

namespace eCAL
{
  /**
   * @brief Service server implementation class.
  **/
  class CServiceServerImpl
  {
  public:
    CServiceServerImpl();
    CServiceServerImpl(const std::string& service_name_);

    ~CServiceServerImpl();

    bool Create(const std::string& service_name_);

    bool Destroy();

    // add and remove callback function for server method calls
    bool AddMethodCallback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const MethodCallbackT& callback_);
    bool RemMethodCallback(const std::string& method_);

    // add and remove callback function for server events
    bool AddEventCallback(eCAL_Server_Event type_, ServerEventCallbackT callback_);
    bool RemEventCallback(eCAL_Server_Event type_);
      
    // check connection state
    bool IsConnected();

    // called by the eCAL::CServiceGate to register a client
    void RegisterClient(const std::string& key_, const SClientAttr& client_);

    // called by eCAL:CServiceGate every second to update registration layer
    void RefreshRegistration();

    std::string GetServiceName() { return m_service_name; };

    // this object must not be copied.
    CServiceServerImpl(const CServiceServerImpl&) = delete;
    CServiceServerImpl& operator=(const CServiceServerImpl&) = delete;

  protected:
    int RequestCallback(const std::string& request_, std::string& response_);
    void EventCallback(eCAL_Server_Event event_, const std::string& message_);

    CTcpServer         m_tcp_server;

    std::string        m_service_name;
    std::string        m_service_id;

    struct SMethodCallback
    {
      eCAL::pb::Method method;
      MethodCallbackT  callback;
    };
    std::mutex         m_method_callback_map_sync;
    typedef std::map<std::string, SMethodCallback> MethodCallbackMapT;
    MethodCallbackMapT m_method_callback_map;

    std::mutex         m_event_callback_map_sync;
    typedef std::map<eCAL_Server_Event, ServerEventCallbackT> EventCallbackMapT;
    EventCallbackMapT  m_event_callback_map;
    
    bool               m_connected;
    bool               m_created;
  };
}
