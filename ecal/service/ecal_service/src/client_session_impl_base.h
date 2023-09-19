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

#include <ecal/service/error.h>
#include <ecal/service/client_session_types.h>

#include <ecal/service/state.h>

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
      ClientSessionBase(const std::shared_ptr<asio::io_context>& io_context_, const EventCallbackT& event_callback)
        : io_context_    (io_context_)
        , socket_        (*io_context_)
        , event_callback_(event_callback)
      {}

    public:
      ClientSessionBase(const ClientSessionBase&)            = delete;
      ClientSessionBase(ClientSessionBase&&)                 = delete;
      ClientSessionBase& operator=(const ClientSessionBase&) = delete;
      ClientSessionBase& operator=(ClientSessionBase&&)      = delete;

      virtual ~ClientSessionBase() = default;

    /////////////////////////////////////
    // API
    /////////////////////////////////////
    public:
      virtual bool async_call_service(const std::shared_ptr<const std::string>& request, const ResponseCallbackT& response_callback) = 0;

      virtual std::string   get_address()                   const = 0;
      virtual std::uint16_t get_port()                      const = 0;

      virtual State         get_state()                     const = 0;
      virtual std::uint8_t  get_accepted_protocol_version() const = 0;
      virtual int           get_queue_size()                const = 0;

      virtual void stop() = 0;

    /////////////////////////////////////
    // Member variables
    /////////////////////////////////////
    protected:
      const std::shared_ptr<asio::io_context>  io_context_;
      asio::ip::tcp::socket                    socket_;
      mutable std::mutex                       socket_mutex_;
      const EventCallbackT                     event_callback_;

    };

  } // namespace service
} // namespace eCAL
