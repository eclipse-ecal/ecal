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
 * @brief  eCAL service server implementation
**/

#pragma once

#include <ecal/namespace.h>
#include <ecal/v5/ecal_callback.h>
#include <ecal/service/types.h>
#include <ecal_service/server.h>

#include "serialization/ecal_serialize_sample_registration.h"
#include "serialization/ecal_struct_service.h"

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace eCAL
{
  /**
   * @brief Implementation class for eCAL service server.
   */
  class CServiceServerImpl : public std::enable_shared_from_this<CServiceServerImpl>
  {
  public:
    // Factory method to create an instance of the client implementation
    static std::shared_ptr<CServiceServerImpl> CreateInstance(
      const std::string& service_name_, const ServerEventCallbackT& event_callback_);

  private:
    // Private constructor to enforce creation through factory method
    CServiceServerImpl(const std::string& service_name_, const ServerEventCallbackT& event_callback_);

  public:
    ~CServiceServerImpl();

    bool SetMethodCallback(const SServiceMethodInformation& method_info_, const ServiceMethodCallbackT& callback_);
    bool RemoveMethodCallback(const std::string& method_);

    // Check connection state of a specific service
    bool IsConnected() const;

    // Called by the registration receiver to process a client registration
    void RegisterClient(const std::string& key_, const v5::SClientAttr& client_);

    // Called by the registration provider to get a registration sample
    Registration::Sample GetRegistration();

    // Retrieves the service id
    SServiceId GetServiceId() const;

    // Retrieves the service name
    std::string GetServiceName() const;

    // Prevent copy and move operations
    CServiceServerImpl(const CServiceServerImpl&) = delete;
    CServiceServerImpl& operator=(const CServiceServerImpl&) = delete;
    CServiceServerImpl(CServiceServerImpl&&) = delete;
    CServiceServerImpl& operator=(CServiceServerImpl&&) = delete;

  private:
    // Start/Stop server
    void Start();
    void Stop();

    // Prepare and retrieve registration and unregistration samples
    Registration::Sample GetRegistrationSample();
    Registration::Sample GetUnregistrationSample();

    // Request and event callback methods
    int RequestCallback(const std::string& request_pb_, std::string& response_pb_);
    void NotifyEventCallback(const SServiceId& service_id_, eServerEvent event_type_, const std::string& message_);

    // Server version (incremented for protocol or functionality changes)
    static constexpr int                   m_server_version = 1;

    // Server attributes
    std::string                            m_service_name;
    EntityIdT                m_service_id;

    // Server connection state and synchronization
    mutable std::mutex                     m_connected_mutex; // mutex protecting m_connected (modified by the event callbacks in another thread)
    bool                                   m_connected = false;
    std::atomic<bool>                      m_created;

    // Server method map and synchronization
    struct SMethod
    {
      Service::Method     method;
      ServiceMethodCallbackT callback;
    };

    using MethodMapT = std::map<std::string, SMethod>;
    std::mutex                             m_method_map_mutex;
    MethodMapT                             m_method_map;

    // Event callback and synchronization
    std::mutex                             m_event_callback_mutex;
    ServerEventCallbackT                   m_event_callback;

    // Server interface
    std::shared_ptr<ecal_service::Server> m_tcp_server;
  };
}
