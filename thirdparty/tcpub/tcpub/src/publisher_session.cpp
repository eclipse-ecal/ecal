// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#include "publisher_session.h"

#include <iostream>

#include "tcp_header.h"
#include "portable_endian.h"

#include "protocol_handshake_message.h"

namespace tcpub
{
  //////////////////////////////////////////////
  /// Constructor & Destructor
  //////////////////////////////////////////////
  
  PublisherSession::PublisherSession(const std::shared_ptr<asio::io_service>&                               io_service
                                     , const std::function<void(const std::shared_ptr<PublisherSession>&)>& session_closed_handler
                                     , const tcpub::logger::logger_t&                                  log_function)
    : io_service_             (io_service)
    , state_                  (State::NotStarted)
    , session_closed_handler_ (session_closed_handler)
    , log_                    (log_function)
    , data_socket_            (*io_service_)
    , data_strand_            (*io_service_)
    , sending_in_progress_    (false)
  {
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    log_(logger::LogLevel::DebugVerbose, "PublisherSession " + endpointToString() + ": Created.");
#endif
  }

  PublisherSession::~PublisherSession()
  {
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    std::stringstream ss;
    ss << std::this_thread::get_id();
    std::string thread_id = ss.str();
    log_(logger::LogLevel::DebugVerbose, "PublisherSession " + endpointToString() + ": Deleting from thread " + thread_id + "...");
#endif

#if (TCPUB_LOG_DEBUG_ENABLED)
    log_(logger::LogLevel::Debug, "PublisherSession " + endpointToString() + ": Deleted.");
#endif
  }

  //////////////////////////////////////////////
  /// Start & Stop
  //////////////////////////////////////////////
  
  void PublisherSession::start()
  {
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    log_(logger::LogLevel::DebugVerbose, "PublisherSession " + endpointToString() + ": Setting tcp::no_delay option.");
#endif
    // Disable Nagle's algorithm. Nagles Algorithm will otherwise cause the
    // Socket to wait for more data, if it encounters a frame that can still
    // fit more data. Obviously, this is an awfull default behaviour, if we
    // want to transmit our data in a timely fashion.
    {
      asio::error_code ec;
      data_socket_.set_option(asio::ip::tcp::no_delay(true), ec);
      if (ec) log_(logger::LogLevel::Warning, "PublisherSession " + endpointToString() + ": Failed setting tcp::no_delay option. The performance may suffer.");
    }

    state_ = State::Handshaking;

    receiveTcpPacket();
  }

  void PublisherSession::cancel()
  {
    sessionClosedHandler();
  }

  void PublisherSession::sessionClosedHandler()
  {
    // Check if this session has already been canceled while at the same time
    // setting it to the CANCELED. This ensures, that the handler will only be
    // run once.
    State previous_state = (state_.exchange(State::Canceled)); 
    if (previous_state == State::Canceled)
      return;

#if (TCPUB_LOG_DEBUG_ENABLED)
    log_(logger::LogLevel::Debug, "PublisherSession " + endpointToString() + ": Closing session.");
#endif

    {
      asio::error_code ec;
      data_socket_.close(ec); // Even if ec indicates an error, the socket is closed now (according to the documentation)
    }

    session_closed_handler_(shared_from_this()); // Run the completion handler
  }

  //////////////////////////////////////////////
  /// ProtocolHandshake
  //////////////////////////////////////////////
  void PublisherSession::receiveTcpPacket()
  {
    readHeaderLength();
  }

  void PublisherSession::readHeaderLength()
  {
    if (state_ == State::Canceled)
      return;

#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    log_(logger::LogLevel::DebugVerbose,  "PublisherSession " + endpointToString() + ": Waiting for data...");
#endif

    std::shared_ptr<TcpHeader> header = std::make_shared<TcpHeader>();

    asio::async_read(data_socket_
                    , asio::buffer(&(header->header_size), sizeof(header->header_size))
                    , asio::transfer_at_least(sizeof(header->header_size))
                    , data_strand_.wrap([me = shared_from_this(), header](asio::error_code ec, std::size_t /*length*/)
                                        {
                                          if (ec)
                                          {
                                            me->log_(logger::LogLevel::Error,  "PublisherSession " + me->endpointToString() + ": Error reading header length: " + ec.message());
                                            me->sessionClosedHandler();;
                                            return;
                                          }
                                          me->readHeaderContent(header);
                                        }));
  }

  void PublisherSession::readHeaderContent(const std::shared_ptr<TcpHeader>& header)
  {
    if (state_ == State::Canceled)
      return;

    if (header->header_size < sizeof(header->header_size))
    {
      log_(logger::LogLevel::Error,  "PublisherSession " + endpointToString() + ": Received header length of " + std::to_string(header->header_size) + ", which is less than the minimal header size.");
      sessionClosedHandler();
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
                                      me->log_(logger::LogLevel::Error,  "PublisherSession " + me->endpointToString() + ": Error reading header content: " + ec.message());
                                      me->sessionClosedHandler();;
                                      return;
                                    }
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
                                    me->log_(logger::LogLevel::DebugVerbose
                                          ,  "PublisherSession " + me->endpointToString()
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

  void PublisherSession::discardDataBetweenHeaderAndPayload(const std::shared_ptr<TcpHeader>& header, uint16_t bytes_to_discard)
  {
    if (state_ == State::Canceled)
      return;

    // This vector is temporary and will be deleted right after we read the data into it
    std::vector<char> data_to_discard;
    data_to_discard.resize(bytes_to_discard);

#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    log_(logger::LogLevel::DebugVerbose,  "PublisherSession " + endpointToString() + ": Discarding " + std::to_string(bytes_to_discard) + " bytes after the header.");
#endif

    asio::async_read(data_socket_
              , asio::buffer(data_to_discard.data(), bytes_to_discard)
              , asio::transfer_at_least(bytes_to_discard)
              , data_strand_.wrap([me = shared_from_this(), header](asio::error_code ec, std::size_t /*length*/)
                                  {
                                    if (ec)
                                    {
                                      me->log_(logger::LogLevel::Error,  "PublisherSession " + me->endpointToString() + ": Error discarding bytes after header: " + ec.message());
                                      me->sessionClosedHandler();
                                      return;
                                    }
                                    me->readPayload(header);
                                  }));
  }

  void PublisherSession::readPayload(const std::shared_ptr<TcpHeader>& header)
  {
    if (state_ == State::Canceled)
      return;

    if (header->data_size == 0)
    {
#if (TCPUB_LOG_DEBUG_ENABLED)
      log_(logger::LogLevel::Debug,  "PublisherSession " + endpointToString() + ": Received data size of 0.");
#endif
      sessionClosedHandler();
      return;
    }

    // Create a buffer for the playload
    std::shared_ptr<std::vector<char>> data_buffer = std::make_shared<std::vector<char>>();
    data_buffer->resize(le64toh(header->data_size));

    asio::async_read(data_socket_
              , asio::buffer(data_buffer->data(), le64toh(header->data_size))
              , asio::transfer_at_least(le64toh(header->data_size))
              , data_strand_.wrap([me = shared_from_this(), header, data_buffer](asio::error_code ec, std::size_t /*length*/)
                                  {
                                    if (ec)
                                    {
                                      me->log_(logger::LogLevel::Error,  "PublisherSession " + me->endpointToString() + ": Error reading payload: " + ec.message());
                                      me->sessionClosedHandler();;
                                      return;
                                    }

                                    // Handle payload
                                    if (header->type == MessageContentType::ProtocolHandshake)
                                    {
                                      ProtocolHandshakeMessage handshake_message;
                                      size_t bytes_to_copy = std::min(data_buffer->size(), sizeof(ProtocolHandshakeMessage));
                                      std::memcpy(&handshake_message, data_buffer->data(), bytes_to_copy);
#if (TCPUB_LOG_DEBUG_ENABLED)
                                      me->log_(logger::LogLevel::Debug,  "PublisherSession " + me->endpointToString() + ": Received Handshake message. Maximum supported protocol version from subsriber: v" + std::to_string(handshake_message.protocol_version));
#endif
                                      me->sendProtocolHandshakeResponse();
                                    }
                                    else
                                    {
                                      me->log_(logger::LogLevel::Warning,  "PublisherSession " + me->endpointToString() + ": Received message is not a handshake message (Type is " + std::to_string(static_cast<uint8_t>(header->type)) + ").");
                                      me->sessionClosedHandler();
                                    }
                                  }));
  }

  void PublisherSession::sendProtocolHandshakeResponse()
  {
    if (state_ == State::Canceled)
      return;

#if (TCPUB_LOG_DEBUG_ENABLED)
    log_(logger::LogLevel::Debug,  "PublisherSession " + endpointToString() + ": Sending ProtocolHandshakeResponse.");
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

    // Send the buffer directly to the client
    sendBufferToClient(buffer);
    State old_state = state_.exchange(State::Running);
    if (old_state != State::Handshaking)
      state_ = old_state;
  }

  //////////////////////////////////////////////
  /// Send Data
  //////////////////////////////////////////////

  void PublisherSession::sendDataBuffer(const std::shared_ptr<std::vector<char>>& buffer)
  {
    if (state_ == State::Canceled)
      return;

#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    std::stringstream buffer_pointer_ss;
    buffer_pointer_ss << "0x" << std::hex << buffer.get();
    const std::string buffer_pointer_string = buffer_pointer_ss.str();
#endif

    {
      std::lock_guard<std::mutex> next_buffer_lock(next_buffer_mutex_);

      if ((state_ == State::Running) &&  !sending_in_progress_)
      {
        // If we are not sending a buffer at the moment, we can directly trigger sending the given buffer
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
        log_(logger::LogLevel::DebugVerbose, "PublisherSession " + endpointToString() + ": Trigger sending buffer " + buffer_pointer_string + ".");
#endif
        sending_in_progress_ = true;
        sendBufferToClient(buffer);
      }
      else
      {
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
        log_(logger::LogLevel::DebugVerbose, "PublisherSession " + endpointToString() + ": Saved buffer " + buffer_pointer_string + " as next buffer.");
#endif
        // Store the new buffer as next buffer
        next_buffer_to_send_             = buffer;
      }
    }
  }

  void PublisherSession::sendBufferToClient(const std::shared_ptr<std::vector<char>>& buffer)
  {
    if (state_ == State::Canceled)
      return;

    asio::async_write(data_socket_
                , asio::buffer(*buffer)
                , data_strand_.wrap(
                  [me = shared_from_this(), buffer](asio::error_code ec, std::size_t /*bytes_to_transfer*/)
                  {
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
                    std::stringstream buffer_pointer_ss;
                    buffer_pointer_ss << "0x" << std::hex << buffer.get();
                    const std::string buffer_pointer_string = buffer_pointer_ss.str();
#endif
                    if (ec)
                    {
                      me->log_(logger::LogLevel::Warning, "PublisherSession " + me->endpointToString() + ": Failed sending data: " + ec.message());
                      me->sessionClosedHandler();
                      return;
                    }

                    if (me->state_ == State::Canceled)
                      return;

                    {
                      std::lock_guard<std::mutex> next_buffer_lock(me->next_buffer_mutex_);

                      if (me->next_buffer_to_send_)
                      {
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
                        me->log_(logger::LogLevel::DebugVerbose, "PublisherSession " + me->endpointToString() + ": Successfully sent buffer " + buffer_pointer_string + ". Next buffer is available, trigger sending it.");
#endif
                        // We have a next buffer!

                        // Copy the next buffer to send from the member variable
                        // to a temporary variable. Then delete the member variable,
                        // so when adding a new buffer as next buffer, it is clear
                        // that we now have taken ownership of that buffer.
                        auto next_buffer_tmp             = me->next_buffer_to_send_;

                        me->next_buffer_to_send_         = nullptr;

                        // Send the next buffer to the client
                        me->sendBufferToClient(next_buffer_tmp);
                      }
                      else
                      {
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
                        me->log_(logger::LogLevel::DebugVerbose, "PublisherSession " + me->endpointToString() + ": Successfully sent buffer " + buffer_pointer_string + ". No next buffer available.");
#endif
                        me->sending_in_progress_ = false;
                      }
                    }
                  }
                ));
  }

  //////////////////////////////////////////////
  /// (Status-) getters
  //////////////////////////////////////////////

  asio::ip::tcp::socket& PublisherSession::getSocket()
  {
    return data_socket_;
  }

  std::string PublisherSession::localEndpointToString() const
  {
    asio::error_code ec;
    auto local_endpoint = data_socket_.local_endpoint(ec);
    if (!ec)
      return local_endpoint.address().to_string() + ":" + std::to_string(local_endpoint.port());
    else
      return "?";
  }

  std::string PublisherSession::remoteEndpointToString() const
  {
    asio::error_code ec;
    auto remote_endpoint = data_socket_.remote_endpoint(ec);
    if (!ec)
      return remote_endpoint.address().to_string() + ":" + std::to_string(remote_endpoint.port());
    else
      return "?";
  }

  std::string PublisherSession::endpointToString() const
  {
    return localEndpointToString() + "->" + remoteEndpointToString();
  }

}
