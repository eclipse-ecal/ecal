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
#include <mutex>

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

#include "ecal_service_logger.h"
#include "ecal_service_tcp_session_server.h"

namespace eCAL
{
  namespace service
  {
    class Server : public std::enable_shared_from_this<Server>
    {
    ///////////////////////////////////////////
    // Types for API
    ///////////////////////////////////////////
  
    public:
      using ServiceCallbackT = std::function<int(const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response)>; // TODO: Make the request a const string
      using EventCallbackT   = std::function<void(eCAL_Server_Event, const std::string&)>; // TODO: THis definition is now both in the server and the server session.

    ///////////////////////////////////////////
    // Constructor, Destructor, Create
    ///////////////////////////////////////////

    public:
      static std::shared_ptr<Server> create(asio::io_context&       io_context
                                                  , unsigned int            protocol_version
                                                  , std::uint16_t           port
                                                  , const ServiceCallbackT& service_callback
                                                  , const EventCallbackT&   event_callback
                                                  , const LoggerT&          logger = default_logger("Service Server"));

    protected:
      Server(asio::io_context&      io_context
                  , std::uint16_t           port
                  , const ServiceCallbackT& service_callback
                  , const EventCallbackT&   event_callback
                  , const LoggerT&          logger);

      Server(const Server&)            = delete;                  // Copy construct
      Server(Server&&)                 = delete;                  // Move construct

      Server& operator=(const Server&) = delete;                  // Copy assign
      Server& operator=(Server&&)      = delete;                  // Move assign
  
    public:
      ~Server();

    ///////////////////////////////////////////
    // API
    ///////////////////////////////////////////
  
    public:
      bool          is_connected()         const;
      int           get_connection_count() const;
      std::uint16_t get_port()             const;

    private:
      void start_accept(unsigned int version);
      //int  on_request  (const std::string& request, std::string& response); // TODO Remove
      //void on_event    (eCAL_Server_Event event, const std::string& message); // TODO Remove

    ///////////////////////////////////////////
    // Member Variables
    ///////////////////////////////////////////
  
    private:
      asio::io_context&             io_context_;
      asio::ip::tcp::acceptor       acceptor_;

      const ServiceCallbackT        service_callback_;
      const EventCallbackT          event_callback_;

      mutable std::mutex            session_list_mutex_;
      std::vector<std::weak_ptr<ServerSessionBase>> session_list_; // TODO: decide whether std::vector is a good idea, as I will have to delete from the middle, when a session is closed.

      const LoggerT                 logger_;
    };
  } // namespace service
} // namespace eCAL
