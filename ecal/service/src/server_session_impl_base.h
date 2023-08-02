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
#include <functional>

#include <string>
#include <sstream>

#ifdef _MSC_VER
  #pragma warning(push)
  #pragma warning(disable: 4834)
#endif

#include <asio.hpp>
#include <utility>

#ifdef _MSC_VER
  #pragma warning(pop)
#endif

#include <ecal/service/server_session_types.h>
#include <ecal/service/state.h>

namespace eCAL
{
  namespace service
  {
    class ServerSessionBase
    {
    /////////////////////////////////////
    // Custom types for API
    /////////////////////////////////////
    public:
      using ShutdownCallbackT  = std::function<void (const std::shared_ptr<ServerSessionBase>&)>;

    /////////////////////////////////////
    // Constructor, Destructor, Create
    /////////////////////////////////////
    public:
      // Delete copy / move constructor and assignment operator
      ServerSessionBase(const ServerSessionBase&)            = delete;                  // Copy construct
      ServerSessionBase(ServerSessionBase&&)                 = delete;                  // Move construct

      ServerSessionBase& operator=(const ServerSessionBase&) = delete;                  // Copy assign
      ServerSessionBase& operator=(ServerSessionBase&&)      = delete;                  // Move assign
      
      virtual ~ServerSessionBase() = default;

    protected:
      ServerSessionBase(const std::shared_ptr<asio::io_context>&         io_context
                      , const ServerServiceCallbackT&                    service_callback
                      , const std::shared_ptr<asio::io_context::strand>& service_callback_strand
                      , const ServerEventCallbackT&                      event_callback
                      , const ShutdownCallbackT&                         shutdown_callback)
        : io_context_             (io_context)
        , socket_                 (*io_context)
        , service_callback_       (service_callback)
        , service_callback_strand_(service_callback_strand)
        , event_callback_         (event_callback)
        , shutdown_callback_      (shutdown_callback)
      {}

    /////////////////////////////////////
    // Public API
    /////////////////////////////////////
    public:
      asio::ip::tcp::socket& socket() { return socket_; }
      virtual void start() = 0;
      virtual void stop() = 0;

      virtual eCAL::service::State get_state() const = 0;

    /////////////////////////////////////
    // Member variables
    /////////////////////////////////////
    protected:
      const std::shared_ptr<asio::io_context>         io_context_;
      asio::ip::tcp::socket                           socket_;
      mutable std::mutex                              socket_mutex_;

      const ServerServiceCallbackT                    service_callback_;
      const std::shared_ptr<asio::io_context::strand> service_callback_strand_;
      const ServerEventCallbackT                      event_callback_;
      const ShutdownCallbackT                         shutdown_callback_;
    };

    } // namespace service
} // namespace eCAL
