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

#include "client_session_impl_v0.h"
#include "client_session_impl_base.h"

#include "protocol_v0.h"
#include "protocol_layout.h"
#include "log_helpers.h"
#include "log_defs.h"

#include <ecal/service/client_session_types.h>
#include <ecal/service/error.h>
#include <ecal/service/logger.h>
#include <ecal/service/state.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <asio.hpp>

namespace eCAL
{
  namespace service
  {
    /////////////////////////////////////
    // Constructor, Destructor, Create
    /////////////////////////////////////
    std::shared_ptr<ClientSessionV0> ClientSessionV0::create(const std::shared_ptr<asio::io_context>&                   io_context
                                                            , const std::vector<std::pair<std::string, std::uint16_t>>& server_list
                                                            , const EventCallbackT&                                     event_callback
                                                            , const LoggerT&                                            logger)
    {
      std::shared_ptr<ClientSessionV0> instance(new ClientSessionV0(io_context, server_list, event_callback, logger));

      // Throw exception, if the server list is empty
      if (server_list.empty())
      {
        throw std::invalid_argument("Server list must not be empty");
      }

      instance->resolve_endpoint(0);

      return instance;
    }

    ClientSessionV0::ClientSessionV0(const std::shared_ptr<asio::io_context>&                   io_context
                                    , const std::vector<std::pair<std::string, std::uint16_t>>& server_list
                                    , const EventCallbackT&                                     event_callback
                                    , const LoggerT&                                            logger)
      : ClientSessionBase(io_context, event_callback)
      , server_list_              (server_list)
      , service_call_queue_strand_(*io_context)
      , resolver_                 (*io_context)
      , logger_                   (logger)
      , state_                    (State::NOT_CONNECTED)
      , stopped_by_user_          (false)
      , service_call_in_progress_ (false)
    {
      ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger_, "Created");
    }

    ClientSessionV0::~ClientSessionV0()
    {
      ClientSessionV0::stop();
      ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger_, "Deleted");
    }

    //////////////////////////////////////
    // Connection establishement
    //////////////////////////////////////
    void ClientSessionV0::resolve_endpoint(size_t server_list_index)
    {
      ECAL_SERVICE_LOG_DEBUG(logger_, "Resolving endpoint [" + server_list_[server_list_index].first + ":" + std::to_string(server_list_[server_list_index].second) + "]...");

      const asio::ip::tcp::resolver::query query(server_list_[server_list_index].first, std::to_string(server_list_[server_list_index].second));

      resolver_.async_resolve(query
                            , service_call_queue_strand_.wrap([me = enable_shared_from_this<ClientSessionV0>::shared_from_this(), server_list_index_copy = server_list_index] // gcc 7 forces us to assign the server_list_index to a new variable
                              (asio::error_code ec, const asio::ip::tcp::resolver::iterator& resolved_endpoints)
                              {
                                if (ec)
                                {
#if ECAL_SERVICE_LOG_DEBUG_ENABLED
                                  {
                                    const std::string message = "Failed resolving endpoint [" + me->server_list_[server_list_index_copy].first + ":" + std::to_string(me->server_list_[server_list_index_copy].second) + "]: " + ec.message();
                                    ECAL_SERVICE_LOG_DEBUG(me->logger_, message);
                                  } 
#endif

                                  if (server_list_index_copy + 1 < me->server_list_.size())
                                  {
                                    // Try next possible endpoint
                                    me->resolve_endpoint(server_list_index_copy + 1);
                                  }
                                  else
                                  {
                                    std::string message = "Failed resolving any endpoint: ";
                                    for (size_t j = 0; j < me->server_list_.size(); ++j)
                                    {
                                      message += me->server_list_[j].first + ":" + std::to_string(me->server_list_[j].second);
                                      if (j + 1 < me->server_list_.size())
                                      {
                                        message += ", ";
                                      }
                                    }
                                    me->logger_(LogLevel::Error, message);
                                    me->handle_connection_loss_error(message);
                                  }
                                  return;
                                }
                                else
                                {
#if ECAL_SERVICE_LOG_DEBUG_VERBOSE_ENABLED
                                  // Verbose-debug log of all endpoints
                                  {
                                    std::string endpoints_str = "Resolved endpoints for " + me->server_list_[server_list_index_copy].first + ": ";
                                    for (auto it = resolved_endpoints; it != asio::ip::tcp::resolver::iterator(); ++it)
                                    {
                                      endpoints_str += endpoint_to_string(*it) + ", ";
                                    }
                                    ECAL_SERVICE_LOG_DEBUG_VERBOSE(me->logger_, endpoints_str);
                                  }
#endif //ECAL_SERVICE_LOG_DEBUG_VERBOSE_ENABLED
                                  me->connect_to_endpoint(resolved_endpoints, server_list_index_copy);
                                }
                              }));
    }

    void ClientSessionV0::connect_to_endpoint(const asio::ip::tcp::resolver::iterator& resolved_endpoints, size_t server_list_index)
    {
      // Convert the resolved_endpoints iterator to an endpoint sequence
      // (i.e. a vector of endpoints)
      auto endpoint_sequence = std::make_shared<std::vector<asio::ip::tcp::endpoint>>();
      for (auto it = resolved_endpoints; it != asio::ip::tcp::resolver::iterator(); ++it)
      {
        endpoint_sequence->push_back(*it);
      }

      const std::lock_guard<std::mutex> socket_lock(socket_mutex_);
      asio::async_connect(socket_
                        , *endpoint_sequence
                        , service_call_queue_strand_.wrap([me = shared_from_this(), endpoint_sequence, server_list_index](asio::error_code ec, const asio::ip::tcp::endpoint& endpoint)
                          {
                            (void)endpoint;
                            if (ec)
                            {
                              {
                                // Log an error
                                const std::string message = "Failed to connect to endpoint [" + me->chosen_endpoint_.first + ":" + std::to_string(me->chosen_endpoint_.second) + "]: " + ec.message();
                                me->logger_(LogLevel::Error, message);
                              }

                              // If there are more servers available, try the next one
                              if (server_list_index + 1 < me->server_list_.size())
                              {
                                me->resolve_endpoint(server_list_index + 1);
                              }
                              else
                              {
                                std::string message = "Failed to connect to any endpoint: ";
                                for (size_t j = 0; j < me->server_list_.size(); ++j)
                                {
                                  message += me->server_list_[j].first + ":" + std::to_string(me->server_list_[j].second);
                                  if (j + 1 < me->server_list_.size())
                                  {
                                    message += ", ";
                                  }
                                }
                                me->logger_(LogLevel::Error, message);
                                me->handle_connection_loss_error(message);
                              }
                              return;
                            }
                            else
                            {
                              ECAL_SERVICE_LOG_DEBUG(me->logger_, "Successfully connected to endpoint [" + endpoint.address().to_string() + ":" + std::to_string(endpoint.port()) + "]");

                              // Disable Nagle's algorithm. Nagles Algorithm will otherwise cause the
                              // Socket to wait for more data, if it encounters a frame that can still
                              // fit more data. Obviously, this is an awfull default behaviour, if we
                              // want to transmit our data in a timely fashion.
                              {
                                asio::error_code socket_option_ec;
                                {
                                  const std::lock_guard<std::mutex> socket_lock(me->socket_mutex_);
                                  me->socket_.set_option(asio::ip::tcp::no_delay(true), socket_option_ec); // NOLINT(bugprone-unused-return-value) -> We already get the value from the ec parameter
                                }
                                if (socket_option_ec)
                                {
                                  me->logger_(LogLevel::Warning, "[" + get_connection_info_string(me->socket_) + "] " + "Failed setting tcp::no_delay option: " + socket_option_ec.message());
                                }                                      
                              }

                              {
                                // Set the chosen endpoint
                                const std::lock_guard<std::mutex> chosen_endpoint_lock(me->chosen_endpoint_mutex_);
                                me->chosen_endpoint_ = me->server_list_[server_list_index];
                              }

                              const std::string message = "Connected to server. Using protocol version 0.";
                              me->logger_(LogLevel::Info, "[" + get_connection_info_string(me->socket_) + "] " + message);

                              {
                                const std::lock_guard<std::mutex> lock(me->service_state_mutex_);
                                me->state_ = State::CONNECTED;
                              }

                              // Call event callback
                              me->event_callback_(eCAL::service::ClientEventType::Connected, message);

                              // Start sending service requests, if there are any
                              {
                                const std::lock_guard<std::mutex> lock(me->service_state_mutex_);
                                if (!me->service_call_queue_.empty())
                                {
                                  // If there are service calls in the queue, we send the next one.
                                  me->service_call_in_progress_ = true;
                                  me->send_next_service_request(me->service_call_queue_.front().request, me->service_call_queue_.front().response_cb);
                                  me->service_call_queue_.pop_front();
                                }
                                else
                                {
                                  // If there are no more service calls to send, we go to error-peeking.
                                  // While error peeking we basically do nothing, except from non-destructively
                                  // reading 1 byte from the socket (i.e. without removing it from the socket).
                                  // This will cause asio / the OS to notify us, when the server closed the connection.

                                  me->service_call_in_progress_ = false;
                                  me->peek_for_error();
                                }
                              }
                            }
                          }));
    }

    //////////////////////////////////////
    // Service calls
    //////////////////////////////////////

    bool ClientSessionV0::async_call_service(const std::shared_ptr<const std::string>& request, const ResponseCallbackT& response_callback)
    {
      // Lock mutex for stopped_by_user_ variable
      const std::lock_guard<std::mutex> service_state_lock(service_state_mutex_);
      
      if (stopped_by_user_)
      {
        return false;
      }
      else
      {
        service_call_queue_strand_.post([me = shared_from_this(), request, response_callback]()
                              {
                                // Variable that enables us to unlock the mutex before actually calling the callback
                                bool call_response_callback_with_error(false);
                              
                                {
                                  const std::lock_guard<std::mutex> lock(me->service_state_mutex_);
                                  if (me->state_ != State::FAILED)
                                  {
                                    // If we are  not in failed state, let's check
                                    // whether we directly invoke the call of if we add it to the queue

                                    if (!me->service_call_in_progress_ && (me->state_ == State::CONNECTED))
                                    {
                                      // Directly call the the service, iff
                                      // 
                                      //  - There is no call in progress
                                      //  
                                      //  and
                                      // 
                                      //  - We are connected
                                      // 
                                      ECAL_SERVICE_LOG_DEBUG_VERBOSE(me->logger_, "[" + get_connection_info_string(me->socket_) + "] " + " No service call in progress. Directly starting next service call.");
                                      me->service_call_in_progress_ = true;
                                      me->send_next_service_request(request, response_callback);
                                    }
                                    else
                                    {
                                      // Add the call to the queue, iff:
                                      // 
                                      //  - A call is already in progress
                                      // 
                                      //  or
                                      // 
                                      //  - We are not connected, yet
                                      //
                                      ECAL_SERVICE_LOG_DEBUG_VERBOSE(me->logger_, "[" + get_connection_info_string(me->socket_) + "] " + "Queuing new service request");
                                      me->service_call_queue_.push_back(ServiceCall{request, response_callback});
                                    }
                                  }
                                  else
                                  {
                                    // If we are in FAILED state, we directly call the callback with an error.
                                    call_response_callback_with_error = true;
                                  }
                                }

                                if(call_response_callback_with_error)
                                {
                                  // If we are in FAILED state, we directly call the callback with an error.
                                  // The mutex is unlocked at this point. That is important, as we have no
                                  // influence on when the callback will return.
                                  ECAL_SERVICE_LOG_DEBUG_VERBOSE(me->logger_, "[" + get_connection_info_string(me->socket_) + "] " + " Client is in FAILED state. Calling callback with error.");
                                  response_callback(eCAL::service::Error::ErrorCode::CONNECTION_CLOSED, nullptr);
                                }
                              });
        return true;
      }
    }

    void ClientSessionV0::send_next_service_request(const std::shared_ptr<const std::string>& request, const ResponseCallbackT& response_cb)
    {
      ECAL_SERVICE_LOG_DEBUG(logger_, "[" + get_connection_info_string(socket_) + "] " + "Sending service request...");

      // V0 writes payload with no header

      const std::lock_guard<std::mutex> socket_lock(socket_mutex_);
      asio::async_write(socket_
                      , asio::buffer(*request)
                      , [me = shared_from_this(), request, response_cb](asio::error_code ec, std::size_t /*bytes_sent*/)
                        {
                          if (ec)
                          {
                            const std::string message = "Failed sending service request: " + ec.message();
                            me->logger_(LogLevel::Error, "[" + get_connection_info_string(me->socket_) + "] " + message);

                            // Call the callback with an error
                            response_cb(Error(Error::ErrorCode::CONNECTION_CLOSED, message), nullptr);

                            // Further handle the error, e.g. unwinding pending service calls and calling the event callback
                            me->handle_connection_loss_error(message);
                            return;
                          }
                          ECAL_SERVICE_LOG_DEBUG_VERBOSE(me->logger_, "[" + get_connection_info_string(me->socket_) + "] " + "Successfully sent service request.");
                          me->receive_service_response(response_cb);
                        });

    }

    void ClientSessionV0::receive_service_response(const ResponseCallbackT& response_cb)
    {
      ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger_, "[" + get_connection_info_string(socket_) + "] " + "Waiting for service response...");

      eCAL::service::ProtocolV0::async_receive_payload_with_header(socket_, socket_mutex_
                            , service_call_queue_strand_.wrap([me = shared_from_this(), response_cb](asio::error_code ec)
                              {
                                const std::string message = "Failed receiving service response: " + ec.message();
                                me->logger_(LogLevel::Error, "[" + get_connection_info_string(me->socket_) + "] " + message);

                                // Call the callback with an error
                                response_cb(Error(Error::ErrorCode::CONNECTION_CLOSED, message), nullptr);

                                // Further handle the error, e.g. unwinding pending service calls and calling the event callback
                                me->handle_connection_loss_error(message);
                              })
                            , service_call_queue_strand_.wrap([me = shared_from_this(), response_cb](const std::shared_ptr<eCAL::service::TcpHeaderV0>& /*header_buffer*/, const std::shared_ptr<std::string>& payload_buffer)
                              {
                                // The response is a Service response
                                ECAL_SERVICE_LOG_DEBUG(me->logger_, "[" + get_connection_info_string(me->socket_) + "] " + "Successfully received service response of " + std::to_string(payload_buffer->size()) + " bytes");

                                // Call the user's callback
                                response_cb(Error::OK, payload_buffer);

                                // Check if there are more items in the queue. If so, send the next request
                                // The mutex must be locket, as we access the queue.
                                {
                                  const std::lock_guard<std::mutex> lock(me->service_state_mutex_);

                                  if (!me->service_call_queue_.empty())
                                  {
                                    // If there are more items, continue calling the service
                                    ECAL_SERVICE_LOG_DEBUG_VERBOSE(me->logger_, "[" + get_connection_info_string(me->socket_) + "] " + " Service call queue contains " + std::to_string(me->service_call_queue_.size()) + " Entries. Starting next service call.");
                                    me->service_call_in_progress_ = true;
                                    me->send_next_service_request(me->service_call_queue_.front().request, me->service_call_queue_.front().response_cb);
                                    me->service_call_queue_.pop_front();
                                  }
                                  else
                                  {
                                    // If there are no more service calls to send, we go to error-peeking.
                                    // While error peeking we basically do nothing, except from non-destructively
                                    // reading 1 byte from the socket (i.e. without removing it from the socket).
                                    // This will cause asio / the OS to notify us, when the server closed the connection.

                                    ECAL_SERVICE_LOG_DEBUG_VERBOSE(me->logger_, "[" + get_connection_info_string(me->socket_) + "] " + " No further servcice calls.");
                                    me->service_call_in_progress_ = false;
                                    me->peek_for_error();
                                  }
                                }
                              }));


    }

    //////////////////////////////////////
    // Status API
    //////////////////////////////////////
    
    std::string ClientSessionV0::get_host() const
    {
      const std::lock_guard<std::mutex> chosen_endpoint_lock(chosen_endpoint_mutex_);
      return chosen_endpoint_.first;
    }

    std::uint16_t ClientSessionV0::get_port() const
    {
      const std::lock_guard<std::mutex> chosen_endpoint_lock(chosen_endpoint_mutex_);
      return chosen_endpoint_.second;
    }

    asio::ip::tcp::endpoint ClientSessionV0::get_remote_endpoint() const
    {
      // form remote endpoint string
      {
        asio::error_code ec;
        auto endpoint = socket_.remote_endpoint(ec);
        if (!ec)
          return endpoint;
        else
          return asio::ip::tcp::endpoint();
      }
    }

    State ClientSessionV0::get_state() const
    {
      const std::lock_guard<std::mutex> lock(service_state_mutex_);
      return state_;
    }

    std::uint8_t ClientSessionV0::get_accepted_protocol_version() const
    {
      return 0;
    }

    int ClientSessionV0::get_queue_size() const
    {
      const std::lock_guard<std::mutex> lock(service_state_mutex_);
      return static_cast<int>(service_call_queue_.size());
    }

    //////////////////////////////////////
    // Shutdown
    //////////////////////////////////////
    void ClientSessionV0::peek_for_error()
    {
      const std::shared_ptr<std::vector<char>> peek_buffer = std::make_shared<std::vector<char>>(1, '\0');

      const std::lock_guard<std::mutex> socket_lock(socket_mutex_);
      socket_.async_receive(asio::buffer(*peek_buffer)
                            , asio::socket_base::message_peek
                            , service_call_queue_strand_.wrap([me = shared_from_this(), peek_buffer](const asio::error_code& ec, std::size_t /*bytes_transferred*/) {
                              if (ec)
                              {
                                const std::string message = "Connection loss while idling: " + ec.message();
                                me->logger_(eCAL::service::LogLevel::Info, "[" + get_connection_info_string(me->socket_) + "] " + message);
                                me->handle_connection_loss_error("Connection loss while idling: " + ec.message());
                              }
                            }));
    }

    void ClientSessionV0::handle_connection_loss_error(const std::string& error_message)
    {
      bool call_event_callback (false); // Variable that enables us to unlock the mutex before we execute the event callback.

      // Close the socket, so all waiting async operations are actually woken
      // up and fail with an error code. If we wouldn't do that, at least on
      // Ubuntu only 1 waiting operations would wake up, while the others would
      // indefinitively continue to wait.
      // Having multiple async operations on the same socket actually does
      // happen, as we are peeking for errors whenever we don't send or
      // receive anything.
      close_socket();

      {
        const std::lock_guard<std::mutex> lock(service_state_mutex_);

        // cancel the connection loss handling, if we are already in FAILED state
        if (state_ == State::FAILED)
          return;

        if (state_ == State::CONNECTED)
        {
          // Event callback
          call_event_callback = true;
        }

        // Set the state to FAILED
        state_ = State::FAILED;

        // call all callbacks from the queue with an error
        if (!service_call_queue_.empty())
        {
          ECAL_SERVICE_LOG_DEBUG(logger_, "[" + get_connection_info_string(socket_) + "] " + "Calling " + std::to_string(service_call_queue_.size()) + " service callbacks with error");
          call_all_callbacks_with_error();
        }
      }

      if (call_event_callback)
      {
        event_callback_(eCAL::service::ClientEventType::Disconnected, error_message);
      }
    }

    void ClientSessionV0::call_all_callbacks_with_error()
    {
      service_call_queue_strand_.post([me = shared_from_this()]()
                                      {
                                        ServiceCall first_service_call;
                                        bool        more_service_calls(false);

                                        {
                                          // Lock the mutex and manipulate the queue. We want the mutex unlocked for the event callback call.
                                          const std::lock_guard<std::mutex> lock(me->service_state_mutex_);
                                          
                                          if (me->service_call_queue_.empty())
                                            return;

                                          first_service_call = std::move(me->service_call_queue_.front());
                                          me->service_call_queue_.pop_front();

                                          more_service_calls = (!me->service_call_queue_.empty());
                                        }

                                        // Execute the callback with an error
                                        first_service_call.response_cb(eCAL::service::Error::ErrorCode::CONNECTION_CLOSED, nullptr); // TODO: I should probably store the error that lead to this somewhere and tell the actual error.

                                        // If there are more sevice calls, call those with an error, as well
                                        if (more_service_calls)
                                          me->call_all_callbacks_with_error();
                                      });
    }

    void ClientSessionV0::stop()
    {
      // This is a function that gets used both by the API and by potentially
      // multiple failing async operations at once. 

      {
        // Set the stopped_by_user_ flag to true, so that the async operations stop enqueuing new service calls.
        const std::lock_guard<std::mutex> service_state_lock(service_state_mutex_);
        stopped_by_user_ = true;
      }

      {
        close_socket();
      }
    }

    void ClientSessionV0::close_socket()
    {
      // Close the socket, so all waiting async operations will fail with an
      // error code. This will also cause the client to call all pending
      // callbacks with an error.
      const std::lock_guard<std::mutex> socket_lock(socket_mutex_);

      if (socket_.is_open())
      {
        {
          asio::error_code ec;
          socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec); // NOLINT(bugprone-unused-return-value) -> we already get the value by the ec parameter
        }

        {
          asio::error_code ec;
          socket_.close(ec); // NOLINT(bugprone-unused-return-value) -> we already get the value by the ec parameter
        }
      }
    }


  }  // namespace service
} // namespace eCAL
