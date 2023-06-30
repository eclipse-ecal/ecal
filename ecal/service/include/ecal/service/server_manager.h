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
    class ServerManager : public std::enable_shared_from_this<ServerManager>
    {
    ///////////////////////////////////////////////////////
    // Constructor, Destructor, Create
    ///////////////////////////////////////////////////////
    public:
      static std::shared_ptr<ServerManager> create(asio::io_context& io_context, const LoggerT& logger = default_logger("Service Server"));

      // delete copy and move constructors and assign operators
      ServerManager(const ServerManager&) = delete;             // Copy construct
      ServerManager(ServerManager&&) = delete;                  // Move construct
      ServerManager& operator=(const ServerManager&) = delete;  // Copy assign
      ServerManager& operator=(ServerManager&&) = delete;       // Move assign

    // Constructor, Destructor
    protected:
      ServerManager(asio::io_context& io_context, const LoggerT& logger);

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
      asio::io_context&                           io_context_;            //!< Reference to the asio io_context
      const LoggerT                               logger_;                //!< Logger for all servers

      mutable std::mutex                          server_manager_mutex_;  //!< Mutex protecting the entire class
      bool                                        stopped_;               //!< Flag indicating, if the manager is stopped
      std::unique_ptr<asio::io_context::work>     work_;                  //!< Work object to keep the io_context alive. Will be deleted, when the manager is stopped.
      std::map<Server*, std::weak_ptr<Server>>    sessions_;
    };

  }
}
