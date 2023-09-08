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

// TODO: Document the entire eCAL::service API
namespace eCAL
{
  namespace service
  {
    class ClientManager : public std::enable_shared_from_this<ClientManager>
    {
    ///////////////////////////////////////////////////////
    // Constructor, Destructor, Create
    ///////////////////////////////////////////////////////
    public:
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
      std::shared_ptr<ClientSession> create_client(std::uint8_t                          protocol_version
                                                  , const std::string&                   address
                                                  , std::uint16_t                        port
                                                  , const ClientSession::EventCallbackT& event_callback);

      size_t client_count() const;

      void stop_clients();

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
      std::map<ClientSession*, std::weak_ptr<ClientSession>> sessions_;
    };

  }
}
