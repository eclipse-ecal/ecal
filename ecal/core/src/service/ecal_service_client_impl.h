/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
#include <ecal/ecal_deprecate.h>
#include <ecal/ecal_callback.h>
#include <ecal/ecal_service_info.h>
#include <ecal/service/client_session.h>

#include "serialization/ecal_serialize_sample_registration.h"
#include "serialization/ecal_struct_service.h"

#include <map>
#include <mutex>
#include <memory>

namespace eCAL
{
  /**
   * @brief Service client implementation class. 
  **/
  class CServiceClientImpl : public std::enable_shared_from_this<CServiceClientImpl>
  {
  public:
    static std::shared_ptr<CServiceClientImpl> CreateInstance();
    static std::shared_ptr<CServiceClientImpl> CreateInstance(const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_);
  
  private:
    CServiceClientImpl();

  public:
    ~CServiceClientImpl();

    bool Start(const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_);
    bool Stop();

    bool SetHostName(const std::string& host_name_);

    // add and remove callback function for service response
    bool AddResponseCallback(const ResponseCallbackT& callback_);
    bool RemResponseCallback();

    // add and remove callback function for client events
    bool AddEventCallback(eCAL_Client_Event type_, ClientEventCallbackT callback_);
    bool RemEventCallback(eCAL_Client_Event type_);
      
    // blocking call, all responses will be returned in service_response_vec_
    bool Call(const std::string& method_name_, const std::string& request_, int timeout_ms_, ServiceResponseVecT* service_response_vec_);

    // blocking call, using callback
    bool Call(const std::string& method_name_, const std::string& request_, int timeout_ms_);

    // asynchronously call, using callback (timeout not supported yet)
    bool CallAsync(const std::string& method_name_, const std::string& request_ /*, int timeout_ms_*/);

    // check connection state
    bool IsConnected();

    // called by the eCAL::CClientGate to register a service
    void RegisterService(const std::string& key_, const SServiceAttr& service_);

    // called by eCAL:CClientGate every second to update registration layer
    Registration::Sample GetRegistration();

    std::string GetServiceName() { return m_service_name; };

    // this object must not be copied and moved
    CServiceClientImpl(const CServiceClientImpl&) = delete;
    CServiceClientImpl& operator=(const CServiceClientImpl&) = delete;
    CServiceClientImpl(CServiceClientImpl&&) = delete;
    CServiceClientImpl& operator=(CServiceClientImpl&&) = delete;

  private:
    std::shared_ptr<std::vector<std::pair<bool, eCAL::SServiceResponse>>> CallBlocking(const std::string& method_name_, const std::string& request_, std::chrono::nanoseconds timeout_);

    static void fromSerializedProtobuf(const std::string& response_pb_, eCAL::SServiceResponse& response_);
    static void fromStruct(const Service::Response& response_struct_, eCAL::SServiceResponse& response_);

    Registration::Sample GetRegistrationSample();
    Registration::Sample GetUnregistrationSample();
    
    void Register();
    void Unregister();

    void CheckForNewServices();
    void CheckForDisconnectedServices();

    void ErrorCallback(const std::string &method_name_, const std::string &error_message_);

    void IncrementMethodCallCount(const std::string& method_name_);

    using ClientMapT = std::map<std::string, std::shared_ptr<eCAL::service::ClientSession>>;
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

    static constexpr int  m_client_version = 1;

    std::string           m_service_name;
    std::string           m_service_id;
    std::string           m_host_name;

    std::mutex                   m_method_sync;
    ServiceMethodInformationMapT m_method_information_map;

    using MethodCallCountMapT = std::map<std::string, uint64_t>;
    MethodCallCountMapT   m_method_call_count_map;

    std::atomic<bool>     m_created;
  };
}
