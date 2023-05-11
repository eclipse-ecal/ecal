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

namespace eCAL
{

  class CAsioServerSessionV0
    : public CAsioServerSession
    , public std::enable_shared_from_this<CAsioServerSessionV0>
  {

  ///////////////////////////////////////////////
  // Create, Constructor, Destructor
  ///////////////////////////////////////////////

  public:
    static std::shared_ptr<CAsioServerSessionV0> create(asio::io_context& io_context_, const RequestCallbackT& request_callback, const EventCallbackT& event_callback);

  protected:
    CAsioServerSessionV0(asio::io_context& io_context_, const RequestCallbackT& request_callback, const EventCallbackT& event_callback);

    // Copy
    CAsioServerSessionV0(const CAsioServerSessionV0&)            = delete;
    CAsioServerSessionV0& operator=(const CAsioServerSessionV0&) = delete;

    // Move
    CAsioServerSessionV0(CAsioServerSessionV0&&) noexcept            = delete;
    CAsioServerSessionV0& operator=(CAsioServerSessionV0&&) noexcept = delete;

  public:
    // Destructor
    ~CAsioServerSessionV0() override = default;

  ///////////////////////////////////////////////
  // Data receiving
  ///////////////////////////////////////////////
  public:
    void start() override;

  private:
    void handle_read(const asio::error_code& ec, size_t bytes_transferred);

    void handle_write(const asio::error_code& ec, std::size_t /*bytes_transferred*/);

    static std::vector<char> pack_write(const std::string& response);

  ///////////////////////////////////////////////
  // Member variables
  ///////////////////////////////////////////////
  private:
    std::string            header_;
    size_t                 header_request_size_ = 0;
    std::string            request_;
    std::string            response_;
    std::vector<char>      packed_response_;
    
    enum { max_length = 64 * 1024 };
    char                   data_[max_length]{};
  };

} // namespace eCAL
