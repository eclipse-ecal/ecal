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

#include "ecal_global_accessors.h"

#include "ecal_clientgate.h"
#include "ecal_service_client_impl.h"
#include "ecal_service_singleton_manager.h"
#include "registration/ecal_registration_provider.h"
#include "serialization/ecal_serialize_service.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace eCAL
{
  std::shared_ptr<CServiceClientImpl> CServiceClientImpl::CreateInstance()
  {
    return(std::shared_ptr<CServiceClientImpl>(new CServiceClientImpl()));
  }

  std::shared_ptr<CServiceClientImpl> CServiceClientImpl::CreateInstance(const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_)
  {
    auto instance = std::shared_ptr<CServiceClientImpl>(new CServiceClientImpl());
    instance->Start(service_name_, method_information_map_);
    return instance;
  }

  /**
   * @brief Service client implementation class.
  **/
  CServiceClientImpl::CServiceClientImpl() :
    m_response_callback(nullptr),
    m_created(false)
  {
  }

  CServiceClientImpl::~CServiceClientImpl()
  {
    Stop();
  }

  bool CServiceClientImpl::Start(const std::string& service_name_, const ServiceMethodInformationMapT& method_information_map_)
  {
    if (m_created) return(false);

    // set service name and methods
    m_service_name           = service_name_;
    m_method_information_map = method_information_map_;
    
    // initialize call count map
    for (const auto& method_information_pair : m_method_information_map)
    {
      m_method_call_count_map[method_information_pair.first] = 0;
    }

    // create service id
    std::stringstream counter;
    counter << std::chrono::steady_clock::now().time_since_epoch().count();
    m_service_id = counter.str();

    // mark as created
    m_created = true;

    // register this client
    Register();

    return(true);
  }

  bool CServiceClientImpl::Stop()
  {
    if (!m_created) return(false);

    // reset client map
    {
      std::lock_guard<std::mutex> const lock(m_client_map_sync);
      m_client_map.clear();
    }

    // reset method callback map
    {
      std::lock_guard<std::mutex> const lock(m_response_callback_sync);
      m_response_callback = nullptr;
    }

    // reset event callback map
    {
      std::lock_guard<std::mutex> const lock(m_event_callback_map_sync);
      m_event_callback_map.clear();
    }

    // mark as no more created
    m_created = false;

    // and unregister this client
    Unregister();

    // reset internals
    m_service_name.clear();
    m_service_id.clear();
    m_host_name.clear();

    return(true);
  }

  bool CServiceClientImpl::SetHostName(const std::string& host_name_)
  {
    if (host_name_ == "*") m_host_name.clear();
    else                   m_host_name = host_name_;
    return(true);
  }

  // add callback function for service response
  bool CServiceClientImpl::AddResponseCallback(const ResponseCallbackT& callback_)
  {
    std::lock_guard<std::mutex> const lock(m_response_callback_sync);
    m_response_callback = callback_;
    return true;
  }

  // remove callback function for service response
  bool CServiceClientImpl::RemResponseCallback()
  {
    std::lock_guard<std::mutex> const lock(m_response_callback_sync);
    m_response_callback = nullptr;
    return true;
  }

  // add callback function for client events
  bool CServiceClientImpl::AddEventCallback(eCAL_Client_Event type_, ClientEventCallbackT callback_)
  {
    if (!m_created) return false;

    // store event callback
    {
      std::lock_guard<std::mutex> const lock(m_event_callback_map_sync);
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug2, m_service_name + "::CServiceClientImpl::AddEventCallback");
#endif
      m_event_callback_map[type_] = std::move(callback_);
    }

    return true;
  }

  // remove callback function for client events
  bool CServiceClientImpl::RemEventCallback(eCAL_Client_Event type_)
  {
    if (!m_created) return false;

    // reset event callback
    {
      std::lock_guard<std::mutex> const lock(m_event_callback_map_sync);
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug2, m_service_name + "::CServiceClientImpl::RemEventCallback");
#endif
      m_event_callback_map[type_] = nullptr;
    }

    return true;
  }

  // blocking call, all responses will be returned in service_response_vec_
  bool CServiceClientImpl::Call(const std::string& method_name_, const std::string& request_, int timeout_ms_, ServiceResponseVecT* service_response_vec_)
  {
    if(service_response_vec_ == nullptr) return false;
    service_response_vec_->clear();

    auto responses = CallBlocking(method_name_, request_, std::chrono::milliseconds(timeout_ms_));
    if (!responses)
    {
      return false;
    }
    else
    {
      // Copy our temporary return vector to the user's return vector
      service_response_vec_->resize(responses->size());
      for (size_t i = 0; i < responses->size(); i++)
      {
        (*service_response_vec_)[i] = (*responses)[i].second;
      }

      // Call the timeout callback for all services that have not returned yet
      if (timeout_ms_ > 0)
      {
        std::lock_guard<std::mutex> const lock_eb(m_event_callback_map_sync);
        auto callback_it = m_event_callback_map.find(eCAL_Client_Event::client_event_timeout);
        if (callback_it != m_event_callback_map.end())
        {
          for (const auto& return_pair : (*responses))
          {
            if (!return_pair.first)
            {
              SClientEventCallbackData sdata;
              sdata.type = client_event_timeout;
              sdata.time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
              (callback_it->second)(m_service_name.c_str(), &sdata);
            }
          }
        }
      }

      // Determine if any call has been successful
      for (size_t i = 0; i < responses->size(); i++)
      {
        if ((*responses)[i].second.call_state == call_state_executed)
          return true;
      }
      return false;
    }
  }

  // blocking call, using callback
  bool CServiceClientImpl::Call(const std::string& method_name_, const std::string& request_, int timeout_ms_)
  {
    auto responses = CallBlocking(method_name_, request_, std::chrono::milliseconds(timeout_ms_));

    if (!responses)
    {
      return false;
    }
    else
    {
      // iterate over responses and call the response callbacks
      {
        std::lock_guard<std::mutex> const lock_cb(m_response_callback_sync);
        if (m_response_callback)
        {
          for (const auto& response_pair : *responses)
          {
            if (response_pair.first)
            {
              // Call callback, if the call didn't timeout
               m_response_callback(response_pair.second);
            }
          }
        }
      }

      // iterate over responses and call the timeout callbacks
      if (timeout_ms_ > 0)
      {
        std::lock_guard<std::mutex> const lock_eb(m_event_callback_map_sync);
        auto callback_it = m_event_callback_map.find(eCAL_Client_Event::client_event_timeout);
        if (callback_it != m_event_callback_map.end())
        {
          for (const auto& return_pair : (*responses))
          {
            if (!return_pair.first)
            {
              SClientEventCallbackData sdata;
              sdata.type = client_event_timeout;
              sdata.time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
              (callback_it->second)(m_service_name.c_str(), &sdata);
            }
          }
        }
      }

      // Determine if any call has been successful and return true, if so.
      for (size_t i = 0; i < responses->size(); i++)
      {
        if ((*responses)[i].second.call_state == call_state_executed)
          return true;
      }
      return false;
    }
  }

  // asynchronously call, using callback
  bool CServiceClientImpl::CallAsync(const std::string& method_name_, const std::string& request_ /*, int timeout_ms_*/)
  {
    // TODO: implement timeout

    if (g_clientgate() == nullptr)
    {
      ErrorCallback(method_name_, "Clientgate error.");
      return false;
    }

    if (!m_created)
    {
      ErrorCallback(method_name_, "Client hasn't been created yet.");
      return false;
    }

    if (m_service_name.empty()
      || method_name_.empty())
    {
      ErrorCallback(method_name_, "Invalid service or method name.");
      return false;
    }

    // check for new server
    CheckForNewServices();

    // Copy raw request in a protocol buffer
    // TODO: The next version of the service protocol should omit the double-serialization (i.e. copying the binary data in a protocol buffer and then serializing that again)
    Service::Request request;
    request.header.mname = method_name_;
    request.request      = std::string(request_.data(), request_.size());

    // serialize request
    auto request_shared_ptr = std::make_shared<std::string>();
    SerializeToBuffer(request, *request_shared_ptr);

    bool at_least_one_service_was_called (false);

    // Call all services
    std::vector<SServiceAttr> const service_vec = g_clientgate()->GetServiceAttr(m_service_name);
    for (const auto& service : service_vec)
    {
      if (m_host_name.empty() || (m_host_name == service.hname))
      {
        std::lock_guard<std::mutex> const lock(m_client_map_sync);
        auto client = m_client_map.find(service.key);
        if (client != m_client_map.end())
        {
          const eCAL::service::ClientResponseCallbackT response_callback
                      = [weak_me = std::weak_ptr<CServiceClientImpl>(shared_from_this()), hostname = service.hname, servicename = service.sname]
                        (const eCAL::service::Error& response_error, const std::shared_ptr<std::string>& response_)
                        {
                          auto me = weak_me.lock();
                          if (!me)
                          {
                            return;
                          }

                          std::lock_guard<std::mutex> const lock(me->m_response_callback_sync);
                          
                          if (me->m_response_callback)
                          {
                            eCAL::SServiceResponse service_response_struct;
                            
                            service_response_struct.host_name    = hostname;
                            service_response_struct.service_name = servicename;

                            if (response_error)
                            {
                              service_response_struct.error_msg    = response_error.ToString();
                              service_response_struct.call_state   = eCallState::call_state_failed;
                              service_response_struct.ret_state    = 0;
                            }
                            else
                            {
                              fromSerializedProtobuf(*response_, service_response_struct);
                            }

                            me->m_response_callback(service_response_struct);
                          }
                        };
          
          if (client->second->async_call_service(request_shared_ptr, response_callback))
          {
            IncrementMethodCallCount(method_name_);
            at_least_one_service_was_called = true;
          }
        }
      }
    }

    return(at_least_one_service_was_called);
  }

  // check connection state
  bool CServiceClientImpl::IsConnected()
  {
    if (!m_created) return false;

    // check for connected clients
    std::lock_guard<std::mutex> const lock(m_connected_services_map_sync);
    return !m_connected_services_map.empty();
  }

  // called by the eCAL::CClientGate to register a service
  void CServiceClientImpl::RegisterService(const std::string& key_, const SServiceAttr& service_)
  {
    // check connections
    std::lock_guard<std::mutex> const lock(m_connected_services_map_sync);

    // is this a new connection ?
    if (m_connected_services_map.find(key_) == m_connected_services_map.end())
    {
      // call connect event
      std::lock_guard<std::mutex> const lock_eb(m_event_callback_map_sync);
      auto e_iter = m_event_callback_map.find(client_event_connected);
      if (e_iter != m_event_callback_map.end())
      {
        SClientEventCallbackData sdata;
        sdata.type = client_event_connected;
        sdata.time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        sdata.attr = service_;
        (e_iter->second)(m_service_name.c_str(), &sdata);
      }
      // add service
      m_connected_services_map[key_] = service_;
    }
  }

  // called by eCAL:CClientGate every second to update registration layer
  Registration::Sample CServiceClientImpl::GetRegistration()
  {
    // refresh connected services map
    CheckForNewServices();

    // check for disconnected services
    CheckForDisconnectedServices();

    return GetRegistrationSample();
  }

  std::shared_ptr<std::vector<std::pair<bool, eCAL::SServiceResponse>>>
                CServiceClientImpl::CallBlocking(const std::string&         method_name_
                                                , const std::string&        request_
                                                , std::chrono::nanoseconds  timeout_)
  {
    if (g_clientgate() == nullptr) return nullptr;
    if (!m_created)                return nullptr;

    if (m_service_name.empty() || method_name_.empty())
      return nullptr;

    // check for new server
    CheckForNewServices();

    // Copy raw request in a protocol buffer
    // TODO: The next version of the service protocol should omit the double-serialization (i.e. copying the binary data in a protocol buffer and then serializing that again)
    Service::Request request;
    request.header.mname = method_name_;
    request.request      = std::string(request_.data(), request_.size());
    // serialize request
    auto request_shared_ptr = std::make_shared<std::string>();
    SerializeToBuffer(request, *request_shared_ptr);

    std::vector<SServiceAttr> const service_vec = g_clientgate()->GetServiceAttr(m_service_name);

    // Create a condition variable and a mutex to wait for the response
    // All variables are in shared pointers, as we need to pass them to the
    // callback function via the lambda capture. When the user uses the timeout,
    // this method may finish earlier than the service calls, so we need to make
    // sure the callbacks can still operate on those variables.
    const auto  mutex                       = std::make_shared<std::mutex>();
    const auto  condition_variable          = std::make_shared<std::condition_variable>();
    const auto  responses                   = std::make_shared<std::vector<std::pair<bool, SServiceResponse>>>(); // Vector with [has_returned, response] pairs, so we know where a timeout has happened.
    const auto  block_modifying_responses   = std::make_shared<bool>(false);
    const auto  finished_service_call_count = std::make_shared<int>(0);

    const auto  expected_service_call_count = std::make_shared<int>(0);

    // Iterate over all service sessions and call each of them.
    // Each successfull call will increment the finished_service_call_count.
    // By comparing that with the expected_service_call_count we later know
    // Whether all service calls have been completed.
    for (const auto& service : service_vec)
    {
      // Only call service if host name matches
      if (m_host_name.empty() || (m_host_name == service.hname))
      {
        // Lock mutex for iterating over client session map
        std::lock_guard<std::mutex> const client_map_lock(m_client_map_sync);

        // Find the actual client session in the map
        auto client = m_client_map.find(service.key);
        if (client != m_client_map.end())
        {
          eCAL::service::ClientResponseCallbackT response_callback;

          {
            const std::lock_guard<std::mutex> lock(*mutex);
            (*expected_service_call_count)++;
            responses->emplace_back();
            responses->back().first               = false; // If this stays false, we have a timout
            responses->back().second.host_name    = service.hname;
            responses->back().second.service_name = service.sname;
            responses->back().second.service_id   = service.key;
            responses->back().second.method_name  = method_name_;
            responses->back().second.error_msg    = "Timeout";
            responses->back().second.ret_state    = 0;
            responses->back().second.call_state   = eCallState::call_state_failed;
            responses->back().second.response     = "";

            // Create a response callback, that will set the response and notify the condition variable
            response_callback
                      = [mutex, condition_variable, responses, block_modifying_responses, finished_service_call_count, i = (responses->size() - 1)]
                        (const eCAL::service::Error& response_error, const std::shared_ptr<std::string>& response_)
                        {
                          const std::lock_guard<std::mutex> lock(*mutex);

                          if (!(*block_modifying_responses))
                          {
                            // This calback has not timeouted. This does not tell us anything about the success, though.
                            (*responses)[i].first = true;

                            if (response_error)
                            {
                              (*responses)[i].second.error_msg    = response_error.ToString();
                              (*responses)[i].second.call_state   = eCallState::call_state_failed;
                              (*responses)[i].second.ret_state    = 0;
                            }
                            else
                            {
                              fromSerializedProtobuf(*response_, (*responses)[i].second);
                            }
                          }
                          
                          (*finished_service_call_count)++;
                          condition_variable->notify_all();
                        };

            // Call service asynchronously
            const bool call_success = client->second->async_call_service(request_shared_ptr, response_callback);

            if (!call_success)
            {
              // If the call failed, we know that the callback will never be called.
              // Thus, we need to increment the finished_service_call_count here.
              (*finished_service_call_count)++;

              // We also store an error in the response vector
              responses->back().second.error_msg    = "Stopped by user";
              responses->back().second.ret_state    = 0;
              responses->back().second.call_state   = eCallState::call_state_failed;
            }
            else
            {
              IncrementMethodCallCount(method_name_);
            }

          } // unlock mutex

        }

      }
    }

    // Lock mutex, call service asynchronously and wait for the condition variable to be notified
    {
      std::unique_lock<std::mutex> lock(*mutex);
      if (timeout_ > std::chrono::nanoseconds::zero())
      {
        condition_variable->wait_for(lock
                                    , timeout_
                                    , [&expected_service_call_count, &finished_service_call_count]()
                                      {
                                        // Wait for all services to return something
                                        return *expected_service_call_count == *finished_service_call_count;
                                      });

      }
      else
      {
        condition_variable->wait(lock, [&expected_service_call_count, &finished_service_call_count]()
                                        {
                                          // Wait for all services to return something
                                          return *expected_service_call_count == *finished_service_call_count;
                                        });
      }
      // Stop the callbacks from modifying the responses vector. This is important
      //in a timeout case, as we want to preserve the vector from when the timeout
      // occured. There is no way to stop the service calls from succeeding after
      // that, but we don't want their values, any more.
      *block_modifying_responses = true;

      return responses;
    }
  }

  void CServiceClientImpl::fromSerializedProtobuf(const std::string& response_pb_, eCAL::SServiceResponse& response_)
  {
    Service::Response response;
    // TODO: The next version of the service protocol should omit the double-serialization (i.e. copying the binary data in a protocol buffer and then serializing that again)
    if (DeserializeFromBuffer(response_pb_.c_str(), response_pb_.size(), response))
    {
      fromStruct(response, response_);
    }
    else
    {
      response_.error_msg  = "Could not parse server response";
      response_.ret_state  = 0;
      response_.call_state = eCallState::call_state_failed;
      response_.response   = "";
    }
  }

  void CServiceClientImpl::fromStruct(const Service::Response& response_struct_, eCAL::SServiceResponse& response_)
  {
    const auto& response_header = response_struct_.header;
    response_.host_name    = response_header.hname;
    response_.service_name = response_header.sname;
    response_.service_id   = response_header.sid;
    response_.method_name  = response_header.mname;
    response_.error_msg    = response_header.error;
    response_.ret_state    = static_cast<int>(response_struct_.ret_state);
    switch (response_header.state)
    {
    case Service::eMethodCallState::executed:
      response_.call_state = call_state_executed;
      break;
    case Service::eMethodCallState::failed:
      response_.call_state = call_state_failed;
      break;
    default:
      break;
    }
    response_.response = std::string(response_struct_.response.data(), response_struct_.response.size());
  }

  Registration::Sample CServiceClientImpl::GetRegistrationSample()
  {
    Registration::Sample ecal_reg_sample;
    ecal_reg_sample.cmd_type = bct_reg_client;

    auto& service_client = ecal_reg_sample.client;
    service_client.version = m_client_version;
    service_client.hname   = Process::GetHostName();
    service_client.pname   = Process::GetProcessName();
    service_client.uname   = Process::GetUnitName();
    service_client.pid     = Process::GetProcessID();
    service_client.sname   = m_service_name;
    service_client.sid     = m_service_id;

    {
      const std::lock_guard<std::mutex> lock(m_method_sync);

      for (const auto& method_information_pair : m_method_information_map)
      {
        const auto& method_name = method_information_pair.first;
        const auto& method_information = method_information_pair.second;

        Service::Method method;
        method.mname      = method_name;
        method.req_type   = method_information.request_type.name;
        method.req_desc   = method_information.request_type.descriptor;
        method.resp_type  = method_information.response_type.name;
        method.resp_desc  = method_information.response_type.descriptor;
        method.call_count = m_method_call_count_map.at(method_name);
        service_client.methods.push_back(method);
      }
    }

    return ecal_reg_sample;
  }

  Registration::Sample CServiceClientImpl::GetUnregistrationSample()
  {
    Registration::Sample ecal_reg_sample;
    ecal_reg_sample.cmd_type = bct_unreg_client;

    auto& service_client = ecal_reg_sample.client;
    service_client.hname   = Process::GetHostName();
    service_client.pname   = Process::GetProcessName();
    service_client.uname   = Process::GetUnitName();
    service_client.pid     = Process::GetProcessID();
    service_client.sname   = m_service_name;
    service_client.sid     = m_service_id;
    service_client.version = m_client_version;

    return ecal_reg_sample;
  }

  void CServiceClientImpl::Register()
  {
    if (!m_created)             return;
    if (m_service_name.empty()) return;

    // register entity
    if (g_registration_provider() != nullptr) g_registration_provider()->RegisterSample(GetRegistrationSample());
  }

  void CServiceClientImpl::Unregister()
  {
    if (m_service_name.empty()) return;

    // unregister entity
    if (g_registration_provider() != nullptr) g_registration_provider()->UnregisterSample(GetUnregistrationSample());
  }

  void CServiceClientImpl::CheckForNewServices()
  {
    if (g_clientgate() == nullptr) return;

    // check for new services
    std::vector<SServiceAttr> const service_vec = g_clientgate()->GetServiceAttr(m_service_name);
    for (const auto& iter : service_vec)
    {
      std::lock_guard<std::mutex> const lock(m_client_map_sync);
      auto client = m_client_map.find(iter.key);
      if (client == m_client_map.end())
      {
        auto client_manager = eCAL::service::ServiceManager::instance()->get_client_manager();
        if (client_manager == nullptr || client_manager->is_stopped()) return;

        // Event callback (unused)
        const eCAL::service::ClientSession::EventCallbackT event_callback
                = [/*this, service_ = iter*/] // Using the this pointer here is extremely unsafe, as it actually forces us to manage the lifetime of this object. UPDATE: this class now inherits from shared_from_this, so when implementing this function, we can store a weak_ptr to this class.
                  (eCAL::service::ClientEventType /*event*/, const std::string& /*message*/) -> void
                  {
                    // I have no idea why, but for some reason the event callbacks of the actual connetions are not even used. The connect / disconnect callbacks are executed whenever a new connection is found, and not when the client has actually connected or disconnected. I am preserving the previous behavior.
                  };

        // Only connect via V0 protocol / V0 port, if V1 port is not available
        const auto protocol_version = (iter.tcp_port_v1 != 0 ? iter.version : 0);
        const auto port_to_use = (protocol_version == 0 ? iter.tcp_port_v0 : iter.tcp_port_v1);

        // Create the client and add it to the map
        const std::vector<std::pair<std::string, uint16_t>> endpoint_list
                  {
                    {iter.hname, port_to_use},
                    {iter.hname + ".local", port_to_use},   // TODO: Make this configurable from the ecal.yaml
                  };
        const auto new_client_session = client_manager->create_client(static_cast<uint8_t>(protocol_version), endpoint_list, event_callback);
        if (new_client_session)
          m_client_map[iter.key] = new_client_session;
      }
    }
  }

  void CServiceClientImpl::CheckForDisconnectedServices()
  {
    std::lock_guard<std::mutex> const lock(m_client_map_sync);
    for (auto& client : m_client_map)
    {
      if (client.second->get_state() == eCAL::service::State::FAILED)
      {
        std::string const service_key = client.first;

        // is the service still in the connecting map ?
        auto iter = m_connected_services_map.find(service_key);
        if (iter != m_connected_services_map.end())
        {
          // call disconnect event
          std::lock_guard<std::mutex> const lock_cb(m_event_callback_map_sync);
          auto e_iter = m_event_callback_map.find(client_event_disconnected);
          if (e_iter != m_event_callback_map.end())
          {
            SClientEventCallbackData sdata;
            sdata.type = client_event_disconnected;
            sdata.time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
            sdata.attr = iter->second;
            (e_iter->second)(m_service_name.c_str(), &sdata);
          }
          // remove service
          m_connected_services_map.erase(iter);
        }
      }
    }
  }

  void CServiceClientImpl::ErrorCallback(const std::string& method_name_, const std::string& error_message_)
  {
    std::lock_guard<std::mutex> const lock(m_response_callback_sync);
    if (m_response_callback)
    {
      SServiceResponse service_response;
      service_response.call_state  = call_state_failed;
      service_response.error_msg   = error_message_;
      service_response.ret_state   = 0;
      service_response.method_name = method_name_;
      service_response.response.clear();
      m_response_callback(service_response);
    }
  }

  void CServiceClientImpl::IncrementMethodCallCount(const std::string& method_name_)
  {
    const std::lock_guard<std::mutex> lock(m_method_sync);
    if (m_method_information_map.find(method_name_) == m_method_information_map.end())
    {
      m_method_information_map[method_name_] = SServiceMethodInformation();
      m_method_call_count_map[method_name_] = 0;
    }
    ++m_method_call_count_map.at(method_name_);
  }
}
