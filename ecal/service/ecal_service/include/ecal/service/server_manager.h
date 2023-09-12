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
#include <map>

#include <ecal/service/server.h>

namespace eCAL
{
  namespace service
  {
    /**
     * @brief Manager for eCAL::service::server instances
     * 
     * The ServerManager keeps track of and manages all eCAL::service::server
     * instances that it created. It is used to create and stop servers. The
     * user doesn't need to manage the servers manually, e.g. for stopping them
     * from a central place.
     * 
     * The ServerManager is only available as shared_ptr. It must be created
     * using the static create() method.
     *
     * - Upon creation, the ServerManager will create a work object for the
     *   given io_context. This will keep the io_context alive, even if there
     *   are no servers running.
     * 
     * - For creating a server, the create_server() method must be used. This
     *   will create a new server instance and return a shared_ptr to it.
     * 
     * - For stopping all servers, the stop() method must be used. This
     *   will stop all servers and delete the internal work object, so the
     *   thread executing it can be joined.
     * 
     * =========================================================================
     * _Important_: Do not stop the io_context. This may cause undefined
     *              behavior. Instead, stop the server manager and wait for the
     *              io_context to run out of work on its own.
     * =========================================================================
     * 
     * Example code:
     *
     * @code {.cpp}
     * 
     *   // Create a server manager
     *   auto server_manager = eCAL::service::ServerManager::create(io_context);
     *  
     *   // Create and start an io_context thread.
     *   std::thread io_context_thread([&io_context]() { io_context->run(); });
     *  
     *   // Create actual server instances
     *   auto server1 = server_manager->create_server(...)
     *   auto server2 = server_manager->create_server(...)
     *   
     *   // DO STUFF
     * 
     *   // Stop ALL servers from a common place
     *   server_manager->stop();
     *      
     *   // Join the io_context thread. The io_context does not need to be stopped.
     *   io_context_thread.join();
     * 
     * @endcode 
     */
    class ServerManager : public std::enable_shared_from_this<ServerManager>
    {
    ///////////////////////////////////////////////////////
    // Constructor, Destructor, Create
    ///////////////////////////////////////////////////////
    public:
      /**
       * @brief Create a new server manager, that can be used to create and stop servers
       * 
       * After creation, the server manager will create a work object for the
       * given io_context. This will keep the io_context alive, even if there
       * are no servers running. When stopping the servers, that work object
       * will be deleted and the io_context will run out of work on its own.
       * 
       * @param io_context  The io context, that will be used for all servers
       * @param logger      A logger-function that will be used for by all servers
       * 
       * @return A shared_ptr to the created server manager
       */
      static std::shared_ptr<ServerManager> create(const std::shared_ptr<asio::io_context>& io_context, const LoggerT& logger = default_logger("Service Server"));

      // delete copy and move constructors and assign operators
      ServerManager(const ServerManager&) = delete;             // Copy construct
      ServerManager(ServerManager&&) = delete;                  // Move construct
      ServerManager& operator=(const ServerManager&) = delete;  // Copy assign
      ServerManager& operator=(ServerManager&&) = delete;       // Move assign

    // Constructor, Destructor
    protected:
      ServerManager(const std::shared_ptr<asio::io_context>& io_context, const LoggerT& logger);

    public:
      ~ServerManager();

    ///////////////////////////////////////////////////////
    // Public API
    ///////////////////////////////////////////////////////
    
    public:
      /**
       * @brief Create a new server instance, which is managed by this server manager.
       * 
       * The server manager will keep track of the create server internally. The
       * server (and all other servers, that have been created by this manager)
       * can be stopped via the stop() method.
       * 
       * @param protocol_version                The protocol version, that will be used by this server
       * @param port                            The port, that the server will listen on. If 0, the OS will choose a free port.
       * @param service_callback                The callback, that will be called for each incoming service call. The callback will be executed in the io_context thread.
       * @param parallel_service_calls_enabled  If true, the server will handle incoming service calls in parallel. If false, the server will handle incoming service calls sequentially.
       * @param event_callback                  The callback, that will be called whenever a client connects or disconnects. The callback will be executed in the io_context thread.
       * 
       * @return a shared pointer to the created server
       */
      std::shared_ptr<Server> create_server(std::uint8_t                    protocol_version
                                          , std::uint16_t                   port
                                          , const Server::ServiceCallbackT& service_callback
                                          , bool                            parallel_service_calls_enabled
                                          , const Server::EventCallbackT&   event_callback);

      /**
       * @brief Get the number of servers, that are currently managed by this server manager
       * @return The number of servers
       */
      size_t server_count() const;

      /**
       * @brief Stop all servers that are currently managed by this server manager
       * 
       * This will also delete the internal work object, so the thread executing
       * the io_context can be joined. The io context will run out of work on
       * its own.
       * Obviously, this is only true if no other managers etc. use the io_context.
       */
      void stop();

      /**
       * @brief Returns true, if the server manager is stopped
       * 
       * If stopped, the server manager cannot be restarted. Create a new one
       * instead. Make sure, that the io_context is executed by a thread again,
       * when creating a new server manger.
       * 
       * @return true, if the server_manager is stopped. False otherwise.
       */
      bool is_stopped() const;

    ///////////////////////////////////////////////////////
    // Member variables
    ///////////////////////////////////////////////////////
    private:
      const std::shared_ptr<asio::io_context>     io_context_;            //!< Reference to the asio io_context
      const LoggerT                               logger_;                //!< Logger for all servers

      mutable std::mutex                          server_manager_mutex_;  //!< Mutex protecting the entire class
      bool                                        stopped_;               //!< Flag indicating, if the manager is stopped
      std::unique_ptr<asio::io_context::work>     work_;                  //!< Work object to keep the io_context alive. Will be deleted, when the manager is stopped.
      std::map<Server*, std::weak_ptr<Server>>    sessions_;              //!< Map of all servers, that are currently managed by this server manager. The raw_ptr is used as key, because it is unique for each server. The weak_ptr is used to actually access the server object, because the server may already be dead and the raw ptr would be dangling in that case.
    };

  }
}
