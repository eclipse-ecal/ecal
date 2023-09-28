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

#include <ecal/service/server_manager.h>

namespace eCAL
{
  namespace service
  {
    ///////////////////////////////////////////////////////
    // Constructor, Destructor, Create
    ///////////////////////////////////////////////////////
    std::shared_ptr<ServerManager> ServerManager::create(const std::shared_ptr<asio::io_context>& io_context, const LoggerT& logger)
    {
      return std::shared_ptr<ServerManager>(new ServerManager(io_context, logger)); 
    }

    ServerManager::ServerManager(const std::shared_ptr<asio::io_context>& io_context, const LoggerT& logger)
      : io_context_(io_context)
      , logger_(logger)
      , stopped_(false)
      , work_(std::make_unique<asio::io_context::work>(*io_context))
    {}

    ServerManager::~ServerManager()
    {
      stop();
    }

    ///////////////////////////////////////////////////////
    // Public API
    ///////////////////////////////////////////////////////
    std::shared_ptr<Server> ServerManager::create_server(std::uint8_t                     protocol_version
                                                        , std::uint16_t                   port
                                                        , const Server::ServiceCallbackT& service_callback
                                                        , bool                            parallel_service_calls_enabled
                                                        , const Server::EventCallbackT&   event_callback)
    {
      const std::lock_guard<std::mutex> lock(server_manager_mutex_);
      if (stopped_)
      {
        return nullptr;
      }

      auto delete_callback = [weak_me = std::weak_ptr<ServerManager>(shared_from_this())](Server* server)
                              {
                                auto me = weak_me.lock();
                                if (me)
                                {
                                  // Remove the session from the sessions_ map
                                  const std::lock_guard<std::mutex> lock(me->server_manager_mutex_);
                                  me->sessions_.erase(server);
                                }
                              };
      auto server = Server::create(io_context_, protocol_version, port, service_callback, parallel_service_calls_enabled, event_callback, logger_, delete_callback);
      sessions_.emplace(server.get(), server);
      return server;
    }

    size_t ServerManager::server_count() const
    {
      const std::lock_guard<std::mutex> lock(server_manager_mutex_);
      return sessions_.size();
    }

    void ServerManager::stop()
    {
      std::map<Server*, std::weak_ptr<Server>> sessions_copy;
      {
        const std::lock_guard<std::mutex> lock(server_manager_mutex_);
        stopped_ = true;
        sessions_copy = sessions_;
      }

      // stop all dservers without having the mutex locked, so we don't crash, when this thread directly calls the delete callback, that itself needs to have the mutex locked.
      for (auto& server_weak : sessions_copy)
      {
        auto server = server_weak.second.lock();
        if (server)
          server->stop();
      }

      work_.reset();
    }

    bool ServerManager::is_stopped() const
    {
      const std::lock_guard<std::mutex> lock(server_manager_mutex_);
      return stopped_;
    }

  }
}
