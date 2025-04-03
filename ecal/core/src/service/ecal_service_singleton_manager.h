/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include <atomic>
#include <cstddef>
#include <ecal_service/client_manager.h>
#include <ecal_service/server_manager.h>

#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace eCAL
{
  namespace service
  {
    class ServiceManager
    {
      ////////////////////////////////////////////////////////////
      // Singleton interface, Constructor, destructor
      ////////////////////////////////////////////////////////////
    public:
      static ServiceManager* instance();

    private:
      ServiceManager();

    public:
      ~ServiceManager();

      // Delete copy constructor and assignment operator
      ServiceManager(const ServiceManager&) = delete;
      ServiceManager& operator=(const ServiceManager&) = delete;

      // Delete move constructor and assignment operator
      ServiceManager(ServiceManager&&) = delete;
      ServiceManager& operator=(ServiceManager&&) = delete;

      ////////////////////////////////////////////////////////////
      // Public API
      ////////////////////////////////////////////////////////////
    public:
      std::shared_ptr<ecal_service::ClientManager> get_client_manager();
      std::shared_ptr<ecal_service::ServerManager> get_server_manager();

      void stop();
      void reset();

      ////////////////////////////////////////////////////////////
      // Member variables
      ////////////////////////////////////////////////////////////
    private:
      static constexpr size_t num_io_threads = 4;

      std::mutex                                    m_singleton_mutex;

      std::atomic<bool>                             m_stopped;
      std::shared_ptr<asio::io_context>             m_io_context;
      std::vector<std::unique_ptr<std::thread>>     m_io_threads;

      std::shared_ptr<ecal_service::ClientManager> m_client_manager;
      std::shared_ptr<ecal_service::ServerManager> m_server_manager;
    };

  }
}
