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

#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#include <ecal/core/pb/ecal.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <map>
#include <mutex>

#include <ecal/service/server.h>

namespace eCAL
{
  /**
   * @brief Service server implementation class.
  **/
  class CServiceServerImpl : public std::enable_shared_from_this<CServiceServerImpl>
  {
  public:
    static std::shared_ptr<CServiceServerImpl> CreateInstance();
    static std::shared_ptr<CServiceServerImpl> CreateInstance(const std::string& service_name_);

  private:
    CServiceServerImpl();

    // Delete copy and move constructors and assign operators
    CServiceServerImpl(const CServiceServerImpl&) = delete;             // Copy construct
    CServiceServerImpl(CServiceServerImpl&&) = delete;                  // Move construct
    CServiceServerImpl& operator=(const CServiceServerImpl&) = delete;  // Copy assign
    CServiceServerImpl& operator=(CServiceServerImpl&&) = delete;       // Move assign

  public:

    // TODO: The this pointer is used internally, so this class must not be copyable or movable.

    ~CServiceServerImpl();

    bool Create(const std::string& service_name_);

    bool Destroy();

    bool AddDescription(const std::string& method_, const std::string& req_type_, const std::string& req_desc_, const std::string& resp_type_, const std::string& resp_desc_);
      
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

  protected:
    void Register(bool force_);
    void Unregister();

    /**
     * @brief Calls the request callback based on the request and fills the response
     * 
     * @param[in]  request_   The service request in serialized protobuf form
     * @param[out] response_  A serialized protobuf response. My not be set at all.
     * 
     * @return  0 if succeeded, -1 if not.
     */
    int RequestCallback(const std::string& request_, std::string& response_);
    void EventCallback(eCAL_Server_Event event_, const std::string& message_);

    bool ApplyServiceToDescGate(const std::string& method_name_
      , const std::string& req_type_name_
      , const std::string& req_type_desc_
      , const std::string& resp_type_name_
      , const std::string& resp_type_desc_);

    std::shared_ptr<eCAL::service::Server> m_tcp_server_v0;
    std::shared_ptr<eCAL::service::Server> m_tcp_server_v1;

    static constexpr int  m_server_version = 1;
    
    std::string           m_service_name;
    std::string           m_service_id;

    struct SMethod
    {
      eCAL::pb::Method method_pb;
      MethodCallbackT  callback;
    };
    std::mutex            m_method_map_sync;
    using MethodMapT = std::map<std::string, SMethod>;
    MethodMapT m_method_map;

    std::mutex            m_event_callback_map_sync;
    using EventCallbackMapT = std::map<eCAL_Server_Event, ServerEventCallbackT>;
    EventCallbackMapT     m_event_callback_map;
    
    bool                  m_created      = false;
    bool                  m_connected_v0 = false;
    bool                  m_connected_v1 = false;
  };
}
