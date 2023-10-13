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

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <mutex>

#include <asio.hpp>

#include "protocol_layout.h"

namespace eCAL
{
  namespace service
  {
    namespace ProtocolV0
    {
      using ErrorCallbackT         = std::function<void(asio::error_code ec)>;
      using SendSuccessCallback    = std::function<void()>;
      using ReceiveSuccessCallback = std::function<void(const std::shared_ptr<eCAL::service::TcpHeaderV0>& header_buffer, const std::shared_ptr<std::string>& payload_buffer)>;

      void async_send_payload_with_header   (asio::ip::tcp::socket& socket, std::mutex& socket_mutex, const std::shared_ptr<const eCAL::service::TcpHeaderV1>& header_buffer, const std::shared_ptr<const std::string>& payload_buffer, const ErrorCallbackT& error_cb, const SendSuccessCallback& success_cb);
      void async_receive_payload_with_header(asio::ip::tcp::socket& socket, std::mutex& socket_mutex, const ErrorCallbackT& error_cb, const ReceiveSuccessCallback& success_cb);
    };
  }
}
