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

#include <condition_variable>
#include <ecal_service/client_session.h>

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <asio.hpp>

#include <ecal_service/error.h>
#include <ecal_service/logger.h>
#include <ecal_service/state.h>

#include "client_session_impl_v1.h"
#include "condition_variable_signaler.h"

namespace ecal_service
{
  std::shared_ptr<ClientSession> ClientSession::create(const std::shared_ptr<asio::io_context>&                  io_context
                                                     , std::uint8_t                                              protocol_version
                                                     , const std::vector<std::pair<std::string, std::uint16_t>>& server_list
                                                     , const EventCallbackT&                                     event_callback
                                                     , const LoggerT&                                            logger
                                                     , const DeleteCallbackT&                                    delete_callback)
  {
    auto deleter = [delete_callback](ClientSession* session)
    {
      delete_callback(session);
      delete session; // NOLINT(cppcoreguidelines-owning-memory)
    };

    return std::shared_ptr<ClientSession>(new ClientSession(io_context, protocol_version, server_list, event_callback, logger), deleter);
  }

  std::shared_ptr<ClientSession> ClientSession::create(const std::shared_ptr<asio::io_context>&                   io_context
                                                      , std::uint8_t                                              protocol_version
                                                      , const std::vector<std::pair<std::string, std::uint16_t>>& server_list
                                                      , const EventCallbackT&                                     event_callback
                                                      , const LoggerT&                                            logger)
  {
    return std::shared_ptr<ClientSession>(new ClientSession(io_context, protocol_version, server_list, event_callback, logger));
  }

  std::shared_ptr<ClientSession> ClientSession::create(const std::shared_ptr<asio::io_context>&                  io_context
                                                     , std::uint8_t                                              protocol_version
                                                     , const std::vector<std::pair<std::string, std::uint16_t>>& server_list
                                                     , const EventCallbackT&                                     event_callback
                                                     , const DeleteCallbackT&                                    delete_callback)
  {
    return ClientSession::create(io_context, protocol_version, server_list, event_callback, default_logger("Service Client"), delete_callback);
  }

  ClientSession::ClientSession(const std::shared_ptr<asio::io_context>&                   io_context
                              , std::uint8_t                                              /*protocol_version*/
                              , const std::vector<std::pair<std::string, std::uint16_t>>& server_list
                              , const EventCallbackT&                                     event_callback
                              , const LoggerT&                                            logger)
  {
    // we support v1 protocol only
    impl_ = ClientSessionV1::create(io_context, server_list, event_callback, logger);
  }

  ClientSession::~ClientSession()
  {
    impl_->stop();
  }

  //////////////////////////////////////////////
  // Public API
  //////////////////////////////////////////////
  bool ClientSession::async_call_service(const std::shared_ptr<const std::string>& request, const ResponseCallbackT& response_callback)
  {
    return impl_->async_call_service(request, response_callback);
  }

  ecal_service::Error ClientSession::call_service(const std::shared_ptr<const std::string>& request, std::shared_ptr<std::string>& response)
  {
    ecal_service::Error error(Error::GENERIC_ERROR);
      
    // Create a condition variable and a mutex to wait for the response
    std::mutex              mutex;
    std::condition_variable condition_variable;
    bool                    is_signaled = false;
    
    bool async_call_successful(false);

    {
      // Create a response callback, that will set the response and notify the condition variable
      const ResponseCallbackT response_callback
                = [&error, &response, signaler = std::make_shared<ConditionVariableSignaler>(condition_variable, mutex, is_signaled)]
                  (const ecal_service::Error& response_error, const std::shared_ptr<std::string>& response_)
                  {
                    response = response_;
                    error    = response_error;
                  };
      async_call_successful = async_call_service(request, response_callback); 
    }

    if(async_call_successful)
    {
      // Lock mutex, call service asynchronously and wait for the condition variable to be notified
      std::unique_lock<std::mutex> lock(mutex);
      condition_variable.wait(lock, [&is_signaled]() { return is_signaled; });
      return error;
    }
    else
    {
      // If the async service call has not been successfull, that means that
      // the user has stopped the client session. In that case, the request
      // hasn't even been enqueued, as there is no way to make sure that the
      // io_context is still being executed. In that case, we must not wait
      // for it to unblock the condition variable.
      // Thus, we just return an error
      return ecal_service::Error::ErrorCode::STOPPED_BY_USER;
    }
  }

  State                   ClientSession::get_state()                     const { return impl_->get_state(); }
  std::uint8_t            ClientSession::get_accepted_protocol_version() const { return impl_->get_accepted_protocol_version(); }
  int                     ClientSession::get_queue_size()                const { return impl_->get_queue_size(); }
  std::string             ClientSession::get_host()                      const { return impl_->get_host(); }
  asio::ip::tcp::endpoint ClientSession::get_remote_endpoint()           const { return impl_->get_remote_endpoint(); }
  std::uint16_t           ClientSession::get_port()                      const { return impl_->get_port(); }
  void                    ClientSession::stop()                                { impl_->stop(); }
} // namespace ecal_service
