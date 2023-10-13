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

#include <ecal/service/client_session.h>

namespace eCAL
{
  namespace service
  {
    /**
     * @brief Manager for eCAL::service::ClientSession instances
     * 
     * The ClientManager is used to create and stop ClientSessions. It keeps
     * track of ClientSession instances that it created. The user doesn't need
     * to manage the Sessions manually, e.g. for stopping them from a central
     * place.
     * 
     * The ClientManager is only available as shared_ptr. It must be created
     * using the static create() method.
     * 
     * - Upon creation, the ClientManager will create a work object for the
     *   given io_context. This will keep the io_context alive, even if there
     *   are no clients running.
     * 
     * - For creating a client, the create_client() method must be used. This
     *   will create a new client instance and return a shared_ptr to it.
     * 
     * - For stopping all clients, the stop() method must be used. This
     *   will stop all clients and delete the internal work object, so the 
     *   thread executing it can be joined.
     * 
     * =========================================================================
     * _Important_: Do not stop the io_context. This may cause undefined behavior.
     *              Instead, stop the client manager and wait for the io_context
     *              to run out of work on its own.
     * =========================================================================
     * 
     * Example code:
     * 
     * @code {.cpp}
     * 
     * // Create the io_context
     * auto io_context = std::make_shared<asio::io_context>();
     * 
     * // Create a thread for the io_context
     * std::thread io_context_thread([&io_context]() { io_context->run(); });
     * 
     * // Create a client manager
     * auto client_manager = eCAL::service::ClientManager::create(io_context, ...);
     * 
     * // Create actual client instances
     * auto client1 = client_manager->create_client(...)
     * auto client2 = client_manager->create_client(...)
     * 
     * // DO STUFF
     * 
     * // Stop ALL clients from a common place
     * client_manager->stop();
     * 
     * // Join the io_context thread. The io_context does not need to be stopped.
     * io_context_thread.join();
     * 
     * @endcode
     * 
     */
    class ClientManager : public std::enable_shared_from_this<ClientManager>
    {
    ///////////////////////////////////////////////////////
    // Constructor, Destructor, Create
    ///////////////////////////////////////////////////////
    public:
      /**
       * @brief Create a new ClientManager instance, that can be used to create and stop ClientSession instances.
       * 
       * After creation, the ClientManager will create a work object for the
       * given io_context. This will keep the io_context alive, even if there
       * are no clients running. When stopping the clients, that work object
       * will be deleted and the io_context will run out of work on its own.
       * 
       * @param io_context The io context, that will be used for all client sessions
       * @param logger     A logger-function that will be used for by all client sessions
       * 
       * @return A shared_ptr to the newly created ClientManager instance
       */
      static std::shared_ptr<ClientManager> create(const std::shared_ptr<asio::io_context>& io_context, const LoggerT& logger = default_logger("Service Client"));

      // delete copy and move constructors and assign operators
      ClientManager(const ClientManager&) = delete;             // Copy construct
      ClientManager(ClientManager&&) = delete;                  // Move construct
      ClientManager& operator=(const ClientManager&) = delete;  // Copy assign
      ClientManager& operator=(ClientManager&&) = delete;       // Move assign

    // Constructor, Destructor
    protected:
      ClientManager(const std::shared_ptr<asio::io_context>& io_context, const LoggerT& logger);

    public:
      ~ClientManager();

    ///////////////////////////////////////////////////////
    // Public API
    ///////////////////////////////////////////////////////
    public:
      /**
       * @brief Create a new ClienSession instance, which is managed by this client manager.
       * 
       * The ClientSession will immediatelly connect to the server through the
       * given address and port. The protocol version must match the server,
       * especially when version 0 (the legacy buggy protocol) is used. Future
       * versions are expected to be compatible to each other.
       * 
       * The Event Callback will be called, when the client has connected to the
       * server or disconnected from it. 
       * 
       * The new Client Session will be managed by the ClientManager and can be
       * stopped from this central place.
       * 
       * @param protocol_version  The protocol version to use for the client session. If 0, the legacy buggy protocol will be used.
       * @param address           The address of the server to connect to
       * @param port              The port of the server to connect to
       * @param event_callback    The callback, that will be called, when the client has connected to the server or disconnected from it. The callback will be executed in the io_context thread.
       * 
       * @return A shared_ptr to the newly created ClientSession instance
       */
      std::shared_ptr<ClientSession> create_client(std::uint8_t                          protocol_version
                                                  , const std::string&                   address
                                                  , std::uint16_t                        port
                                                  , const ClientSession::EventCallbackT& event_callback);

      /**
       * @brief Returns the number of managed client sessions
       * 
       * @return The number of managed client sessions
       */
      size_t client_count() const;

      /**
       * @brief Stops the client manager and all managed client sessions
       * 
       * This will stop all managed client sessions and delete the internal
       * work object, so the thread executing the io_context can be joined.
       * The io context will run out of work on its own.
       * 
       * Once stopped, the client manager cannot be used anymore. It must be
       * deleted and a new one must be created.
       * 
       * Make sure, that the io_context is executed by a thread again, when
       * creating a new client manager.
       */
      void stop();

      /**
       * @brief Returns true, if the client manager is stopped
       * 
       * If stopped, the client manager cannot be restarted. Create a new one
       * instead. Make sure, that the io_context is executed by a thread again,
       * when creating a new client manager.
       * 
       * @return true, if the client_manager is stopped. False otherwise.
       */
      bool is_stopped() const;

    ///////////////////////////////////////////////////////
    // Member variables
    ///////////////////////////////////////////////////////
    private:
      const std::shared_ptr<asio::io_context>     io_context_;            //!< Reference to the asio io_context
      const LoggerT                               logger_;                //!< Logger for all clients

      mutable std::mutex                          client_manager_mutex_;  //!< Mutex protecting the entire class
      bool                                        stopped_;               //!< Flag indicating, if the manager is stopped 
      std::unique_ptr<asio::io_context::work>     work_;                  //!< Work object to keep the io_context alive. Will be deleted, when the manager is stopped.
      std::map<ClientSession*, std::weak_ptr<ClientSession>> sessions_;   //!< Map of all managed client sessions. The raw_ptr is used as key, because it is unique for each client. The weak_ptr is used to actually access the client object, because the client may already be dead and the raw ptr would be dangling in that case.
    };

  }
}
