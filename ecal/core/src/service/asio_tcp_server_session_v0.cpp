///* ========================= eCAL LICENSE =================================
// *
// * Copyright (C) 2016 - 2019 Continental Corporation
// *
// * Licensed under the Apache License, Version 2.0 (the "License");
// * you may not use this file except in compliance with the License.
// * You may obtain a copy of the License at
// * 
// *      http://www.apache.org/licenses/LICENSE-2.0
// * 
// * Unless required by applicable law or agreed to in writing, software
// * distributed under the License is distributed on an "AS IS" BASIS,
// * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// * See the License for the specific language governing permissions and
// * limitations under the License.
// *
// * ========================= eCAL LICENSE =================================
//*/
//
//#include "asio_tcp_server_session_v0.h"
//
//#include "ecal_tcpheader.h"
//
//#include "asio_tcp_server_logging.h"
//#include <iostream>
//
//namespace eCAL
//{
//
//  ///////////////////////////////////////////////
//  // Create, Constructor, Destructor
//  ///////////////////////////////////////////////
//
//  std::shared_ptr<CAsioTcpServerSessionV0> CAsioTcpServerSessionV0::create(asio::io_context& io_context_, const RequestCallbackT& request_callback, const EventCallbackT& event_callback)
//  {
//    return std::shared_ptr<CAsioTcpServerSessionV0>(new CAsioTcpServerSessionV0(io_context_, request_callback, event_callback));
//  }
//
//  CAsioTcpServerSessionV0::CAsioTcpServerSessionV0(asio::io_context& io_context_, const RequestCallbackT& request_callback, const EventCallbackT& event_callback)
//    : CAsioTcpServerSession(io_context_, request_callback, event_callback)
//  {
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//    const auto message = get_log_string("DEBUG", "Created");
//    std::cout << message << std::endl;
//#endif
//  }
//
//  CAsioTcpServerSessionV0::~CAsioTcpServerSessionV0()
//  {
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//    const auto message = get_log_string("DEBUG", "Deleted");
//    std::cout << message << std::endl;
//#endif
//  }
//
//  ///////////////////////////////////////////////
//  // Data receiving
//  ///////////////////////////////////////////////
//  void CAsioTcpServerSessionV0::start()
//  {
//    const auto message = get_log_string("Connected");
//    event_callback_(eCAL_Server_Event::server_event_connected, message);
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED)
//    std::cout << message << std::endl;
//#endif
//
//    // Disable Nagle's algorithm. Nagles Algorithm will otherwise cause the
//    // Socket to wait for more data, if it encounters a frame that can still
//    // fit more data. Obviously, this is an awfull default behaviour, if we
//    // want to transmit our data in a timely fashion.
//    {
//      asio::error_code ec;
//      socket_.set_option(asio::ip::tcp::no_delay(true), ec);
//      if (ec)
//      {
//        std::cerr << get_log_string("WARNING", "Failed to set socket option 'no_delay' to 'true': " + ec.message()) << std::endl;;
//      }
//    }
//
//    socket_.async_read_some(asio::buffer(data_, max_length)
//                          , [me = shared_from_this()](asio::error_code ec, std::size_t bytes_read)
//                            {
//                               me->handle_read(ec, bytes_read);
//                            });
//                    
//  }
//
//  void CAsioTcpServerSessionV0::handle_read(const asio::error_code& ec, size_t bytes_transferred)
//  {
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//    std::cout << get_log_string("DEBUG", "Received " + std::to_string(bytes_transferred) + " bytes.") << std::endl;
//#endif
//
//    if (!ec)
//    {
//      if (request_callback_)
//      {
//        // collect request
//        request_ += std::string(data_, bytes_transferred);
//        // are there some more data on the socket ?
//        if (socket_.available() != 0u)
//        {
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//          std::cout << get_log_string("DEBUG", "More data is available on socket! Reading more data...") << std::endl;
//#endif
//          socket_.async_read_some(asio::buffer(data_, max_length)
//                                , [me = shared_from_this()](asio::error_code ec, std::size_t bytes_read)
//                                  {
//                                    me->handle_read(ec, bytes_read);
//                                  });
//        }
//        // no more data
//        else
//        {
//          // execute service callback
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//          std::cout << get_log_string("DEBUG", "Socket currently doesn't hold any more data.") << std::endl;
//          std::cout << get_log_string("DEBUG", "handle_read final request size: " + std::to_string(request_.size()) + ". Executing callback...") << std::endl;
//#endif
//          response_.clear();
//          request_callback_(request_, response_);
//          request_.clear();
//
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//          std::cout << get_log_string("DEBUG", "handle_read server callback executed. Reponse size: " + std::to_string(response_.size()) + ".") << std::endl;
//#endif
//    
//          // write response back
//          packed_response_.clear();
//          packed_response_ = pack_write(response_);
//
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//          const auto header = reinterpret_cast<const STcpHeader*>(packed_response_.data());
//          std::cout << get_log_string("DEBUG", "Writing response of " + std::to_string(packed_response_.size()) + " bytes (including header!)...") << std::endl;;
//          std::cout << get_log_string("DEBUG", "Header has the following content:") << std::endl;
//          std::cout << get_log_string("DEBUG", "  header->package_size_n = " + std::to_string(ntohl(header->package_size_n))) << std::endl;
//          std::cout << get_log_string("DEBUG", "  header->version        = " + std::to_string(header->version))               << std::endl;
//          std::cout << get_log_string("DEBUG", "  header->message_type   = " + std::to_string(static_cast<std::uint8_t>(header->message_type))) << std::endl;
//          std::cout << get_log_string("DEBUG", "  header->header_size_n  = " + std::to_string(ntohs(header->header_size_n)))  << std::endl;
//          // TODO: The reserved field is printed in network byte order, as Win7 compatibility of WinSocks2 does not define 64bit byte swap functions. I didn't want to introduce hacks or implement it myself, just for printing an empty reserved field.
//          std::cout << get_log_string("DEBUG", "  header->reserved       = " + std::to_string(header->reserved))              << std::endl;
//#endif
//
//          asio::async_write(socket_
//                          , asio::buffer(packed_response_.data(), packed_response_.size())
//                          , [me = shared_from_this()](asio::error_code ec, std::size_t bytes_written)
//                            {
//                              me->handle_write(ec, bytes_written);
//                            });
//        }
//      }
//    }
//    else
//    {
//      const auto message = get_log_string("Disconnected on read: " + ec.message());
//
//      // handle the disconnect
//      if (event_callback_)
//      {
//        event_callback_(server_event_disconnected, message);
//      }
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED)
//      std::cerr << message << std::endl;
//#endif
//    }
//  }
//
//  void CAsioTcpServerSessionV0::handle_write(const asio::error_code& ec, std::size_t bytes_transferred)
//  {
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//    if (ec)
//    {
//      const auto message = get_log_string("DEBUG", "Error encountered while sending response: " + ec.message());
//      std::cerr << message << std::endl;
//    }
//    else
//    {
//      const auto message = get_log_string("DEBUG", "Successfully wrote " + std::to_string(bytes_transferred) + " bytes.");
//      std::cout << message << std::endl;
//    }
//#endif
//
//    if (!ec)
//    {
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//      const auto message = get_log_string("DEBUG", "Waiting for data on socket...");
//      std::cerr << message << std::endl;
//#endif
//      socket_.async_read_some(asio::buffer(data_, max_length)
//                            , [me = shared_from_this()](asio::error_code ec, std::size_t bytes_read)
//                              {
//                                me->handle_read(ec, bytes_read);
//                              });
//    }
//    else
//    {
//      const auto message = get_log_string("Disconnected on write: " + ec.message());
//      // handle the disconnect
//      if (event_callback_)
//      {
//        event_callback_(server_event_disconnected, message);
//      }
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED)
//        std::cerr << message << std::endl;
//#endif
//    }
//  }
//
//  std::vector<char> CAsioTcpServerSessionV0::pack_write(const std::string& response)
//  {
//    // create header
//    eCAL::STcpHeader tcp_header;
//    // set up package size
//    const size_t psize = response.size();
//    tcp_header.package_size_n = htonl(static_cast<uint32_t>(psize));
//    tcp_header.header_size_n  = htons(static_cast<uint32_t>(sizeof(eCAL::STcpHeader)));
//    tcp_header.version        = 0;
//    // repack
//    std::vector<char> packed_response(sizeof(tcp_header) + psize);
//    memcpy(packed_response.data(), &tcp_header, sizeof(tcp_header));
//    memcpy(packed_response.data() + sizeof(tcp_header), response.data(), psize);
//    return packed_response;
//  }
//
//} // namespace eCAL