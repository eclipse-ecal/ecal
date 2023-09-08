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
  // TODO: move all raw-service-files in a common folder (tests, sample, library)
  namespace service
  {
    /**
     * @brief Manager for eCAL::service::server instances
     * 
     * The ServerManager is a singleton class, which manages all eCAL::service::server instances. It is used to create and stop servers. The user doesn't need to manage the servers manually, e.g. for stopping them from a central place and stopping the io_context.
     * The ServerManager is only available as shared_ptr. It must be created using the static create() method.
     *
     * - Upon creation, the ServerManager will create a work object for the given io_context. This will keep the io_context alive, even if there are no servers running.
     * - For creating a server, the create_server() method must be used. This will create a new server instance and return a shared_ptr to it.
     * - For stopping all servers, the stop_servers() method must be used. This will stop all servers and delete the internal work object, so the thread executing it can be joined.
     * 
     * Example code:
     * 
     *     auto server_manager = eCAL::service::ServerManager::create(io_context);

     */
    class ServerManager : public std::enable_shared_from_this<ServerManager>
    {
    ///////////////////////////////////////////////////////
    // Constructor, Destructor, Create
    ///////////////////////////////////////////////////////
    public:
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
      std::shared_ptr<Server> create_server(std::uint8_t                    protocol_version
                                          , std::uint16_t                   port
                                          , const Server::ServiceCallbackT& service_callback
                                          , bool                            parallel_service_calls_enabled
                                          , const Server::EventCallbackT&   event_callback);

      size_t server_count() const;

      void stop_servers();

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
      std::map<Server*, std::weak_ptr<Server>>    sessions_;
    };

  }
}
