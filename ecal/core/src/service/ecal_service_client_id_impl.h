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
#include <ecal/ecal_types.h>
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
  class CServiceClientIDImpl : public std::enable_shared_from_this<CServiceClientIDImpl>
  {
  public:
    static std::shared_ptr<CServiceClientIDImpl> CreateInstance(const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_);

  private:
    // private constructor to enforce creation through factory method
    CServiceClientIDImpl(const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_);

  public:
    ~CServiceClientIDImpl();

    // add and remove callback function for service response
    bool AddResponseCallback(const ResponseIDCallbackT& callback_);
    bool RemoveResponseCallback();

    // add and remove callback function for client events
    bool AddEventCallback(eCAL_Client_Event type_, ClientEventCallbackT callback_);
    bool RemoveEventCallback(eCAL_Client_Event type_);

    // return service id's of all matching services
    std::vector<Registration::SEntityId> GetServiceIDs();
      
    // blocking call specific service, response will be returned as pair<bool, SServiceReponse>
    std::pair<bool, SServiceResponse> CallWithResponse(const Registration::SEntityId& entity_id_, const std::string& method_name_, const std::string& request_, int timeout_ms_);

    // blocking call specific service, using callback
    bool CallWithCallback(const Registration::SEntityId& entity_id_, const std::string& method_name_, const std::string& request_, int timeout_ms_);

    // check connection state of specific service
    bool IsConnected(const Registration::SEntityId& entity_id_);

    // check connection
    bool IsConnected();

    // called by the registration receiver to process a service registration
    void RegisterService(const Registration::SEntityId& entity_id_, const SServiceAttr& service_);

    // called by the registration layer to get registration sample
    Registration::Sample GetRegistration();

    // this object must not be copied and moved
    CServiceClientIDImpl(const CServiceClientIDImpl&) = delete;
    CServiceClientIDImpl& operator=(const CServiceClientIDImpl&) = delete;
    CServiceClientIDImpl(CServiceClientIDImpl&&) = delete;
    CServiceClientIDImpl& operator=(CServiceClientIDImpl&&) = delete;

  private:
    std::pair<bool, SServiceResponse> CServiceClientIDImpl::CallBlocking(const Registration::SEntityId& entity_id_, const std::string& method_name_, const std::string& request_, std::chrono::nanoseconds timeout_);

    Registration::Sample GetRegistrationSample();
    Registration::Sample GetUnregistrationSample();

    void Register();
    void Unregister();

    void UpdateConnectionStates();
    void ErrorCallback(const Registration::SEntityId& entity_id_, const std::string& method_name_, const std::string& error_message_);
    void IncrementMethodCallCount(const std::string& method_name_);

    static constexpr int          m_client_version = 1;

    // service attributes
    std::string                   m_service_name;
    std::string                   m_client_id;

    // client session map
    struct SClient
    {
      SServiceAttr                                  service_attr;
      std::shared_ptr<eCAL::service::ClientSession> client_session;
      bool                                          connected = false;
    };
    using ClientSessionsMapT = std::map<Registration::SEntityId, SClient>;
    std::mutex                    m_client_session_map_sync;
    ClientSessionsMapT            m_client_session_map;

    // method informations (methods and its attributes like data type and data description)
    std::mutex                    m_method_information_map_sync;
    ServiceMethodInformationMapT  m_method_information_map;

    // method call count
    using MethodCallCountMapT = std::map<std::string, uint64_t>;
    MethodCallCountMapT           m_method_call_count_map;

    // response callback
    std::mutex                    m_response_callback_sync;
    ResponseIDCallbackT           m_response_callback;

    // event callback
    std::mutex                    m_event_callback_map_sync;
    using EventCallbackMapT = std::map<eCAL_Client_Event, ClientEventCallbackT>;
    EventCallbackMapT             m_event_callback_map;
  };
}
