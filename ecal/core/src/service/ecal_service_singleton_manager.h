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

#include <ecal/service/server_manager.h>
#include <ecal/service/client_manager.h>

#include <memory>

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

	  // Delete copy constructor and assignment operator
	  ServiceManager(const ServiceManager&)            = delete;
	  ServiceManager& operator=(const ServiceManager&) = delete;

	  // Delete move constructor and assignment operator
	  ServiceManager(ServiceManager&&)            = delete;
	  ServiceManager& operator=(ServiceManager&&) = delete;

	public:
	  ~ServiceManager();

	////////////////////////////////////////////////////////////
	// Public API
	////////////////////////////////////////////////////////////
	public:
	  std::shared_ptr<eCAL::service::ClientManager> get_client_manager();
	  std::shared_ptr<eCAL::service::ServerManager> get_server_manager();

	  void stop();
	  void reset();

	////////////////////////////////////////////////////////////
	// Member variables
	////////////////////////////////////////////////////////////
	private:
	  static constexpr size_t num_io_threads = 4;

	  std::mutex                                    singleton_mutex;

      std::atomic<bool>                             stopped;
      std::shared_ptr<asio::io_context>             io_context;
      std::vector<std::unique_ptr<std::thread>>     io_threads;

	  std::shared_ptr<eCAL::service::ClientManager> client_manager;
      std::shared_ptr<eCAL::service::ServerManager> server_manager;
	};

  }
}
