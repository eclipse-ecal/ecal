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

#include "asio_server_session.h"

#include <string>
#include <vector>
#include <memory>

namespace eCAL
{

  class CAsioServerSessionV1
    : public CAsioServerSession
    , public std::enable_shared_from_this<CAsioServerSessionV1>
  {

  ///////////////////////////////////////////////
  // Create, Constructor, Destructor
  ///////////////////////////////////////////////

  public:
    static std::shared_ptr<CAsioServerSessionV1> create(asio::io_context& io_context_, const RequestCallbackT& request_callback, const EventCallbackT& event_callback);

  protected:
    CAsioServerSessionV1(asio::io_context& io_context_, const RequestCallbackT& request_callback, const EventCallbackT& event_callback);

    // Copy
    CAsioServerSessionV1(const CAsioServerSessionV1&)            = delete;
    CAsioServerSessionV1& operator=(const CAsioServerSessionV1&) = delete;

    // Move
    CAsioServerSessionV1(CAsioServerSessionV1&&) noexcept            = delete;
    CAsioServerSessionV1& operator=(CAsioServerSessionV1&&) noexcept = delete;

  public:
    // Destructor
    ~CAsioServerSessionV1() = default;

  ///////////////////////////////////////////////
  // Data receiving and sending
  ///////////////////////////////////////////////
  public:
    void start() override;

  private:
    void read_request_header_start();
    void read_request_header_rest (const std::shared_ptr<std::vector<char>>& header_buffer, size_t bytes_already_read);
    void read_request_payload     (const std::shared_ptr<std::vector<char>>& header_buffer);

    void execute_callback         (const std::shared_ptr<std::string>& request_payload_buffer);

    void send_response_header     (const std::shared_ptr<std::string>& response);
    void send_response_payload    (const std::shared_ptr<std::string>& response);
  };

} // namespace eCAL
