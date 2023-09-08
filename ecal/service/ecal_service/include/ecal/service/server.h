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

#pragma once

#include <memory>

#include <asio.hpp>

#include "logger.h"
#include "server_session_types.h"

namespace eCAL
{
  namespace service
  {
    // Forward declaration
    class ServerImpl;

    class Server
    {
    //////////////////////////////////////////////
    // Internal types for better consistency
    //////////////////////////////////////////////
    public:
      using EventCallbackT   = ServerEventCallbackT;
      using ServiceCallbackT = ServerServiceCallbackT;
      using DeleteCallbackT  = std::function<void(Server*)>;

    ///////////////////////////////////////////
    // Constructor, Destructor, Create
    ///////////////////////////////////////////

    public:
      static std::shared_ptr<Server> create(const std::shared_ptr<asio::io_context>& io_context
                                          , std::uint8_t                             protocol_version
                                          , std::uint16_t                            port
                                          , const ServiceCallbackT&                  service_callback
                                          , bool                                     parallel_service_calls_enabled
                                          , const EventCallbackT&                    event_callback
                                          , const LoggerT&                           logger
                                          , const DeleteCallbackT&                   delete_callback);

      static std::shared_ptr<Server> create(const std::shared_ptr<asio::io_context>& io_context
                                          , std::uint8_t                             protocol_version
                                          , std::uint16_t                            port
                                          , const ServiceCallbackT&                  service_callback
                                          , bool                                     parallel_service_calls_enabled
                                          , const EventCallbackT&                    event_callback
                                          , const LoggerT&                           logger = default_logger("Service Server"));

      static std::shared_ptr<Server> create(const std::shared_ptr<asio::io_context>& io_context
                                          , std::uint8_t                             protocol_version
                                          , std::uint16_t                            port
                                          , const ServiceCallbackT&                  service_callback
                                          , bool                                     parallel_service_calls_enabled
                                          , const EventCallbackT&                    event_callback
                                          , const DeleteCallbackT&                   delete_callback);
    protected:
      Server(const std::shared_ptr<asio::io_context>& io_context
            , std::uint8_t                            protocol_version
            , std::uint16_t                           port
            , const ServiceCallbackT&                 service_callback
            , bool                                    parallel_service_calls_enabled
            , const EventCallbackT&                   event_callback
            , const LoggerT&                          logger);

    public:
      Server(const Server&)            = delete;                  // Copy construct
      Server(Server&&)                 = delete;                  // Move construct

      Server& operator=(const Server&) = delete;                  // Copy assign
      Server& operator=(Server&&)      = delete;                  // Move assign
  
      ~Server() = default;

    ///////////////////////////////////////////
    // API
    ///////////////////////////////////////////
  
    public:
      bool          is_connected()         const;
      int           get_connection_count() const;
      std::uint16_t get_port()             const;
      
      void          stop();

    ///////////////////////////////////////////
    // Member Variables
    ///////////////////////////////////////////
    private:
      std::shared_ptr<ServerImpl> impl_;
    };

  } // namespace service
} // namespace eCAL
