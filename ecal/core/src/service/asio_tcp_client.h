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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4834)
#endif
#include <asio.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "ecal/cimpl/ecal_callback_cimpl.h"

namespace eCAL
{
  class CAsioTcpClient
  {
  /////////////////////////////////////
  // Custom types for API
  /////////////////////////////////////
  public:
    using EventCallbackT = std::function<void (eCAL_Client_Event, const std::string &)>;

  /////////////////////////////////////
  // Constructor, Destructor, Create
  /////////////////////////////////////
  protected:
    CAsioTcpClient(asio::io_context& io_context_, EventCallbackT event_callback)
      : io_context_    (io_context_)
      , socket_        (io_context_)
      , event_callback_(event_callback)
    {}

  public:
    virtual ~CAsioTcpClient() = default;

  /////////////////////////////////////
  // Log / message related methods
  /////////////////////////////////////
  public:
    virtual std::string get_log_prefix() const = 0;

    inline std::string get_connection_info_string() const
    {
      std::string local_endpoint_string  = "???";
      std::string remote_endpoint_string = "???";
      
      // Form local endpoint string
      {
        asio::error_code ec;
        const auto endpoint = socket_.local_endpoint(ec);
        if (!ec)
          local_endpoint_string = endpoint_to_string(endpoint);
      }

      // form remote endpoint string
      {
        asio::error_code ec;
        const auto endpoint = socket_.remote_endpoint(ec);
        if (!ec)
          remote_endpoint_string = endpoint_to_string(endpoint);
      }

      return local_endpoint_string + " -> " + remote_endpoint_string;
    };

    static inline std::string endpoint_to_string(const asio::ip::tcp::endpoint& endpoint)
    {
      asio::error_code ec;
      const std::string address_string = endpoint.address().to_string(ec);
      if (!ec)
        return address_string + ":" + std::to_string(endpoint.port());
      else
        return "???";
    }

    inline std::string get_log_string(const std::string& message) const
    {
      return get_log_string("", message);
    }

    inline std::string get_log_string(const std::string& severity_string, const std::string& message) const
    {
      std::stringstream ss;
      ss << "[" << get_log_prefix() << "]";
      if (!severity_string.empty())
        ss << " [" << severity_string << "]";
      ss << " [" << get_connection_info_string() << " ]";
      ss << " " << message;

      return ss.str();
    }

  /////////////////////////////////////
  // Member variables
  /////////////////////////////////////
  protected:
    asio::io_context&     io_context_;
    asio::ip::tcp::socket socket_;
    EventCallbackT        event_callback_;
  };

} // namespace eCAL
