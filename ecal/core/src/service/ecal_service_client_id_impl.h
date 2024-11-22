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
    // Factory method to create an instance of the client implementation
    static std::shared_ptr<CServiceClientIDImpl> CreateInstance(
      const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_);

  private:
    // Private constructor to enforce creation through factory method
    CServiceClientIDImpl(const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_);

  public:
    ~CServiceClientIDImpl();

    // Add and remove callback function for client events
    bool AddEventCallback(eCAL_Client_Event type_, ClientEventIDCallbackT callback_);
    bool RemoveEventCallback(eCAL_Client_Event type_);

    // Retrieve service IDs of all matching services
    std::vector<Registration::SEntityId> GetServiceIDs();

    // Blocking call to a specific service; returns response as pair<bool, SServiceResponse>
    std::pair<bool, SServiceResponse> CallWithResponse(
      const Registration::SEntityId& entity_id_, const std::string& method_name_,
      const std::string& request_, int timeout_ms_);

    // Blocking call to a specific service using callback
    bool CallWithCallback(
      const Registration::SEntityId& entity_id_, const std::string& method_name_,
      const std::string& request_, int timeout_ms_, const ResponseIDCallbackT& repsonse_callback_);

    // Check connection state of a specific service
    bool IsConnected(const Registration::SEntityId& entity_id_);

    // Called by the registration receiver to process a service registration
    void RegisterService(const Registration::SEntityId& entity_id_, const SServiceAttr& service_);

    // Called by the registration layer to get a registration sample
    Registration::Sample GetRegistration();

    // Prevent copy and move operations
    CServiceClientIDImpl(const CServiceClientIDImpl&) = delete;
    CServiceClientIDImpl& operator=(const CServiceClientIDImpl&) = delete;
    CServiceClientIDImpl(CServiceClientIDImpl&&) = delete;
    CServiceClientIDImpl& operator=(CServiceClientIDImpl&&) = delete;

  private:
    // SClient struct representing a client session and its connection state
    struct SClient
    {
      SServiceAttr service_attr;
      std::shared_ptr<eCAL::service::ClientSession> client_session;
      bool connected = false;
    };

    // SResponseData struct for handling response callbacks
    struct SResponseData
    {
      std::shared_ptr<std::mutex>                        mutex;
      std::shared_ptr<std::condition_variable>           condition_variable;
      std::shared_ptr<std::pair<bool, SServiceResponse>> response;
      std::shared_ptr<bool>                              block_modifying_response;
      std::shared_ptr<bool>                              finished;

      SResponseData() :
        mutex(std::make_shared<std::mutex>()),
        condition_variable(std::make_shared<std::condition_variable>()),
        response(std::make_shared<std::pair<bool, SServiceResponse>>(false, SServiceResponse())),
        block_modifying_response(std::make_shared<bool>(false)),
        finished(std::make_shared<bool>(false))
      {}
    };

    // Initializes the method call count map
    void InitializeMethodCallCounts();

    // Generates a unique client ID
    void GenerateClientID();

    // Resets all callbacks and clears stored client information
    void ResetAllCallbacks();

    // Blocking call to a specific service with timeout
    std::pair<bool, SServiceResponse> CallBlocking(SClient& client_, const std::string& method_name_,
      const std::string& request_, std::chrono::nanoseconds timeout_);

    // Prepare and retrieve registration and unregistration samples
    Registration::Sample GetRegistrationSample();
    Registration::Sample GetUnregistrationSample();

    // Register and unregister client information
    void Register();
    void Unregister();

    // Update the connection states for client sessions
    void UpdateConnectionStates();

    // Notify error callback with specific details
    //void ErrorCallback(const Registration::SEntityId& entity_id_, const std::string& method_name_,
    //  const std::string& error_message_);

    // Increment method call count for tracking
    void IncrementMethodCallCount(const std::string& method_name_);

    // Helper methods for client session handling and request serialization
    bool TryGetClient(const Registration::SEntityId& entity_id_, SClient& client_);
    std::shared_ptr<std::string> SerializeRequest(const std::string& method_name_, const std::string& request_);
    std::pair<bool, SServiceResponse> WaitForResponse(SClient& client_, const std::string& method_name_,
      std::chrono::nanoseconds timeout_,
      std::shared_ptr<std::string> request_shared_ptr_);

    std::shared_ptr<SResponseData> PrepareInitialResponse(SClient& client_, const std::string& method_name_);
    eCAL::service::ClientResponseCallbackT CreateResponseCallback(std::shared_ptr<SResponseData> response_data_);
    SServiceResponse PrepareErrorResponse(const std::string& error_message_);

    // Notify specific event callback
    void NotifyEventCallback(eCAL_Client_Event event_type_, const SServiceAttr& service_attr_);

    // Client version (incremented for protocol or functionality changes)
    static constexpr int m_client_version = 1;

    // Service attributes
    std::string m_service_name;
    std::string m_client_id;

    // Client session map and synchronization
    using ClientSessionsMapT = std::map<Registration::SEntityId, SClient>;
    std::mutex         m_client_session_map_sync;
    ClientSessionsMapT m_client_session_map;

    // Method information map (tracks method attributes like data type and description)
    std::mutex                   m_method_information_map_sync;
    ServiceMethodInformationMapT m_method_information_map;

    // Method call count map (tracks number of calls for each method)
    using MethodCallCountMapT = std::map<std::string, uint64_t>;
    MethodCallCountMapT m_method_call_count_map;

    // Event callback map and synchronization
    std::mutex m_event_callback_map_sync;
    using EventCallbackMapT = std::map<eCAL_Client_Event, ClientEventIDCallbackT>;
    EventCallbackMapT m_event_callback_map;
  };
}
