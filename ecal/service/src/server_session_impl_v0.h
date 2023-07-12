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

#include "server_session_impl_base.h"
#include <ecal/service/logger.h>
#include <ecal/service/server_session_types.h>

#include <ecal/service/state.h>

namespace eCAL
{
  namespace service
  {
    class ServerSessionV0 
      : public ServerSessionBase
      , public std::enable_shared_from_this<ServerSessionV0>
    {

    ///////////////////////////////////////////////
    // Create, Constructor, Destructor
    ///////////////////////////////////////////////

    public:
      static std::shared_ptr<ServerSessionV0> create(const std::shared_ptr<asio::io_context>&          io_context
                                                    , const ServerServiceCallbackT&                    service_callback
                                                    , const std::shared_ptr<asio::io_context::strand>& service_callback_strand
                                                    , const ServerEventCallbackT&                      event_callback
                                                    , const ShutdownCallbackT&                         shutdown_callback
                                                    , const LoggerT&                                   logger);

    protected:
      ServerSessionV0(const std::shared_ptr<asio::io_context>&         io_context
                    , const ServerServiceCallbackT&                    service_callback
                    , const std::shared_ptr<asio::io_context::strand>& service_callback_strand
                    , const ServerEventCallbackT&                      event_callback
                    , const ShutdownCallbackT&                         shutdown_callback
                    , const LoggerT&                                   logger);

    public:
      // Copy
      ServerSessionV0(const ServerSessionV0&)            = delete;
      ServerSessionV0& operator=(const ServerSessionV0&) = delete;

      // Move
      ServerSessionV0(ServerSessionV0&&) noexcept            = delete;
      ServerSessionV0& operator=(ServerSessionV0&&) noexcept = delete;

      // Destructor
      ~ServerSessionV0() override;

      ///////////////////////////////////////////////
      // Data receiving and sending
      ///////////////////////////////////////////////
    public:
      void start() override;
      void stop()  override;

      eCAL::service::State get_state() const override;

    private:
      void handle_read(const asio::error_code& ec, size_t bytes_transferred, const std::shared_ptr<std::string>& request);

      void handle_write(const asio::error_code& ec, std::size_t /*bytes_transferred*/);

    /////////////////////////////////////
    // Member variables
    /////////////////////////////////////
    private:
      const LoggerT logger_;

      std::atomic<State>  state_;

      enum { max_length = 64 * 1024 };
      char                   data_[max_length]{};
    };

  }
}
