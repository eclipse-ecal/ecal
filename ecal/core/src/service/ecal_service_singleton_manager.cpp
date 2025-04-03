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

#include "ecal_service_singleton_manager.h"

#include <cstddef>
#include <ecal/log.h>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace eCAL
{
  namespace service
  {
    ecal_service::LoggerT ecal_logger(const std::string& node_name)
    {
      return [node_name](const ecal_service::LogLevel log_level, const std::string& message)
                        {
                          switch (log_level)
                          {
                          case ecal_service::LogLevel::DebugVerbose:
                            eCAL::Logging::Log(eCAL::Logging::log_level_debug4, "[" + node_name + "] " + message);
                            break;
                          case ecal_service::LogLevel::Debug:
                            eCAL::Logging::Log(eCAL::Logging::log_level_debug1, "[" + node_name + "] " + message);
                            break;
                          case ecal_service::LogLevel::Info:
                            eCAL::Logging::Log(eCAL::Logging::log_level_debug1, "[" + node_name + "] " + message);
                            break;
                          case ecal_service::LogLevel::Warning:
                            eCAL::Logging::Log(eCAL::Logging::log_level_warning, "[" + node_name + "] " + message);
                            break;
                          case ecal_service::LogLevel::Error:
                            eCAL::Logging::Log(eCAL::Logging::log_level_error, "[" + node_name + "] " + message);
                            break;
                          case ecal_service::LogLevel::Fatal:
                            eCAL::Logging::Log(eCAL::Logging::log_level_fatal, "[" + node_name + "] " + message);
                            break;
                          default:
                            break;
                          }
                        };

    }

    ////////////////////////////////////////////////////////////
	// Singleton interface, Constructor, destructor
	////////////////////////////////////////////////////////////
    constexpr size_t ServiceManager::num_io_threads;

    ServiceManager* ServiceManager::instance()
    {
      static ServiceManager instance;
      return &instance;
    }

    ServiceManager::ServiceManager()
      : m_stopped(false)
    {}

    ServiceManager::~ServiceManager()
    {
      stop();
    }

	////////////////////////////////////////////////////////////
	// Public API
	////////////////////////////////////////////////////////////

    std::shared_ptr<ecal_service::ClientManager> ServiceManager::get_client_manager()
    {
      // Quickly check the atomic stopped boolean before actually locking the
      // mutex. It can theoretically change before we got mutex access, so we
      // will have to check it again.
      if (m_stopped)
        return nullptr;

      // Lock the mutex to actually make it thread safe
      const std::lock_guard<std::mutex> singleton_lock(m_singleton_mutex);
      if (!m_stopped)
      {
        // Create io_context, if it didn't exist, yet
        if (!m_io_context)
          m_io_context = std::make_unique<asio::io_context>();

        // Create the client manager, if it didn't exist, yet
        if (!m_client_manager)
          m_client_manager = ecal_service::ClientManager::create(m_io_context, ecal_logger("Service Client"));

        // Start io threads, if necessary
        if (m_io_threads.empty())
        {
          for (size_t i = 0; i < num_io_threads; i++)
          {
            m_io_threads.emplace_back(std::make_unique<std::thread>([this]() { m_io_context->run(); }));
          }
        }
        
        // Return the client manager. The client manager has its own dummy work
        // object, so it will keep the io_context alive, until the
        // client_manager is stopped.
        return m_client_manager;
      }
      return nullptr;
    }

    std::shared_ptr<ecal_service::ServerManager> ServiceManager::get_server_manager()
    {
      // Quickly check the atomic stopped boolean before actually locking the
      // mutex. It can theoretically change before we got mutex access, so we
      // will have to check it again.
      if (m_stopped)
        return nullptr;

      // Lock the mutex to actually make it thread safe
      const std::lock_guard<std::mutex> singleton_lock(m_singleton_mutex);
      if (!m_stopped)
      {
        // Create io_context, if it didn't exist, yet
        if (!m_io_context)
          m_io_context = std::make_unique<asio::io_context>();

        // Create the server manager, if it didn't exit, yet
        if (!m_server_manager)
          m_server_manager = ecal_service::ServerManager::create(m_io_context, ecal_logger("Service Server"));

        // Start io threads, if necessary
        if (m_io_threads.empty())
        {
          for (size_t i = 0; i < num_io_threads; i++)
          {
            m_io_threads.emplace_back(std::make_unique<std::thread>([this]() { m_io_context->run(); }));
          }
        }
        
        // Return the server manager. The server manager has its own dummy work
        // object, so it will keep the io_context alive, until the
        // client_manager is stopped.
        return m_server_manager;
      }
      return nullptr;
    }

    void ServiceManager::stop()
    {
      const std::lock_guard<std::mutex> singleton_lock(m_singleton_mutex);

      m_stopped = true;

      if (m_server_manager)
        m_server_manager->stop();

      if (m_client_manager)
        m_client_manager->stop();

      for (const auto& thread : m_io_threads)
        thread->join();

      m_server_manager.reset();
      m_client_manager.reset();
      m_io_threads.clear();
      m_io_context.reset();
    }

    void ServiceManager::reset()
    {
      const std::lock_guard<std::mutex> singleton_lock(m_singleton_mutex);
      m_stopped = false;
    }

  } // namespace service
} // namespace eCAL
