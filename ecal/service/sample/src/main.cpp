/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2023 Continental Corporation
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
#include <ecal/service/client_manager.h>

#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char** argv)
{
  // Create an io_context
  auto io_context = std::make_shared<asio::io_context>();

  // Create a server and client manager
  auto server_manager = eCAL::service::ServerManager::create(io_context);
  auto client_manager = eCAL::service::ClientManager::create(io_context);

  // Create and start an io_context thread.
  // The io_context will be stopped, when the server_manager and client_manager are stopped.
  std::thread io_context_thread([&io_context]() { io_context->run(); });

  // Server Service callback
  // 
  // This callback will be called, when a client calls the service.
  // It is responsible for filling the response object.
  auto server_service_callback
          = [](const std::shared_ptr<const std::string>& request, const std::shared_ptr<std::string>& response) -> void
            {
                *response = "Response on \"" + *request + "\"";
            };

  // Client Callback
  //
  // This callback will be called, when the service call is finished.
  auto client_response_callback
          = [](const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
            {
              if (error)
                std::cerr << "Error calling service: " << error.ToString() << std::endl;
              else
                std::cout << "Received response: "  << *response << std::endl;
            };

  // Event callbacks (empty)
  auto server_event_callback = [](eCAL::service::ServerEventType /*event*/, const std::string& /*message*/) {};
  auto client_event_callback = [](eCAL::service::ClientEventType /*event*/, const std::string& /*message*/) {};

  // Create server
  // The server will choose a free port automatically.
  auto server = server_manager->create_server(1, 0, server_service_callback, true, server_event_callback);

  // Create client
  // The client will connect to the server on the given port.
  auto client = client_manager->create_client(1, "127.0.0.1", server->get_port(), client_event_callback);

  // Call the service non-blocking. The response will be passed to the callback.
  for (int i = 1; i <= 10; i++)
  {
    const auto request = std::make_shared<std::string>("Hello World " + std::to_string(i));
    client->async_call_service(request, client_response_callback);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  std::cout << "Shutting down :)" << std::endl;

  // Use managers to stop servers and clients
  server_manager->stop();
  client_manager->stop();

  // Join the io_context thread
  io_context_thread.join();

}
