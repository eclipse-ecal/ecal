// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#include "subscriber_session_impl.h"

#include "portable_endian.h"

#include "protocol_handshake_message.h"

namespace tcpub
{
  //////////////////////////////////////////////
  /// Constructor & Destructor
  //////////////////////////////////////////////
  
  SubscriberSession_Impl::SubscriberSession_Impl(const std::shared_ptr<asio::io_service>&                             io_service
                                                , const std::string&                                                  address
                                                , uint16_t                                                            port
                                                , int                                                                 max_reconnection_attempts
                                                , const std::function<std::shared_ptr<std::vector<char>>()>&          get_buffer_handler
                                                , const std::function<void(const std::shared_ptr<SubscriberSession_Impl>&)>& session_closed_handler
                                                , const tcpub::logger::logger_t&                                      log_function)
    : address_                (address)
    , port_                   (port)
    , resolver_               (*io_service)
    , max_reconnection_attempts_(max_reconnection_attempts)
    , retries_left_           (max_reconnection_attempts)
    , retry_timer_            (*io_service, std::chrono::seconds(1))
    , canceled_               (false)
    , data_socket_            (*io_service)
    , data_strand_            (*io_service)
    , get_buffer_handler_     (get_buffer_handler)
    , session_closed_handler_ (session_closed_handler)
    , log_                    (log_function)
  {}

  // Destructor
  SubscriberSession_Impl::~SubscriberSession_Impl()
  {
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    std::stringstream ss;
    ss << std::this_thread::get_id();
    std::string thread_id = ss.str();
    log_(logger::LogLevel::DebugVerbose, "SubscriberSession " + endpointToString() + ": Deleting from thread " + thread_id + "...");
#endif

    cancel();

#if (TCPUB_LOG_DEBUG_ENABLED)
    log_(logger::LogLevel::Debug, "SubscriberSession " + endpointToString() + ": Deleted.");
#endif
  }

  //////////////////////////////////////////////
  /// Connect to publisher
  //////////////////////////////////////////////
  
  void SubscriberSession_Impl::start()
  {
    if (canceled_) return;

    // Start resolving the endpoint given in the constructor
    resolveEndpoint();
  }

  void SubscriberSession_Impl::resolveEndpoint()
  {
    asio::ip::tcp::resolver::query query(address_, std::to_string(port_));

    if (canceled_)
    {
      connectionFailedHandler();
      return;
    }

    resolver_.async_resolve(query
                            , [me = shared_from_this()](asio::error_code ec, const asio::ip::tcp::resolver::iterator& resolved_endpoints)
                              {
                                if (ec)
                                {
                                  me->log_(logger::LogLevel::Warning, "SubscriberSession " + me->endpointToString() + ": Failed to resolve address: " + ec.message());
                                  me->connectionFailedHandler();
                                  return;
                                }
                                else
                                {
                                  me->connectToEndpoint(resolved_endpoints);
                                }
                              });
  }

  void SubscriberSession_Impl::connectToEndpoint(const asio::ip::tcp::resolver::iterator& resolved_endpoints)
  {
    if (canceled_)
    {
      connectionFailedHandler();
      return;
    }

    auto endpoint_to_connect_to = resolved_endpoints->endpoint(); // Default to first endpoint
    for (auto it = resolved_endpoints; it != asio::ip::tcp::resolver::iterator(); it++)
    {
      if (it->endpoint().address().is_loopback())
      {
        // If we find a loopback endpoint we use that one.
        endpoint_to_connect_to = it->endpoint();
        break;
      }
    }

    endpoint_ = endpoint_to_connect_to;

#if (TCPUB_LOG_DEBUG_ENABLED)
    log_(logger::LogLevel::Debug, "SubscriberSession " + endpointToString() + ": Trigger async connect to endpoint.");
#endif // 

    data_socket_.async_connect(endpoint_to_connect_to
                            , [me = shared_from_this()](asio::error_code ec)
                              {
                                if (ec)
                                {
                                  me->log_(logger::LogLevel::Warning, "SubscriberSession " + me->endpointToString() + ": Failed connecting to publisher: " + ec.message());
                                  me->connectionFailedHandler();
                                  return;
                                }
                                else
                                {
#if (TCPUB_LOG_DEBUG_ENABLED)
                                  me->log_(logger::LogLevel::Debug, "SubscriberSession " + me->endpointToString() + ": Successfully connected to publisher " + me->endpointToString());
#endif

#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
                                  me->log_(logger::LogLevel::DebugVerbose, "SubscriberSession " + me->endpointToString() + ": Setting tcp::no_delay option.");
#endif
                                  // Disable Nagle's algorithm. Nagles Algorithm will otherwise cause the
                                  // Socket to wait for more data, if it encounters a frame that can still
                                  // fit more data. Obviously, this is an awfull default behaviour, if we
                                  // want to transmit our data in a timely fashion.
                                  {
                                    asio::error_code nodelay_ec;
                                    me->data_socket_.set_option(asio::ip::tcp::no_delay(true), nodelay_ec);
                                    if (nodelay_ec) me->log_(logger::LogLevel::Warning, "SubscriberSession " + me->endpointToString() + ": Failed setting tcp::no_delay option. The performance may suffer.");
                                  }

                                  // Start reading a package by reading the header length. Everything will
                                  // unfold from there automatically.
                                  me->sendProtokolHandshakeRequest();
                                }
                              });
  }

  void SubscriberSession_Impl::sendProtokolHandshakeRequest()
  {
    if (canceled_)
    {
      connectionFailedHandler();
      return;
    }

#if (TCPUB_LOG_DEBUG_ENABLED)
    log_(logger::LogLevel::Debug,  "SubscriberSession " + endpointToString() + ": Sending ProtocolHandshakeRequest.");
#endif

    std::shared_ptr<std::vector<char>> buffer = std::make_shared<std::vector<char>>();
    buffer->resize(sizeof(TcpHeader) + sizeof(ProtocolHandshakeMessage));

    TcpHeader* header   = reinterpret_cast<TcpHeader*>(buffer->data());
    header->header_size = htole16(sizeof(TcpHeader));
    header->type        = MessageContentType::ProtocolHandshake;
    header->reserved    = 0;
    header->data_size   = htole64(sizeof(ProtocolHandshakeMessage));

    ProtocolHandshakeMessage* handshake_message = reinterpret_cast<ProtocolHandshakeMessage*>(&(buffer->operator[](sizeof(TcpHeader))));
    handshake_message->protocol_version         = 0; // At the moment, we only support Version 0. 

    asio::async_write(data_socket_
                , asio::buffer(*buffer)
                , data_strand_.wrap(
                  [me = shared_from_this(), buffer](asio::error_code ec, std::size_t /*bytes_to_transfer*/)
                  {
                    if (ec)
                    {
                      me->log_(logger::LogLevel::Warning, "SubscriberSession " + me->endpointToString() + ": Failed sending ProtocolHandshakeRequest: " + ec.message());
                      me->connectionFailedHandler();
                      return;
                    }
                    me->readHeaderLength();
                  }));
  }


  void SubscriberSession_Impl::connectionFailedHandler()
  {
    {
      asio::error_code ec;
      data_socket_.close(ec); // Even if ec indicates an error, the socket is closed now (according to the documentation)
    }

    if (!canceled_ && (retries_left_ < 0 || retries_left_ > 0))
    {
      // Decrement the number of retries we have left
      if (retries_left_ > 0)
        retries_left_--;

#if (TCPUB_LOG_DEBUG_ENABLED)
      log_(logger::LogLevel::Debug, "SubscriberSession " + endpointToString() + ": Waiting and retrying to connect");
#endif

      // Retry connection after a short time
      retry_timer_.async_wait([me = shared_from_this()](asio::error_code ec)
                              {
                                if (ec)
                                {
                                  me->log_(logger::LogLevel::Warning, "SubscriberSession " + me->endpointToString() + ": Waiting to reconnect failed: " + ec.message());
                                  me->session_closed_handler_(me);
                                  return;
                                }
                                me->resolveEndpoint();
                              });
    }
    else
    {
      session_closed_handler_(shared_from_this());
    }
  }
  
  /////////////////////////////////////////////
  // Data receiving
  /////////////////////////////////////////////
  
  void SubscriberSession_Impl::readHeaderLength()
  {
    if (canceled_)
    {
      connectionFailedHandler();
      return;
    }

#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    log_(logger::LogLevel::DebugVerbose,  "SubscriberSession " + endpointToString() + ": Waiting for data...");
#endif

    std::shared_ptr<TcpHeader> header = std::make_shared<TcpHeader>();

    asio::async_read(data_socket_
                     , asio::buffer(&(header->header_size), sizeof(header->header_size))
                     , asio::transfer_at_least(sizeof(header->header_size))
                     , data_strand_.wrap([me = shared_from_this(), header](asio::error_code ec, std::size_t /*length*/)
                                        {
                                          if (ec)
                                          {
                                            me->log_(logger::LogLevel::Error,  "SubscriberSession " + me->endpointToString() + ": Error reading header length: " + ec.message());
                                            me->connectionFailedHandler();;
                                            return;
                                          }
                                          me->readHeaderContent(header);
                                        }));
  }

  void SubscriberSession_Impl::readHeaderContent(const std::shared_ptr<TcpHeader>& header)
  {
    if (canceled_)
    {
      connectionFailedHandler();
      return;
    }

    if (header->header_size < sizeof(header->header_size))
    {
      log_(logger::LogLevel::Error,  "SubscriberSession " + endpointToString() + ": Received header length of " + std::to_string(header->header_size) + ", which is less than the minimal header size.");
      connectionFailedHandler();
      return;
    }

    const uint16_t remote_header_size = le16toh(header->header_size);
    const uint16_t my_header_size     = sizeof(*header);

    const uint16_t bytes_to_read_from_socket    = std::min(remote_header_size, my_header_size) - sizeof(header->header_size);
    const uint16_t bytes_to_discard_from_socket = (remote_header_size > my_header_size ? (remote_header_size - my_header_size) : 0);

    asio::async_read(data_socket_
                    , asio::buffer(&reinterpret_cast<char*>(header.get())[sizeof(header->header_size)], bytes_to_read_from_socket)
                    , asio::transfer_at_least(bytes_to_read_from_socket)
                    , data_strand_.wrap([me = shared_from_this(), header, bytes_to_discard_from_socket](asio::error_code ec, std::size_t /*length*/)
                                        {
                                          if (ec)
                                          {
                                            me->log_(logger::LogLevel::Error,  "SubscriberSession " + me->endpointToString() + ": Error reading header content: " + ec.message());
                                            me->connectionFailedHandler();;
                                            return;
                                          }
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
                                          me->log_(logger::LogLevel::DebugVerbose,  "SubscriberSession " + me->endpointToString()
                                            + ": Received header content: "
                                            + "data_size: "       + std::to_string(le64toh(header->data_size)));
#endif

                                          if (bytes_to_discard_from_socket > 0)
                                          {
                                            me->discardDataBetweenHeaderAndPayload(header, bytes_to_discard_from_socket);
                                          }
                                          else
                                          {
                                            me->readPayload(header);
                                          }
                                        }));
  }

  void SubscriberSession_Impl::discardDataBetweenHeaderAndPayload(const std::shared_ptr<TcpHeader>& header, uint16_t bytes_to_discard)
  {
    if (canceled_)
    {
      connectionFailedHandler();
      return;
    }

    std::vector<char> data_to_discard;
    data_to_discard.resize(bytes_to_discard);

#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    log_(logger::LogLevel::DebugVerbose,  "SubscriberSession " + endpointToString() + ": Discarding " + std::to_string(bytes_to_discard) + " bytes after the header.");
#endif

    asio::async_read(data_socket_
                    , asio::buffer(data_to_discard.data(), bytes_to_discard)
                    , asio::transfer_at_least(bytes_to_discard)
                    , data_strand_.wrap([me = shared_from_this(), header](asio::error_code ec, std::size_t /*length*/)
                                        {
                                          if (ec)
                                          {
                                            me->log_(logger::LogLevel::Error,  "SubscriberSession " + me->endpointToString() + ": Error discarding bytes after header: " + ec.message());
                                            me->connectionFailedHandler();;
                                            return;
                                          }
                                          me->readPayload(header);
                                        }));
  }

  void SubscriberSession_Impl::readPayload(const std::shared_ptr<TcpHeader>& header)
  {
    if (canceled_)
    {
      connectionFailedHandler();
      return;
    }

    if (header->data_size == 0)
    {
#if (TCPUB_LOG_DEBUG_ENABLED)
        log_(logger::LogLevel::Debug,  "SubscriberSession " + endpointToString() + ": Received data size of 0.");
#endif
      readHeaderLength();
      return;
    }

    // Get a buffer. This may be a used or a new one.
    std::shared_ptr<std::vector<char>> data_buffer = get_buffer_handler_();

    if (data_buffer->capacity() < le64toh(header->data_size))
    {
      // Reserve 10% extra memory
      data_buffer->reserve(static_cast<size_t>(le64toh(header->data_size) * 1.1));
    }

    // Resize the buffer to the required size
    data_buffer->resize(le64toh(header->data_size));

    asio::async_read(data_socket_
                , asio::buffer(data_buffer->data(), le64toh(header->data_size))
                , asio::transfer_at_least(le64toh(header->data_size))
                , data_strand_.wrap([me = shared_from_this(), header, data_buffer](asio::error_code ec, std::size_t /*length*/)
                                    {
                                      if (ec)
                                      {
                                        me->log_(logger::LogLevel::Error,  "SubscriberSession " + me->endpointToString() + ": Error reading payload: " + ec.message());
                                        me->connectionFailedHandler();;
                                        return;
                                      }

                                      // Reset the max amount of reconnects
                                      me->retries_left_ = me->max_reconnection_attempts_;

                                      if (header->type == MessageContentType::ProtocolHandshake)
                                      {
                                        ProtocolHandshakeMessage handshake_message;
                                        size_t bytes_to_copy = std::min(data_buffer->size(), sizeof(ProtocolHandshakeMessage));
                                        std::memcpy(&handshake_message, data_buffer->data(), bytes_to_copy);
#if (TCPUB_LOG_DEBUG_ENABLED)
                                        me->log_(logger::LogLevel::Debug,  "SubscriberSession " + me->endpointToString() + ": Received Handshake message. Using Protocol version v" + std::to_string(handshake_message.protocol_version));
#endif
                                        if (handshake_message.protocol_version > 0)
                                        {
                                          me->log_(logger::LogLevel::Error,  "SubscriberSession " + me->endpointToString() + ": Publisher set protocol version to v" + std::to_string(handshake_message.protocol_version) + ". This protocol is not supported.");
                                          me->connectionFailedHandler();
                                          return;
                                        }
                                      }
                                      else if (header->type == MessageContentType::RegularPayload)
                                      {
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
                                        me->log_(logger::LogLevel::DebugVerbose,  "SubscriberSession " + me->endpointToString() + ": Received message of type \"RegularPayload\"");
#endif
                                        // Call the callback first, ...
                                        me->data_strand_.post([me, data_buffer, header]()
                                                              {
                                                                if (me->canceled_)
                                                                {
                                                                  me->connectionFailedHandler();
                                                                  return;
                                                                }
                                                                me->synchronous_callback_(data_buffer, header);
                                                              });

                                      }
                                      else
                                      {
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
                                        me->log_(logger::LogLevel::DebugVerbose,  "SubscriberSession " + me->endpointToString() + ": Received message has unknow type: " + std::to_string(static_cast<int>(header->type)));
#endif
                                      }

                                      // ... then start reading the next message
                                      me->data_strand_.post([me]()
                                                            {
                                                              me->readHeaderLength();
                                                            });
                                    }));
  }

  //////////////////////////////////////////////
  /// Public API
  //////////////////////////////////////////////
  
  void SubscriberSession_Impl::setSynchronousCallback(const std::function<void(const std::shared_ptr<std::vector<char>>&, const std::shared_ptr<TcpHeader>&)>& callback)
  {
    if (canceled_) return;

    // We let asio set the callback for the following reasons:
    //   - We can protect the variable with the data_strand => If the callback is currently running, the new callback will be applied afterwards
    //   - We don't need an additional mutex, so a synchronous callback should actually be able to set another callback that gets activated once the current callback call ends
    //   - Reading the next message will start once the callback call is finished. Therefore, read and callback are synchronized and the callback calls don't start stacking up
    data_strand_.post([me = shared_from_this(), callback]()
                      {
                        me->synchronous_callback_ = callback;
                      });
  }

  std::string SubscriberSession_Impl::getAddress() const
  {
    return address_;
  }

  uint16_t SubscriberSession_Impl::getPort() const
  {
    return port_;
  }

  void SubscriberSession_Impl::cancel()
  {
    bool already_canceled = canceled_.exchange(true);

    if (already_canceled) return;

#if (TCPUB_LOG_DEBUG_ENABLED)
    log_(logger::LogLevel::Debug, "SubscriberSession " + endpointToString() + ": Cancelling...");
#endif
    
    {
      asio::error_code ec;
      data_socket_.close(ec);
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
      if (ec)
        log_(logger::LogLevel::DebugVerbose, "SubscriberSession " + endpointToString() + ": Failed closing socket: " + ec.message());
      else
        log_(logger::LogLevel::DebugVerbose, "SubscriberSession " + endpointToString() + ": Successfully closed socket.");
#endif
    }

    {
      asio::error_code ec;
      data_socket_.cancel(ec); // Even if ec indicates an error, the socket is closed now (according to the documentation)
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
      if (ec)
        log_(logger::LogLevel::DebugVerbose, "SubscriberSession " + endpointToString() + ": Failed cancelling socket: " + ec.message());
      else
        log_(logger::LogLevel::DebugVerbose, "SubscriberSession " + endpointToString() + ": Successfully canceled socket.");
#endif
    }

    {
      asio::error_code ec;
      retry_timer_.cancel(ec);
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
      if (ec)
        log_(logger::LogLevel::DebugVerbose, "SubscriberSession " + endpointToString() + ": Failed canceling retry timer: " + ec.message());
      else
        log_(logger::LogLevel::DebugVerbose, "SubscriberSession " + endpointToString() + ": Successfully canceled retry timer.");
#endif
    }

    resolver_.cancel();
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    log_(logger::LogLevel::DebugVerbose, "SubscriberSession " + endpointToString() + ": Successfully canceled resovler.");
#endif
  }

  bool SubscriberSession_Impl::isConnected() const
  {
    asio::error_code ec;
    data_socket_.remote_endpoint(ec);

    // If we can get the remote endpoint, we consider the socket as connected.
    // Otherwise it is not connected.

    if (ec)
      return false;
    else
      return true;
  }

  std::string SubscriberSession_Impl::remoteEndpointToString() const
  {
    return address_ + ":" + std::to_string(port_);
  }

  std::string SubscriberSession_Impl::localEndpointToString() const
  {
    asio::error_code ec;
    auto local_endpoint = data_socket_.local_endpoint(ec);
    if (ec)
      return "?";
    else
      return local_endpoint.address().to_string() + ":" + std::to_string(local_endpoint.port());
  }

  std::string SubscriberSession_Impl::endpointToString() const
  {
    return localEndpointToString() + "->" + remoteEndpointToString();
  }
}
