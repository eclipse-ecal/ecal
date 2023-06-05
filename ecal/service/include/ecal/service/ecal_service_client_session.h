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

#include <ecal/service/ecal_service_client_session_types.h>
#include "ecal_service_logger.h"

namespace eCAL
{
  namespace service
  {
    // Forward declarations
    class ClientSessionBase;

    // Class
    class ClientSession
    {
    //////////////////////////////////////////////
    // Internal types for better consistency
    //////////////////////////////////////////////
    public:
      using EventCallbackT    = ClientEventCallbackT;
      using ResponseCallbackT = ClientResponseCallbackT;

    //////////////////////////////////////////////
    // Constructor, Destructor, Create
    //////////////////////////////////////////////
    public:
      static std::shared_ptr<ClientSession> create(asio::io_context&      io_context
                                                  , std::uint8_t          protocol_version
                                                  , const std::string&    address
                                                  , std::uint16_t         port
                                                  , const EventCallbackT& event_callback
                                                  , const LoggerT&        logger = default_logger("Service Client"));

    protected:
      ClientSession(asio::io_context&       io_context
                    , std::uint8_t          protocol_version
                    , const std::string&    address
                    , std::uint16_t         port
                    , const EventCallbackT& event_callback
                    , const LoggerT&        logger);

      // Delete copy constructor and assignment operator
      ClientSession(const ClientSession&)            = delete;
      ClientSession& operator=(const ClientSession&) = delete;

      // Delete move constructor and assignment operator
      ClientSession(ClientSession&&)            = delete;
      ClientSession& operator=(ClientSession&&) = delete;

    public:
      ~ClientSession();

    //////////////////////////////////////////////
    // Public API
    //////////////////////////////////////////////
    public:
      void async_call_service(const std::shared_ptr<const std::string>& request, const ResponseCallbackT& response_callback);
      eCAL::service::Error call_service(const std::shared_ptr<const std::string>& request, std::shared_ptr<std::string>& response);

      // TODO: add API that allows status checking on the client

    //////////////////////////////////////////////
    // Member Variables
    //////////////////////////////////////////////
    private:
      std::shared_ptr<ClientSessionBase> impl_;
    };
  } // namespace service
} // namespace eCAL
