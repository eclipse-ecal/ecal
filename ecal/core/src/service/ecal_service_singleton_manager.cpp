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

#include "ecal_service_singleton_manager.h"

#include <ecal/ecal_log.h>

namespace eCAL
{
  namespace service
  {
    namespace
    {
      std::mutex                                    singleton_mutex;

      std::atomic<bool>                             stopped(false);

      std::unique_ptr<asio::io_context>             io_context;
      std::vector<std::unique_ptr<std::thread>>     io_threads;
      constexpr size_t                              num_io_threads = 4;

      std::shared_ptr<eCAL::service::ClientManager> client_manager;
      std::shared_ptr<eCAL::service::ServerManager> server_manager;
    }

    eCAL::service::LoggerT ecal_logger(const std::string& node_name)
    {
      return [node_name](const LogLevel log_level, const std::string& message)
                        {
                          switch (log_level)
                          {
                          case LogLevel::DebugVerbose:
                            eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_debug4, "[" + node_name + "] " + message);
                            break;
                          case LogLevel::Debug:
                            eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_debug1, "[" + node_name + "] " + message);
                            break;
                          case LogLevel::Info:
                            eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_info, "[" + node_name + "] " + message);
                            break;
                          case LogLevel::Warning:
                            eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_warning, "[" + node_name + "] " + message);
                            break;
                          case LogLevel::Error:
                            eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_error, "[" + node_name + "] " + message);
                            break;
                          case LogLevel::Fatal:
                            eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_fatal, "[" + node_name + "] " + message);
                            break;
                          default:
                            break;
                          }
                        };

    }

    std::shared_ptr<eCAL::service::ClientManager> global_client_manager()
    {
      // Quickly check the atomic stopped boolean before actually locking the
      // mutex. It can theoretically change before we got mutex access, so we
      // will have to check it again.
      if (stopped)
        return nullptr;

      // Lock the mutex to actually make it thread safe
      std::lock_guard<std::mutex> singleton_lock(singleton_mutex);
      if (!stopped)
      {
        // Create io_context, if it didn't exist, yet
        if (!io_context)
          io_context = std::make_unique<asio::io_context>();

        // Create the client manager, if it didn't exit, yet
        if (!client_manager)
          client_manager = eCAL::service::ClientManager::create(*io_context, ecal_logger("Service Client"));

        // Start io threads, if necessary
        if (io_threads.empty())
        {
          for (int i = 0; i < num_io_threads; i++)
          {
            io_threads.emplace_back(std::make_unique<std::thread>([]() { io_context->run(); }));
          }
        }
        
        // Return the client manager. The client manager has its own dummy work
        // object, so it will keep the io_context alive, until the
        // client_manager is stopped.
        return client_manager;
      }
      return nullptr;
    }

    std::shared_ptr<eCAL::service::ServerManager> global_server_manager()
    {
      // Quickly check the atomic stopped boolean before actually locking the
      // mutex. It can theoretically change before we got mutex access, so we
      // will have to check it again.
      if (stopped)
        return nullptr;

      // Lock the mutex to actually make it thread safe
      std::lock_guard<std::mutex> singleton_lock(singleton_mutex);
      if (!stopped)
      {
        // Create io_context, if it didn't exist, yet
        if (!io_context)
          io_context = std::make_unique<asio::io_context>();

        // Create the server manager, if it didn't exit, yet
        if (!server_manager)
          server_manager = eCAL::service::ServerManager::create(*io_context, ecal_logger("Service Server"));

        // Start io threads, if necessary
        if (io_threads.empty())
        {
          for (int i = 0; i < num_io_threads; i++)
          {
            io_threads.emplace_back(std::make_unique<std::thread>([]() { io_context->run(); }));
          }
        }
        
        // Return the server manager. The server manager has its own dummy work
        // object, so it will keep the io_context alive, until the
        // client_manager is stopped.
        return server_manager;
      }
      return nullptr;
    }

    void stop_global_service_managers()
    {
      std::lock_guard<std::mutex> singleton_lock(singleton_mutex);

      stopped = true;

      if (server_manager)
        server_manager->stop_servers();

      if (client_manager)
        client_manager->stop_clients();

      for (const auto& thread : io_threads)
        thread->join();

      server_manager.reset();
      client_manager.reset();
      io_threads.clear();
    }
  }
}