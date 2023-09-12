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

#include <memory>

#include <asio.hpp>

#include "logger.h"
#include "server_session_types.h"

namespace eCAL
{
  namespace service
  {
    // Forward declaration
    class ServerImpl;

    /**
     * @brief The eCAL::service::Server class represents a binary service server that can be used may multiple clients.
     * 
     * The Server needs an io_context to run. It will listen on a specified
     * port. When desired, the server can be opened on port 0, which will the OS
     * chose a free port.
     * The server uses one callback for the actual service function and another
     * callback for events. When stopping, a Disconnect event for each connected
     * client will be fired.
     * 
     * Callbacks are executed in the context of the io_context. Therfore, long
     * running callbacks can block the server and everything else, that is
     * dependent on the io_context.
     * 
     * =========================================================================
     * _Important_: Do not stop the io_context while the server is running.
     *              This may cause undefined behavior. In fact, the io_context
     *              should run out of work on its own.
     * 
     * _Tip_:       Use the eCAL::service::ServerManager class to manage the
     *              server's lifecycle. This enables you to stop all servers from
     *              a single place when performing an application shutdown.
     * =========================================================================
     * 
     * Sample code:
     * 
     *     auto io_context = std::make_shared<asio::io_context>();
     *     auto dummy_work = std::make_shared<asio::io_context::work>(*io_context);
     *     
     *     std::thread io_thread([&io_context]() { io_context->run(); });
     *     
     *     auto server = eCAL::service::Server::create(io_context, ...)
     *     
     *     // Do stuff
     *     
     *     server->stop();
     *     dummy_work->reset();
     *     io_thread.join();
     * 
     */
    class Server
    {
    //////////////////////////////////////////////
    // Internal types for better consistency
    //////////////////////////////////////////////
    public:
      using EventCallbackT   = ServerEventCallbackT;
      using ServiceCallbackT = ServerServiceCallbackT;
      using DeleteCallbackT  = std::function<void(Server*)>;

    ///////////////////////////////////////////
    // Constructor, Destructor, Create
    ///////////////////////////////////////////

    public:
      /**
       * @brief Creates a new Server instance.
       * 
       * A new server will directly start listening on the specified port and wait for new connections.
       * 
       * =========================================================================
       * _Important_: Do not stop the io_context while the server is running.
       *              This may cause undefined behavior. In fact, the io_context
       *              should run out of work on its own.
       * 
       * _Tip_:       Use the eCAL::service::ServerManager class to manage the
       *              server's lifecycle. This enables you to stop all servers from
       *              a single place when performing an application shutdown.
       * =========================================================================
       * 
       * @param io_context                      The io_context to use for the server and all callbacks
       * @param protocol_version                The protocol version to use. When this is 0, the buggy protocol version 0 will be used.
       * @param port                            The port to listen on. When this is 0, the OS will chose a free port.
       * @param service_callback                The callback to use for service calls. Will be executed in the context of the io_context.
       * @param parallel_service_calls_enabled  When true, service calls will be executed in parallel. When false, service calls will be executed sequentially.
       * @param event_callback                  The callback to use for events (clients connect or clients disconnect). Will be executed in the context of the io_context.
       * @param logger                          A function used for logging.
       * @param delete_callback                 A callback that will be executed when the server is deleted.
       * 
       * @return The new server instance.
       */
      static std::shared_ptr<Server> create(const std::shared_ptr<asio::io_context>& io_context
                                          , std::uint8_t                             protocol_version
                                          , std::uint16_t                            port
                                          , const ServiceCallbackT&                  service_callback
                                          , bool                                     parallel_service_calls_enabled
                                          , const EventCallbackT&                    event_callback
                                          , const LoggerT&                           logger
                                          , const DeleteCallbackT&                   delete_callback);

      static std::shared_ptr<Server> create(const std::shared_ptr<asio::io_context>& io_context
                                          , std::uint8_t                             protocol_version
                                          , std::uint16_t                            port
                                          , const ServiceCallbackT&                  service_callback
                                          , bool                                     parallel_service_calls_enabled
                                          , const EventCallbackT&                    event_callback
                                          , const LoggerT&                           logger = default_logger("Service Server"));

      static std::shared_ptr<Server> create(const std::shared_ptr<asio::io_context>& io_context
                                          , std::uint8_t                             protocol_version
                                          , std::uint16_t                            port
                                          , const ServiceCallbackT&                  service_callback
                                          , bool                                     parallel_service_calls_enabled
                                          , const EventCallbackT&                    event_callback
                                          , const DeleteCallbackT&                   delete_callback);
    protected:
      Server(const std::shared_ptr<asio::io_context>& io_context
            , std::uint8_t                            protocol_version
            , std::uint16_t                           port
            , const ServiceCallbackT&                 service_callback
            , bool                                    parallel_service_calls_enabled
            , const EventCallbackT&                   event_callback
            , const LoggerT&                          logger);

    public:
      Server(const Server&)            = delete;                  // Copy construct
      Server(Server&&)                 = delete;                  // Move construct

      Server& operator=(const Server&) = delete;                  // Copy assign
      Server& operator=(Server&&)      = delete;                  // Move assign
  
      ~Server() = default;

    ///////////////////////////////////////////
    // API
    ///////////////////////////////////////////
  
    public:

      /**
       * @brief Checks if any client is currently connected to the server.
       * 
       * @return true, when at least one client is connected. False otherwise.
       */
      bool          is_connected()         const;

      /**
       * @brief Get the number of currently connected clients.
       * 
       * @return the number of connected clients
       */
      int           get_connection_count() const;

      /**
       * @brief Returns the port the server is listening on.
       * 
       * When the server was created with port 0, this will return the port the
       * OS chose.
       * 
       * @return The port
       */
      std::uint16_t get_port()             const;
      
      /**
       * @brief Stops the server
       * 
       * This closes the socket and stops the server. After the server has been
       * stopped, it will still execute disconnect callbacks. The server will
       * not accept new connections, though.
       * 
       * A server that has been stopped cannot be restarted. Create a new
       * server, when desired.
       */
      void          stop();

    ///////////////////////////////////////////
    // Member Variables
    ///////////////////////////////////////////
    private:
      std::shared_ptr<ServerImpl> impl_;        //!< The private implementation
    };

  } // namespace service
} // namespace eCAL
