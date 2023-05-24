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

#include "ecal_service_tcp_session_client.h"
#include "ecal_service_logger.h"

namespace eCAL
{
  namespace service
  {
    class ClientSessionV1
      : public ClientSessionBase
      , public std::enable_shared_from_this<ClientSessionV1>
    {
    /////////////////////////////////////
    // Types for public API
    /////////////////////////////////////
    public:
      // TODO: This should probably also have an error code
      using ResponseCallbackT = std::function<void (const std::shared_ptr<std::string>&)>;

    /////////////////////////////////////
    // Constructor, Destructor, Create
    /////////////////////////////////////
    public:
      static std::shared_ptr<ClientSessionV1> create(asio::io_context& io_context_, const std::string& address, std::uint16_t port, const LoggerT& logger_ = default_logger("Service Client V1"));

    protected:
      ClientSessionV1(asio::io_context& io_context_, const LoggerT& logger);

      // TODO: Add again
    public:
      ~ClientSessionV1() override;

    /////////////////////////////////////
    // Log / message related methods
    /////////////////////////////////////
    public:
      std::string get_log_prefix() const override { return "ClientSessionV1"; }
      // TODO add again

    //////////////////////////////////////
    // Implementation
    //////////////////////////////////////
    private:
      void resolve_endpoint(const std::string& address, std::uint16_t port);
      void connect_to_endpoint(const asio::ip::tcp::resolver::iterator& resolved_endpoints);

      void send_protocol_handshake_request();
      void receive_protocol_handshake_response();

    public:
      void send_service_reqest(const std::shared_ptr<std::string>& request, const ResponseCallbackT& response_cb);

    private:
      void receive_service_response(const ResponseCallbackT& response_cb);

    //////////////////////////////////////
    // Member variables
    //////////////////////////////////////
    private:
      enum class State
      {
        NOT_CONNECTED,
        HANDSHAKE,
        CONNECTED,
        FAILED,
      };

      static constexpr std::uint8_t MIN_SUPPORTED_PROTOCOL_VERSION = 1;
      static constexpr std::uint8_t MAX_SUPPORTED_PROTOCOL_VERSION = 1;

      asio::ip::tcp::resolver resolver_;

      State                   state_;
      std::uint8_t            accepted_protocol_version_;

      const LoggerT           logger_;
    };
  }
}
