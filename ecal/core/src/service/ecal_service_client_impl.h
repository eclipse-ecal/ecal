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
 * @brief  eCAL service client implementation
**/

#include <ecal/ecal.h>
#include <ecal/ecal_callback.h>

#include "service/ecal_tcpclient.h"

#include <map>
#include <mutex>

namespace eCAL
{
  /**
   * @brief Service client implementation class. 
  **/
  class CServiceClientImpl
  {
  public:
    CServiceClientImpl();
    CServiceClientImpl(const std::string& service_name_);

    ~CServiceClientImpl();

    bool Create(const std::string& service_name_);

    bool Destroy();

    bool SetHostName(const std::string& host_name_);

    // add and remove callback function for service response
    bool AddResponseCallback(const ResponseCallbackT& callback_);
    bool RemResponseCallback();

    // add and remove callback function for client events
    bool AddEventCallback(eCAL_Client_Event type_, ClientEventCallbackT callback_);
    bool RemEventCallback(eCAL_Client_Event type_);
      
    // blocking call, no broadcast, first matching service only, response will be returned in service_response_
    [[deprecated]]
    bool Call(const std::string& method_name_, const std::string& request_, struct SServiceResponse& service_response_);
    
    // blocking call, all responses will be returned in service_response_vec_
    bool Call(const std::string& method_name_, const std::string& request_, int timeout_, ServiceResponseVecT* service_response_vec_);

    // blocking call, using callback
    bool Call(const std::string& method_name_, const std::string& request_, int timeout_);

    // asynchronously call, using callback (timeout not supported yet)
    bool CallAsync(const std::string& method_name_, const std::string& request_ /*, int timeout_*/);

    // check connection state
    bool IsConnected();

    // called by eCAL::CClientGate to register a service
    void RegisterService(const std::string& key_, unsigned int version_, const SServiceAttr& service_);

    // called by eCAL:CClientGate every second to update registration layer
    void RefreshRegistration();

    std::string GetServiceName() { return m_service_name; };

    // this object must not be copied and moved
    CServiceClientImpl(const CServiceClientImpl&) = delete;
    CServiceClientImpl& operator=(const CServiceClientImpl&) = delete;
    CServiceClientImpl(CServiceClientImpl&&) = delete;
    CServiceClientImpl& operator=(CServiceClientImpl&&) = delete;

  protected:
    void Register(bool force_);
    void Unregister();

    void CheckForNewServices();

    bool SendRequests(const std::string& host_name_, const std::string& method_name_, const std::string& request_, int timeout_);
    bool SendRequest(const std::shared_ptr<CTcpClient>& client_, const std::string& method_name_, const std::string& request_, int timeout_, struct SServiceResponse& service_response_);

    void SendRequestAsync(const std::shared_ptr<CTcpClient>& client_, const std::string& method_name_, const std::string& request_, int timeout_);

    void ErrorCallback(const std::string &method_name_, const std::string &error_message_);

    using ClientMapT = std::map<std::string, std::shared_ptr<CTcpClient>>;
    std::mutex            m_client_map_sync;
    ClientMapT            m_client_map;

    std::mutex            m_response_callback_sync;
    ResponseCallbackT     m_response_callback;

    std::mutex            m_event_callback_map_sync;
    using EventCallbackMapT = std::map<eCAL_Client_Event, ClientEventCallbackT>;
    EventCallbackMapT     m_event_callback_map;

    std::mutex            m_connected_services_map_sync;
    using ServiceAttrMapT = std::map<std::string, SServiceAttr>;
    ServiceAttrMapT       m_connected_services_map;

    static constexpr int  m_version = 0;
    std::string           m_service_name;
    std::string           m_service_id;
    std::string           m_host_name;

    bool                  m_created;
  };
}
