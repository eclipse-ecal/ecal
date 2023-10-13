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

#include <ecal/service/client_manager.h>

namespace eCAL
{
  namespace service
  {
    ///////////////////////////////////////////////////////
    // Constructor, Destructor, Create
    ///////////////////////////////////////////////////////
    std::shared_ptr<ClientManager> ClientManager::create(const std::shared_ptr<asio::io_context>& io_context, const LoggerT& logger)
    {
      return std::shared_ptr<ClientManager>(new ClientManager(io_context, logger)); 
    }

    ClientManager::ClientManager(const std::shared_ptr<asio::io_context>& io_context, const LoggerT& logger)
      : io_context_(io_context)
      , logger_(logger)
      , stopped_(false)
      , work_(std::make_unique<asio::io_context::work>(*io_context))
    {}

    ClientManager::~ClientManager()
    {
      stop();
    }

    ///////////////////////////////////////////////////////
    // Public API
    ///////////////////////////////////////////////////////
    std::shared_ptr<ClientSession> ClientManager::create_client(std::uint8_t                          protocol_version
                                                               , const std::string&                   address
                                                               , std::uint16_t                        port
                                                               , const ClientSession::EventCallbackT& event_callback)
    {
      const std::lock_guard<std::mutex> lock(client_manager_mutex_);
      if (stopped_)
      {
        return nullptr;
      }

      auto deleter = [weak_me = std::weak_ptr<ClientManager>(shared_from_this())](ClientSession* session)
      {
        auto me = weak_me.lock();
        if (me)
        {
          // Remove the session from the sessions_ map
          const std::lock_guard<std::mutex> lock(me->client_manager_mutex_);
          me->sessions_.erase(session);
        }
      };

      auto client = ClientSession::create(io_context_, protocol_version, address, port, event_callback, logger_, deleter);
      sessions_.emplace(client.get(), client);
      return client;
    }

    size_t ClientManager::client_count() const
    {
      const std::lock_guard<std::mutex> lock(client_manager_mutex_);
      return sessions_.size();
    }

    void ClientManager::stop()
    {
      std::map<ClientSession*, std::weak_ptr<ClientSession>> sessions_copy;
      {
        const std::lock_guard<std::mutex> lock(client_manager_mutex_);
        stopped_ = true;
        sessions_copy = sessions_;
      }

      // stop all clients without having the mutex locked, so we don't crash, when this thread directly calls the delete callback, that itself needs to have the mutex locked.
      for (auto& server_weak : sessions_copy)
      {
        auto server = server_weak.second.lock();
        if (server)
          server->stop();
      }

      work_.reset();
    }

    bool ClientManager::is_stopped() const
    {
      const std::lock_guard<std::mutex> lock(client_manager_mutex_);
      return stopped_;
    }

  }
}
