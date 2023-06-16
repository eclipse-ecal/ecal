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

#include "client_session_impl_base.h"
#include <ecal/service/logger.h>

#include <deque>
#include <mutex>

namespace eCAL
{
  namespace service
  {
    class ClientSessionV0
      : public ClientSessionBase
      , public std::enable_shared_from_this<ClientSessionV0>
    {
    //////////////////////////////////////
    // Internal types
    //////////////////////////////////////
    private:
      struct ServiceCall
      {
        std::shared_ptr<const std::string> request;
        ResponseCallbackT                  response_cb;
      };

    /////////////////////////////////////
    // Constructor, Destructor, Create
    /////////////////////////////////////
    public:
      static std::shared_ptr<ClientSessionV0> create(asio::io_context&      io_context_
                                                    , const std::string&    address
                                                    , std::uint16_t         port
                                                    , const EventCallbackT& event_callback_
                                                    , const LoggerT&        logger_ = default_logger("Service Client V1"));

    protected:
      ClientSessionV0(asio::io_context& io_context_, const EventCallbackT& event_callback_, const LoggerT& logger);

    public:
      ~ClientSessionV0() override;

    //////////////////////////////////////
    // Connection establishement
    //////////////////////////////////////
    private:
      void resolve_endpoint(const std::string& address, std::uint16_t port);
      void connect_to_endpoint(const asio::ip::tcp::resolver::iterator& resolved_endpoints);

    //////////////////////////////////////
    // Service calls
    //////////////////////////////////////
    public:
      void async_call_service(const std::shared_ptr<const std::string>& request, const ResponseCallbackT& response_callback) override;

    private:
      void send_next_service_request(const std::shared_ptr<const std::string>& request, const ResponseCallbackT& response_cb);
      void receive_service_response(const ResponseCallbackT& response_cb);
    
    //////////////////////////////////////
    // Status API
    //////////////////////////////////////
    public:
      State        get_state()                     const override;
      std::uint8_t get_accepted_protocol_version() const override;
      int          get_queue_size()                const override;

    //////////////////////////////////////
    // Shutdown
    //////////////////////////////////////
    public:
      void peek_for_error();
      void handle_connection_loss_error(const std::string& message);
      void call_all_callbacks_with_error();
      void stop() override;

    //////////////////////////////////////
    // Member variables
    //////////////////////////////////////
    private:
      asio::io_context::strand  service_call_queue_strand_;
      asio::ip::tcp::resolver   resolver_;
      const LoggerT             logger_;

      mutable std::mutex        service_state_mutex_;
      State                     state_;
      std::deque<ServiceCall>   service_call_queue_;
      bool                      service_call_in_progress_;
    };
  }
}
