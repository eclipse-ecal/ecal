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

#include <ecal/service/ecal_service_server.h>

#include <algorithm>

#include "ecal_service_server_impl.h"

namespace eCAL
{
  namespace service
  {
    ///////////////////////////////////////////
    // Constructor, Destructor, Create
    ///////////////////////////////////////////
    std::shared_ptr<Server> Server::create(asio::io_context&        io_context
                                          , std::uint8_t            protocol_version
                                          , std::uint16_t           port
                                          , const ServiceCallbackT& service_callback
                                          , const EventCallbackT&   event_callback
                                          , const LoggerT&          logger)
    {
      return std::shared_ptr<Server>(new Server(io_context, protocol_version, port, service_callback, event_callback, logger));
    }

    Server::Server(asio::io_context&      io_context
                , std::uint8_t            protocol_version
                , std::uint16_t           port
                , const ServiceCallbackT& service_callback
                , const EventCallbackT&   event_callback
                , const LoggerT&          logger)
    {
      impl_ = ServerImpl::create(io_context, protocol_version, port, service_callback, event_callback, logger);
    }

    Server::~Server()
    {}

    ///////////////////////////////////////////
    // API
    ///////////////////////////////////////////
  
    bool          Server::is_connected()         const { return impl_->is_connected(); }
    int           Server::get_connection_count() const { return impl_->get_connection_count(); }
    std::uint16_t Server::get_port()             const { return impl_->get_port(); }

  } // namespace service
} // namespace eCAL
