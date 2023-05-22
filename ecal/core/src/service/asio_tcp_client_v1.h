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

#include "asio_tcp_client.h"

namespace eCAL
{
  class CAsioTcpClientV1 : public CAsioTcpClient, public std::enable_shared_from_this<eCAL::CAsioTcpClientV1>
  {
  /////////////////////////////////////
  // Constructor, Destructor, Create
  /////////////////////////////////////
  public:
    static std::shared_ptr<eCAL::CAsioTcpClientV1> create(asio::io_context& io_context_, const std::string& address, std::uint16_t port);

  protected:
    CAsioTcpClientV1(asio::io_context& io_context_, const std::string& address, std::uint16_t port);

  public:
    virtual ~CAsioTcpClientV1() override;

  /////////////////////////////////////
  // Log / message related methods
  /////////////////////////////////////
  public:
    std::string get_log_prefix() const override { return "CAsioTcpClientV1"; }

  //////////////////////////////////////
  // Implementation
  //////////////////////////////////////
  private:
    void resolve_endpoint();
    void connect_to_endpoint(const asio::ip::tcp::resolver::iterator& resolved_endpoints);
    void send_protocol_handshake();

    void read_header_start();
    void read_header_rest(const std::shared_ptr<std::vector<char>>& header_buffer, size_t bytes_already_read);
    void read_payload(const std::shared_ptr<std::vector<char>>& header_buffer);
    void handle_payload(const std::shared_ptr<std::vector<char>>& header_buffer, const std::shared_ptr<std::string>& payload);

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

    std::string             address_;
    std::uint16_t           port_;
    asio::ip::tcp::resolver resolver_;

    State                   state_;
    std::uint8_t            accepted_protocol_version_;
  };

} // namespace eCAL
