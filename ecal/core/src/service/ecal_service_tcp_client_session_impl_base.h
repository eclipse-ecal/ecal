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
#include "ecal_service_error.h"

#include "ecal_service_tcp_client_session_types.h"

namespace eCAL
{
  namespace service
  {
    class ClientSessionBase
    {
    /////////////////////////////////////
    // Custom types for API
    /////////////////////////////////////
    public:
      using EventCallbackT    = eCAL::service::ClientEventCallbackT;
      using ResponseCallbackT = eCAL::service::ClientResponseCallbackT;

    /////////////////////////////////////
    // Constructor, Destructor, Create
    /////////////////////////////////////
    protected:
      ClientSessionBase(asio::io_context& io_context_, const EventCallbackT& event_callback)
        : io_context_    (io_context_)
        , socket_        (io_context_)
        , event_callback_(event_callback)
      {}

    public:
      virtual ~ClientSessionBase() = default;

    /////////////////////////////////////
    // API
    /////////////////////////////////////
    public:
      virtual void async_call_service(const std::shared_ptr<std::string>& request, const ResponseCallbackT& response_callback) = 0;
      virtual void stop() = 0;

    /////////////////////////////////////
    // Member variables
    /////////////////////////////////////
    protected:
      asio::io_context&     io_context_;
      asio::ip::tcp::socket socket_;
      const EventCallbackT  event_callback_;
    };

  } // namespace service
} // namespace eCAL
