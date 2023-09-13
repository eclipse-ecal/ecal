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

#include <ecal/service/logger.h>
#include <ecal/service/server_session_types.h>

#include "server_session_impl_base.h"

namespace eCAL
{
  namespace service
  {
    class ServerImpl : public std::enable_shared_from_this<ServerImpl>
    {
    ///////////////////////////////////////////
    // Constructor, Destructor, Create
    ///////////////////////////////////////////

    public:
      static std::shared_ptr<ServerImpl> create(const std::shared_ptr<asio::io_context>& io_context
                                              , std::uint8_t                             protocol_version
                                              , std::uint16_t                            port
                                              , const ServerServiceCallbackT&            service_callback
                                              , bool                                     parallel_service_calls_enabled
                                              , const ServerEventCallbackT&              event_callback
                                              , const LoggerT&                           logger = default_logger("Service Server"));

    protected:
      ServerImpl(const std::shared_ptr<asio::io_context>& io_context
                , const ServerServiceCallbackT&           service_callback
                , bool                                    parallel_service_calls_enabled
                , const ServerEventCallbackT&             event_callback
                , const LoggerT&                          logger);

    public:
      ServerImpl(const ServerImpl&)            = delete;                  // Copy construct
      ServerImpl(ServerImpl&&)                 = delete;                  // Move construct

      ServerImpl& operator=(const ServerImpl&) = delete;                  // Copy assign
      ServerImpl& operator=(ServerImpl&&)      = delete;                  // Move assign
  
      ~ServerImpl();

    ///////////////////////////////////////////
    // API
    ///////////////////////////////////////////
  
    public:
      bool          is_connected()         const;
      int           get_connection_count() const;
      std::uint16_t get_port()             const;

      void          stop();

    private:
      void start_accept(std::uint8_t protocol_version, std::uint16_t port);
      void wait_for_next_client(std::uint8_t protocol_version);

    ///////////////////////////////////////////
    // Member Variables
    ///////////////////////////////////////////
  
    private:
      const std::shared_ptr<asio::io_context>         io_context_;
      asio::ip::tcp::acceptor                         acceptor_;
      mutable std::mutex                              acceptor_mutex_;                                //!< Mutex for stopping the server. The stop() function is both used externally (via API) and from within the server itself. Closing the acceptor is not thread-safe, so we need to protect it.

      const bool                                      parallel_service_calls_enabled_;
      const std::shared_ptr<asio::io_context::strand> service_callback_common_strand_;
      const ServerServiceCallbackT                    service_callback_;
      const ServerEventCallbackT                      event_callback_;

      mutable std::mutex                              session_list_mutex_;
      std::vector<std::weak_ptr<ServerSessionBase>>   session_list_;

      const LoggerT                                   logger_;

    };
  } // namespace service
} // namespace eCAL
