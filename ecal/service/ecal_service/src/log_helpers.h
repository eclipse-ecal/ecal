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

#include <asio.hpp>

namespace eCAL
{
  namespace service
  {
    // Forward declarations
    inline std::string get_connection_info_string(const asio::ip::tcp::socket& socket);
    inline std::string endpoint_to_string(const asio::ip::tcp::endpoint& endpoint);

    inline std::string get_connection_info_string(const asio::ip::tcp::socket& socket)
    {
      std::string local_endpoint_string  = "???";
      std::string remote_endpoint_string = "???";

      // Form local endpoint string
      {
        asio::error_code ec;
        const auto endpoint = socket.local_endpoint(ec);
        if (!ec)
          local_endpoint_string = endpoint_to_string(endpoint);
      }

      // form remote endpoint string
      {
        asio::error_code ec;
        const auto endpoint = socket.remote_endpoint(ec);
        if (!ec)
          remote_endpoint_string = endpoint_to_string(endpoint);
      }

      return local_endpoint_string + " -> " + remote_endpoint_string;
    };

    inline std::string endpoint_to_string(const asio::ip::tcp::endpoint& endpoint)
    {
      asio::error_code ec;
      const std::string address_string = endpoint.address().to_string(ec);
      if (!ec)
        return address_string + ":" + std::to_string(endpoint.port());
      else
        return "???";
    }
  } // namespace service
}// namespace eCAL
