#include "publisher_session.h"

#include <iostream>

#include "tcp_header.h"
#include "portable_endian.h"

namespace tcpub
{
  //////////////////////////////////////////////
  /// Constructor & Destructor
  //////////////////////////////////////////////
  
  PublisherSession::PublisherSession(const std::shared_ptr<asio::io_service>&                               io_service
                                     , const std::function<void(const std::shared_ptr<PublisherSession>&)>& session_closed_handler
                                     , const tcpub::logger::logger_t&                                  log_function)
    : io_service_             (io_service)
    , canceled_               (false)
    , session_closed_handler_ (session_closed_handler)
    , sending_in_progress_    (false)
    , log_                    (log_function)
    , data_socket_            (*io_service_)
    , data_strand_            (*io_service_)
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
    bool already_canceled = (canceled_.exchange(true)); 
    if (already_canceled)
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
  /// Send Data
  //////////////////////////////////////////////

  void PublisherSession::sendDataBuffer(const std::shared_ptr<std::vector<char>>& buffer)
  {
    if (canceled_) return;

#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    std::stringstream buffer_pointer_ss;
    buffer_pointer_ss << "0x" << std::hex << buffer.get();
    const std::string buffer_pointer_string = buffer_pointer_ss.str();
#endif

    {
      std::lock_guard<std::mutex> next_buffer_lock(next_buffer_mutex_);

      if (!sending_in_progress_)
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
    if (canceled_) return;

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

                    if (me->canceled_) return;

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

                        me->next_buffer_to_send_             = nullptr;

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
