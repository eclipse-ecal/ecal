/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 * Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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
 * @brief  eCAL service client interface
**/

#include <ecal/service/client.h>
#include <ecal/ecal.h>

#include "ecal_clientgate.h"
#include "ecal_global_accessors.h"
#include "ecal_service_client_impl.h"

#include <string>

namespace eCAL
{
  CServiceClient::CServiceClient(const std::string& service_name_, const ServiceMethodInformationSetT& method_information_set_, const ClientEventCallbackT& event_callback_)
  {
    // create client implementation
    auto service_client_impl = CServiceClientImpl::CreateInstance(service_name_, method_information_set_, event_callback_);
    m_service_client_impl = service_client_impl;
    // register client
    auto clientgate = g_clientgate();
    if (clientgate) clientgate->Register(service_name_, service_client_impl);
  }

  CServiceClient::~CServiceClient()
  {
    auto service_client_impl = m_service_client_impl.lock();
    // could be already destroyed by move
    if (service_client_impl == nullptr) return;

    // unregister client
    auto clientgate = g_clientgate();
    if (clientgate) clientgate->Unregister(service_client_impl->GetServiceName(), service_client_impl);
  }

  CServiceClient::CServiceClient(CServiceClient&& rhs) noexcept
    : m_service_client_impl(std::move(rhs.m_service_client_impl))
  {
  }

  CServiceClient& CServiceClient::operator=(CServiceClient&& rhs) noexcept
  {
    if (this != &rhs)
    {
      m_service_client_impl = std::move(rhs.m_service_client_impl);
    }
    return *this;
  }

  std::vector<CClientInstance> CServiceClient::GetClientInstances() const
  {
    auto service_client_impl = m_service_client_impl.lock();
    std::vector<CClientInstance> instances;
    if (service_client_impl) 
    {
      auto entity_ids = service_client_impl->GetServiceIDs();
      instances.reserve(entity_ids.size());
      for (const auto& entity_id : entity_ids)
      {
        instances.emplace_back(entity_id, service_client_impl);
      }
    }
    return instances;
  }

  bool CServiceClient::CallWithResponse(const std::string& method_name_, const std::string& request_, ServiceResponseVecT& service_response_vec_, int timeout_) const
  {
    service_response_vec_.clear();
    service_response_vec_.reserve(GetClientInstances().size());

    // Create response callback that fills the service_response_vec_.
    std::mutex service_response_vec_mutex;

    const ResponseCallbackT response_callback_
          = [&service_response_vec_, &service_response_vec_mutex](const SServiceResponse& service_response_)->void
            {
              // Lock the mutex to ensure thread-safe access to the response vector.
              std::lock_guard<std::mutex> lock(service_response_vec_mutex);
              service_response_vec_.emplace_back(service_response_);
            };

    // Blocking-call with the callback.
    return CallWithCallback(method_name_, request_, response_callback_, timeout_);
  }

  bool CServiceClient::CallWithCallback(const std::string& method_name_, const std::string& request_, const ResponseCallbackT& response_callback_, int timeout_ms_) const
  {
    const bool uses_valid_timeout = (timeout_ms_ > 0);
    auto instances = GetClientInstances();

    // in case of no instance is connected we return false immediately
    if (instances.size() == 0)
    {
      return false;
    }

    // Create a boolean for returning. The documentation of this method states:
    // 
    // "True if all calls were successful and minimum one instance was connected, otherwise false."
    //
    // Thus, we start with true and set it to false, in case that one of the calls
    // fails.
    bool all_calls_successful = true;

    // Create a condition variable and a mutex to wait for the response
    // All variables are in shared pointers, as we need to pass them to the
    // callback function via the lambda capture. When the user uses the timeout,
    // this method may finish earlier than the service calls, so we need to make
    // sure the callbacks can still operate on those variables.
    const auto mutex                             = std::make_shared<std::mutex>();
    const auto condition_variable                = std::make_shared<std::condition_variable>();
    const auto expected_service_call_count       = std::make_shared<int>(0);
    const auto finished_service_call_count       = std::make_shared<int>(0);
    const auto finished_response_callbacks_count = std::make_shared<int>(0);
    const auto timeout_has_been_reached          = std::make_shared<bool>(false);

    ///////////////////////////////////
    // 1/2) Start service calls async
    ///////////////////////////////////
    
    using timeout_response_info_t = std::pair<bool, ::eCAL::SServiceResponse>;
    std::vector<std::shared_ptr<timeout_response_info_t>> timeouted_calls_vector;
    
    if (uses_valid_timeout)
    {
      // Reserve enough space
      timeouted_calls_vector.reserve(instances.size());
    }

    // Iterate over all service instances and call each of them.
    // Each successfull call will increment the finished_service_call_count.
    // By comparing that with the expected_service_call_count we later know
    // Whether all service calls have been completed.
    for (auto& instance : instances)
    {
      std::shared_ptr<timeout_response_info_t> timeout_response_info_ptr;
      if (uses_valid_timeout)
      {
        // Prepare timeout response info, that will be filled in case of a timeout
        timeout_response_info_ptr = std::make_shared<timeout_response_info_t>(false, ::eCAL::SServiceResponse());

        // Add an element to the timeouted_calls_vector, so that we later know
        // which call exactly has timeouted.
        timeout_response_info_ptr->first = true; // True means that the call has timeouted. This variable is set to false by the response callback, in case that the call returns in time.
        timeout_response_info_ptr->second.call_state                                 = eCallState::timeouted;
        timeout_response_info_ptr->second.server_id.service_id.entity_id             = instance.GetClientID().entity_id;
        timeout_response_info_ptr->second.server_id.service_id.process_id            = instance.GetClientID().process_id;
        timeout_response_info_ptr->second.server_id.service_id.host_name             = instance.GetClientID().host_name;
        timeout_response_info_ptr->second.server_id.service_name                     = GetServiceName();
        timeout_response_info_ptr->second.service_method_information.method_name     = method_name_;
        //timeout_response_info_ptr->second.service_method_information.request_type  = ???
        //timeout_response_info_ptr->second.service_method_information.response_type = ???
        //timeout_response_info_ptr->second.ret_state                                = 0;
        //timeout_response_info_ptr->second.response                                 = "";
        timeout_response_info_ptr->second.error_msg                                  = "Timeout";
      }

      // Create a response callback, that will set the response and notify the condition variable
      eCAL::ResponseCallbackT notifying_response_callback
                = [mutex
                  , condition_variable
                  , finished_service_call_count
                  , timeout_has_been_reached
                  , timeout_response_info_ptr
                  , finished_response_callbacks_count
                  , response_callback_
#ifndef NDEBUG
                  , method_name_
                  , service_name = GetServiceName()
#endif // !NDEBUG
                  ]
                  (const eCAL::SServiceResponse& service_response) -> void
                  {
                    bool call_response_callback = false;

                    {
                      const std::lock_guard<std::mutex> lock(*mutex);

                      // only proceed if the timeout has not been reached, yet.
                      // Otherwise, we must not modify the result variables
                      // anymore or call the user callback.
                      if (!(*timeout_has_been_reached))
                      {
                        // This service call has not timeouted. Thus, we can call
                        // the user callback.
                        // In cases where the timeout is had been reached, the
                        // user callback must not be called anymore, as we have
                        // already done that from the surrunding code.

                        // Increment finished service call count and notify condition variable
                        (*finished_service_call_count)++;

                        // Mark this call as not-timeouted, so that we don't call
                        // the timeout callback later.
                        if (timeout_response_info_ptr)
                        {
                          timeout_response_info_ptr->first = false;
                        }

                        // We want to unlock the variable, before calling the user
                        // callback, as the callback may take some time to execute.
                        // Thus, while the mutex is locked, we store whether we
                        // want to execute it or not in an additional variable.
                        call_response_callback = true;

                        // Notify the condition variable that a service call has finished
                        condition_variable->notify_all();
                      }
#ifndef NDEBUG
                      else
                      {
                        eCAL::Logging::Log(eCAL::Logging::log_level_debug1, "CServiceClient::CallWithCallback: Received server response for "
                          + service_name + "." + method_name_ + "(), but service call has already timed out.");
                      }
#endif
                    }

                    if (call_response_callback)
                    {
                      // Call the user callback with the actual response
                      response_callback_(service_response);

                      // Lock the mutex again to increment the callback count
                      const std::lock_guard<std::mutex> lock(*mutex);
                      (*finished_response_callbacks_count)++;
                      condition_variable->notify_all();
                    }
                  };

      const bool call_success = instance.CallWithCallbackAsync(method_name_, request_, notifying_response_callback);

      if (call_success)
      {
        // If the call was successful, we increment the expected_service_call_count
        // In cases, where the call was NOT successful, the callback will never
        // be called, so we must not wait for it.
        const std::lock_guard<std::mutex> lock(*mutex);

        (*expected_service_call_count)++;

        if (uses_valid_timeout)
        {
          // Store the timeout information only, if the call was successful,
          // because only then the callback will be called and therefore only
          // then the timout information even have a chance to be modified by
          // that callback and set to non-timeouted.
          // Also, we need to keep timeouted_calls_vector.size() and
          // expected_service_call_count in sync.
          timeouted_calls_vector.push_back(timeout_response_info_ptr);
        }
      }
      else
      {
        all_calls_successful = false; // At least one call has failed to start, so the overall success must be false.
      }
    }

    //////////////////////////////////
    // 2/2) Wait for service call(s)
    //////////////////////////////////

    // Lock mutex, and wait for the condition variable to be notified
    {
      std::unique_lock<std::mutex> lock(*mutex);
      if (uses_valid_timeout)
      {
        // Emulate a timeout by using wait_for. After the timeout, we simply
        // forbid service that have not returned, yet to call the user_callback.
        // There is no way to cancel the service calls, so hiding them is the
        // only option.
        condition_variable->wait_for(lock
                                    , std::chrono::milliseconds(timeout_ms_)
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

      // Stop the callbacks from calling user callbacks and modifying their timeout
      // information. This is important in a timeout case, as we want to preserve
      // the state from the moment the timeout has occured. There is no way to
      // stop the service calls from succeeding after that, but we don't want
      // their values, any more.
      *timeout_has_been_reached = true;
      
      if (*expected_service_call_count != *finished_service_call_count)
      {
        all_calls_successful = false; // At least one call has timeouted, so the overall success must be false.
#ifndef NDEBUG
        eCAL::Logging::Log(eCAL::Logging::log_level_debug1, "CServiceClient::CallWithCallback: "
          + std::to_string(*expected_service_call_count - *finished_service_call_count) + " service call(s) did not return in time for "
          + GetServiceName() + "." + method_name_ + "()");
#endif
      }
    }

    if (uses_valid_timeout)
    {
      // If services have not returned, yet, their callback will never be called.
      // Thus, we call their callbacks from here and report a timeout.
      for (const auto& timeout_response_info_ptr : timeouted_calls_vector)
      {
        if (timeout_response_info_ptr->first)
        {
          response_callback_(timeout_response_info_ptr->second);
        }
      }
    }

    // Finally, we need to wait until all user callbacks have been executed.
    // We wait until all finished service calls (finished_service_call_count)
    // have also executed their user callbacks (finished_response_callbacks_count).
    // Non-finished service calls will not execute their user callbacks, so
    // we must not wait for those.
    {
      std::unique_lock<std::mutex> lock(*mutex);
      condition_variable->wait(lock, [&finished_service_call_count, &finished_response_callbacks_count]()
                                      {
                                        // Wait for all user callbacks to have been executed
                                        return *finished_service_call_count == *finished_response_callbacks_count;
                                      });
    }

    return all_calls_successful;
  }

  bool CServiceClient::CallWithCallbackAsync(const std::string& method_name_, const std::string& request_, const ResponseCallbackT& response_callback_) const
  {
    auto instances = GetClientInstances();

    // in case of no instance is connected we return fasle immediately
    if (instances.size() == 0)
      return false;

    bool return_state = true;
    for (auto& instance : instances)
    {
      return_state &= instance.CallWithCallbackAsync(method_name_, request_, response_callback_);
    }
    return return_state;
  }

  const std::string& CServiceClient::GetServiceName() const
  {
    auto service_client_impl = m_service_client_impl.lock();
    static const std::string empty_service_name{};
    if (service_client_impl) return service_client_impl->GetServiceName();
    return empty_service_name;
  }

  const SServiceId& CServiceClient::GetServiceId() const
  {
    auto service_client_impl = m_service_client_impl.lock();
    static const SServiceId empty_service_id{};
    if (service_client_impl) return service_client_impl->GetServiceId();
    return empty_service_id;
  }

  bool CServiceClient::IsConnected() const
  {
    const auto instances = GetClientInstances();
    for (const auto& instance : instances)
    {
      if (instance.IsConnected()) return true;
    }
    return false;
  }
}
