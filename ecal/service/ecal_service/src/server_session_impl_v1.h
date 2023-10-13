/* ========================= eCAL LICENSE ===== ============================
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

#include "server_session_impl_base.h"
#include <ecal/service/logger.h>
#include <ecal/service/server_session_types.h>

#include <ecal/service/state.h>

namespace eCAL
{
  namespace service
  {
    class ServerSessionV1 
      : public ServerSessionBase
      , public std::enable_shared_from_this<ServerSessionV1>
    {

    ///////////////////////////////////////////////
    // Create, Constructor, Destructor
    ///////////////////////////////////////////////

    public:
      static std::shared_ptr<ServerSessionV1> create(const std::shared_ptr<asio::io_context>&          io_context
                                                    , const ServerServiceCallbackT&                    service_callback
                                                    , const std::shared_ptr<asio::io_context::strand>& service_callback_strand
                                                    , const ServerEventCallbackT&                      event_callback
                                                    , const ShutdownCallbackT&                         shutdown_callback
                                                    , const LoggerT&                                   logger);

    protected:
      ServerSessionV1(const std::shared_ptr<asio::io_context>&         io_context
                    , const ServerServiceCallbackT&                    service_callback
                    , const std::shared_ptr<asio::io_context::strand>& service_callback_strand
                    , const ServerEventCallbackT&                      event_callback
                    , const ShutdownCallbackT&                         shutdown_callback
                    , const LoggerT&                                   logger);

    public:
      // Copy
      ServerSessionV1(const ServerSessionV1&)            = delete;
      ServerSessionV1& operator=(const ServerSessionV1&) = delete;

      // Move
      ServerSessionV1(ServerSessionV1&&) noexcept            = delete;
      ServerSessionV1& operator=(ServerSessionV1&&) noexcept = delete;

      // Destructor
      ~ServerSessionV1() override;

      ///////////////////////////////////////////////
      // Data receiving and sending
      ///////////////////////////////////////////////
    public:
      void start() override;
      void stop()  override;

      eCAL::service::State get_state() const override;

    private:
      void receive_handshake_request();
      void send_handshake_response();

      void receive_service_request();
      void send_service_response(const std::shared_ptr<std::string>& response_buffer);

    /////////////////////////////////////
    // Member variables
    /////////////////////////////////////
    private:
      static constexpr std::uint8_t MIN_SUPPORTED_PROTOCOL_VERSION = 1;
      static constexpr std::uint8_t MAX_SUPPORTED_PROTOCOL_VERSION = 1;

      std::atomic<State>      state_;
      std::uint8_t            accepted_protocol_version_;

      const LoggerT logger_;
    };
  }
}
