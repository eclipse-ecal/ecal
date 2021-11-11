// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

#include <asio.hpp>

#include "tcpub_logger_abstraction.h"
#include "tcp_header.h"

namespace tcpub
{
  class SubscriberSession_Impl : public std::enable_shared_from_this<SubscriberSession_Impl>
  {
  //////////////////////////////////////////////
  /// Constructor & Destructor
  //////////////////////////////////////////////
  public:
    SubscriberSession_Impl(const std::shared_ptr<asio::io_service>&                             io_service
                          , const std::string&                                                  address
                          , uint16_t                                                            port
                          , int                                                                 max_reconnection_attempts
                          , const std::function<std::shared_ptr<std::vector<char>>()>&          get_buffer_handler
                          , const std::function<void(const std::shared_ptr<SubscriberSession_Impl>&)>& session_closed_handler
                          , const tcpub::logger::logger_t&                                     log_function);


    // Copy
    SubscriberSession_Impl(const SubscriberSession_Impl&)            = delete;
    SubscriberSession_Impl& operator=(const SubscriberSession_Impl&) = delete;

    // Move
    SubscriberSession_Impl& operator=(SubscriberSession_Impl&&)      = delete;
    SubscriberSession_Impl(SubscriberSession_Impl&&)                 = delete;

    // Destructor
    ~SubscriberSession_Impl();

  //////////////////////////////////////////////
  /// Connect to publisher
  //////////////////////////////////////////////
  public:
    void start();

  private:
    void resolveEndpoint();
    void connectToEndpoint(const asio::ip::tcp::resolver::iterator& resolved_endpoints);

    void sendProtokolHandshakeRequest();

    void connectionFailedHandler();

  /////////////////////////////////////////////
  // Data receiving
  /////////////////////////////////////////////
  private:
    void readHeaderLength();
    void readHeaderContent(const std::shared_ptr<TcpHeader>& header);
    void discardDataBetweenHeaderAndPayload(const std::shared_ptr<TcpHeader>& header, uint16_t bytes_to_discard);
    void readPayload(const std::shared_ptr<TcpHeader>& header);

  //////////////////////////////////////////////
  /// Public API
  //////////////////////////////////////////////
  public:
    void        setSynchronousCallback(const std::function<void(const std::shared_ptr<std::vector<char>>&, const std::shared_ptr<TcpHeader>&)>& callback);

    std::string getAddress() const;
    uint16_t    getPort()    const;

    void        cancel();
    bool        isConnected() const;

    std::string remoteEndpointToString() const;
    std::string localEndpointToString() const;
    std::string endpointToString() const;

  //////////////////////////////////////////////
  /// Member variables
  //////////////////////////////////////////////
  private:
    // Endpoint and resolver given by / constructed by the constructor
    std::string             address_;
    uint16_t                port_;
    asio::ip::tcp::resolver resolver_;
    asio::ip::tcp::endpoint endpoint_;

    // Amount of retries left
    int                max_reconnection_attempts_;
    int                retries_left_;
    asio::steady_timer retry_timer_;
    std::atomic<bool>  canceled_;

    // TCP Socket & Queue (protected by the strand!)
    asio::ip::tcp::socket         data_socket_;
    asio::io_service::strand      data_strand_;   // Used for socket operations and the callback. This is done so messages don't queue up in the asio stack. We only start receiving new messages, after we have delivered the current one.

    // Handlers
    const std::function<std::shared_ptr<std::vector<char>>()>                                         get_buffer_handler_;         /// Function for retrieving / constructing an empty buffer
    const std::function<void(const std::shared_ptr<SubscriberSession_Impl>&)>                         session_closed_handler_;     /// Handler that is called when the session is closed
    std::function<void(const std::shared_ptr<std::vector<char>>&, const std::shared_ptr<TcpHeader>&)> synchronous_callback_;       /// [PROTECTED BY data_strand_!] Callback that is called when a complete message has been received. Executed in the asio constext, so this must be cheap!

    // Logger
    const tcpub::logger::logger_t log_;
  };
}
