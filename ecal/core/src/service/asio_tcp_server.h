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

#include <functional>
#include <string>
#include <memory>
#include <atomic>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4834)
#endif
#include <asio.hpp>
#include <utility>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <ecal/cimpl/ecal_callback_cimpl.h>

#include "asio_tcp_server_session.h"
#include "asio_tcp_server_session_v0.h"
#include "asio_tcp_server_session_v1.h"

namespace eCAL
{
  class CAsioTcpServer : public std::enable_shared_from_this<eCAL::CAsioTcpServer>
  {
  ///////////////////////////////////////////
  // Types for API
  ///////////////////////////////////////////
  
  public:
    using RequestCallbackT = std::function<int(const std::string&, std::string&)>;
    using EventCallbackT   = std::function<void(eCAL_Server_Event, const std::string&)>;

  ///////////////////////////////////////////
  // Constructor, Destructor, Create
  ///////////////////////////////////////////

  public:
    static std::shared_ptr<CAsioTcpServer> create(asio::io_context&       io_context
                                                , unsigned int            protocol_version
                                                , std::uint16_t           port
                                                , const RequestCallbackT& request_callback
                                                , const EventCallbackT&   event_callback);

  protected:
    CAsioTcpServer(asio::io_context&      io_context
                , std::uint16_t           port
                , const RequestCallbackT& request_callback
                , const EventCallbackT&   event_callback);

    CAsioTcpServer(const CAsioTcpServer&)            = delete;                  // Copy construct
    CAsioTcpServer(CAsioTcpServer&&)                 = delete;                  // Move construct

    CAsioTcpServer& operator=(const CAsioTcpServer&) = delete;                  // Copy assign
    CAsioTcpServer& operator=(CAsioTcpServer&&)      = delete;                  // Move assign
  
  public:
    ~CAsioTcpServer();

  ///////////////////////////////////////////
  // API
  ///////////////////////////////////////////
  
  public:
    bool          is_connected() const;
    std::uint16_t get_port()     const;

  private:
    void start_accept(unsigned int version);
    int  on_request  (const std::string& request, std::string& response);
    void on_event    (eCAL_Server_Event event, const std::string& message);

  ///////////////////////////////////////////
  // Member Variables
  ///////////////////////////////////////////
  
  private:
    asio::io_context&             io_context_;
    asio::ip::tcp::acceptor       acceptor_;

    const RequestCallbackT        request_callback_;
    const EventCallbackT          event_callback_;

    std::atomic<int>              connection_count_;
  };

} // namespace eCAL
