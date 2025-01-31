/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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
#include <ecal/deprecate.h>
#include <ecal/v5/ecal_callback.h>
#include <ecal/service/types.h>
#include <ecal/types.h>
#include <ecal_service/client_session.h>

#include "serialization/ecal_serialize_sample_registration.h"
#include "serialization/ecal_struct_service.h"

#include <map>
#include <mutex>
#include <memory>
#include <vector>

namespace eCAL
{
    /**
     * @brief Service client implementation class.
    **/
    class CServiceClientImpl : public std::enable_shared_from_this<CServiceClientImpl>
    {
    public:
      // Factory method to create an instance of the client implementation
      static std::shared_ptr<CServiceClientImpl> CreateInstance(
        const std::string& service_name_, const ServiceMethodInformationSetT& method_information_map_, const ClientEventCallbackT& event_callback_);

    private:
      // Private constructor to enforce creation through factory method
      CServiceClientImpl(const std::string& service_name_, const ServiceMethodInformationSetT& method_information_map_, const ClientEventCallbackT& event_callback_);

    public:
      ~CServiceClientImpl();

      // Retrieve service IDs of all matching services
      std::vector<SEntityId> GetServiceIDs();

      // Blocking call to a specific service; returns response as pair<bool, SServiceResponse>
      // if a callback is provided call the callback as well
      std::pair<bool, SServiceResponse> CallWithCallback(
        const SEntityId& entity_id_, const std::string& method_name_,
        const std::string& request_, const ResponseCallbackT& response_callback_, int timeout_ms_);

      // Asynchronous call to a specific service using callback
      bool CallWithCallbackAsync(
        const SEntityId& entity_id_, const std::string& method_name_,
        const std::string& request_, const ResponseCallbackT& response_callback_);

      // Check connection state of a specific service
      bool IsConnected(const SEntityId& entity_id_);

      // Called by the registration receiver to process a service registration
      void RegisterService(const SEntityId& entity_id_, const v5::SServiceAttr& service_);

      // Called by the registration provider to get a registration sample
      Registration::Sample GetRegistration();

      // Retrieves the service id
      SServiceId GetServiceId() const;

      // Retrieves the service name
      std::string GetServiceName() const;

      // Prevent copy and move operations
      CServiceClientImpl(const CServiceClientImpl&) = delete;
      CServiceClientImpl& operator=(const CServiceClientImpl&) = delete;
      CServiceClientImpl(CServiceClientImpl&&) = delete;
      CServiceClientImpl& operator=(CServiceClientImpl&&) = delete;

    private:
      // Prepare and retrieve registration and unregistration samples
      Registration::Sample GetRegistrationSample();
      Registration::Sample GetUnregistrationSample();

      // SClient struct representing a client session and its connection state
      struct SClient
      {
        v5::SServiceAttr service_attr;
        std::shared_ptr<ecal_service::ClientSession> client_session;
        bool connected = false;
      };

      // Get client for specific entity id
      bool GetClientByEntity(const SEntityId& entity_id_, SClient& client_);

      // Blocking call to a specific service method with timeout
      std::pair<bool, SServiceResponse> CallMethodWithTimeout(const SEntityId& entity_id_, SClient& client_,
        const std::string& method_name_, const std::string& request_, std::chrono::nanoseconds timeout_);

      // Update the connection states for client sessions
      void UpdateConnectionStates();

      // Increment method call count for tracking
      void IncrementMethodCallCount(const std::string& method_name_);

      // Notify specific event callback
      void NotifyEventCallback(const SServiceId& service_id_, eClientEvent event_type_);

      // SResponseData struct for handling response callbacks
      struct SResponseData
      {
        std::shared_ptr<std::mutex>                          mutex;
        std::shared_ptr<std::condition_variable>             condition_variable;
        std::shared_ptr<std::pair<bool, SServiceResponse>> response;
        std::shared_ptr<bool>                                block_modifying_response;
        std::shared_ptr<bool>                                finished;

        SResponseData() :
          mutex(std::make_shared<std::mutex>()),
          condition_variable(std::make_shared<std::condition_variable>()),
          response(std::make_shared<std::pair<bool, SServiceResponse>>(false, SServiceResponse())),
          block_modifying_response(std::make_shared<bool>(false)),
          finished(std::make_shared<bool>(false))
        {}
      };

      static std::shared_ptr<SResponseData> PrepareInitialResponse(const SClient& client_, const std::string& method_name_);
      static ecal_service::ClientResponseCallbackT CreateResponseCallback(const SClient& client_, const std::shared_ptr<SResponseData>& response_data_);

      static SServiceResponse DeserializedResponse(const SClient& client_, const std::string& response_pb_);

      // Client version (incremented for protocol or functionality changes)
      static constexpr int         m_client_version = 1;

      // Service attributes
      std::string                  m_service_name;
      EntityIdT      m_client_id;

      // Client session map and synchronization
      using ClientSessionsMapT = std::map<SEntityId, SClient>;
      std::mutex                   m_client_session_map_mutex;
      ClientSessionsMapT           m_client_session_map;

      // Method information map (tracks method attributes like data type and description)
      std::mutex                   m_method_information_set_mutex;
      ServiceMethodInformationSetT        m_method_information_set;

      // Method call count map (tracks number of calls for each method)
      using MethodCallCountMapT = std::map<std::string, uint64_t>;
      MethodCallCountMapT          m_method_call_count_map;

      // Event callback map and synchronization
      std::mutex                   m_event_callback_mutex;
      ClientEventCallbackT       m_event_callback;
    };
}
