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
    class ClientSessionV1
      : public ClientSessionBase
      , public std::enable_shared_from_this<ClientSessionV1>
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
      static std::shared_ptr<ClientSessionV1> create(const std::shared_ptr<asio::io_context>& io_context
                                                    , const std::string&                      address
                                                    , std::uint16_t                           port
                                                    , const EventCallbackT&                   event_callback
                                                    , const LoggerT&                          logger_ = default_logger("Service Client V1"));

    protected:
      ClientSessionV1(const std::shared_ptr<asio::io_context>& io_context
                    , const std::string&                       address
                    , std::uint16_t                            port
                    , const EventCallbackT&                    event_callback
                    , const LoggerT&                           logger);

    public:
      // Delete copy / move constructor and assignment operator
      ClientSessionV1(const ClientSessionV1&)            = delete;                  // Copy construct
      ClientSessionV1(ClientSessionV1&&)                 = delete;                  // Move construct

      ClientSessionV1& operator=(const ClientSessionV1&) = delete;                  // Copy assign
      ClientSessionV1& operator=(ClientSessionV1&&)      = delete;                  // Move assign

      ~ClientSessionV1() override;

    //////////////////////////////////////
    // Connection establishement
    //////////////////////////////////////
    private:
      void resolve_endpoint();
      void connect_to_endpoint(const asio::ip::tcp::resolver::iterator& resolved_endpoints);

      void send_protocol_handshake_request();
      void receive_protocol_handshake_response();

    //////////////////////////////////////
    // Service calls
    //////////////////////////////////////
    public:
      bool async_call_service(const std::shared_ptr<const std::string>& request, const ResponseCallbackT& response_callback) override;

    private:
      void send_next_service_request(const std::shared_ptr<const std::string>& request, const ResponseCallbackT& response_cb);
      void receive_service_response(const ResponseCallbackT& response_cb);
    
    //////////////////////////////////////
    // Status API
    //////////////////////////////////////
    public:
      std::string   get_address()                   const override;
      std::uint16_t get_port()                      const override;
      State         get_state()                     const override;
      std::uint8_t  get_accepted_protocol_version() const override;
      int           get_queue_size()                const override;

    //////////////////////////////////////
    // Shutdown
    //////////////////////////////////////
    public:
      void peek_for_error();
      void handle_connection_loss_error(const std::string& message);
      void call_all_callbacks_with_error();

      /**
       * @brief Stop the client.
       * 
       * This will close the connection AND set the state to stopped. This
       * means, that no further async service calls will be accepted. From now
       * on, all async service calls will just return false and the callback
       * will not be called any more in order to give the io_thread the chance
       * of shutting down.
       */
      void stop() override;

    private:
      /**
       * @brief Closes the internal socket
       * 
       * This will close the connection without setting the state to stopped.
       * This means, that further async service calls will still be accepted,
       * but they will fail. The failure is still communicated by calling the
       * callback. For a proper shutdown, where the plan is to stop the
       * io_context thread as well, the stop() function must be used, or the
       * client must be destroyed.
       */
      void close_socket();


    //////////////////////////////////////
    // Member variables
    //////////////////////////////////////
    private:
      static constexpr std::uint8_t MIN_SUPPORTED_PROTOCOL_VERSION = 1;
      static constexpr std::uint8_t MAX_SUPPORTED_PROTOCOL_VERSION = 1;

      const std::string         address_;                                       //!< The original address that this client was created with.
      const std::uint16_t       port_;                                          //!< The original port that this client was created with.

      asio::io_context::strand  service_call_queue_strand_;
      asio::ip::tcp::resolver   resolver_;
      const LoggerT             logger_;

      std::atomic<std::uint8_t> accepted_protocol_version_;

      mutable std::mutex        service_state_mutex_;
      State                     state_;                     //!< The connection state of this client. Protected by service_state_mutex_.
      bool                      stopped_by_user_;           //!< Telling whether we actively stopped the client. Protected by service_state_mutex_. When set, the client will not accept any more async service calls.

      std::deque<ServiceCall>   service_call_queue_;
      bool                      service_call_in_progress_;
    };
  }
}
