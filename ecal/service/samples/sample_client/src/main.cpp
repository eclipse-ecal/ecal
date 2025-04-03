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

#include <cstddef>
#include <cstdint>
#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <asio.hpp>

#include <ecal_service/client_manager.h>
#include <ecal_service/client_session_types.h>
#include <ecal_service/error.h>
#include <ecal_service/logger.h>

void print_usage(const std::string& arg0)
{
  std::cout << "Usage: " << arg0  << " Host:Port [Host:Port ...] [--protocol-version <Version>]" << std::endl;
}

std::pair<std::string, uint16_t> parse_server(const std::string& server)
{
  // Find the last ':' to split into host and port
  auto pos = server.find_last_of(':');
  if (pos == std::string::npos)
  {
    throw std::runtime_error("Invalid server: " + server);
  }

  return std::make_pair(server.substr(0, pos), static_cast<uint16_t>(std::stoi(server.substr(pos + 1))));
}

int main(int argc, char** argv)
{
  // Convert command line arguments into vector<std::string>
  std::vector<std::string> args;
  args.reserve(argc);
  for (int i = 0; i < argc; ++i)
  {
    args.emplace_back(argv[i]);
  }

  std::vector<std::pair<std::string, uint16_t>> server_list;
  std::uint8_t protocol_version = 1;

  // Parse command line arguments
  for (size_t i = 1; i < args.size(); ++i)
  {
    if (args[i] == "-h" || args[i] == "--help")
    {
      print_usage(args[0]);
      return 0;
    }
    else if (args[i] == "--protocol-version")
    {
      if (i + 1 < args.size())
      {
        protocol_version = static_cast<std::uint8_t>(std::stoi(args[i + 1]));
        ++i;
      }
      else
      {
        print_usage(args[0]);
        return 1;
      }
    }
    else
    {
      try
      {
        server_list.push_back(parse_server(args[i]));
      }
      catch (const std::exception& e)
      {
        std::cerr << e.what() << std::endl;
        print_usage(args[0]);
        return 1;
      }
    }
  }

  // Fail if the server list is empty
  if (server_list.empty())
  {
    print_usage(args[0]);
    return 1;
  }

  // Create an io_context
  auto io_context = std::make_shared<asio::io_context>();

  // Create a client manager
  auto client_manager = ecal_service::ClientManager::create(io_context, ecal_service::default_logger("Client", ecal_service::LogLevel::DebugVerbose));

  // Create and start an io_context thread.
  // The io_context will be stopped, when the server_manager and client_manager are stopped.
  std::thread io_context_thread([&io_context]() { io_context->run(); });

  // Client Callback
  //
  // This callback will be called, when the service call is finished.
  auto client_response_callback
          = [](const ecal_service::Error& error, const std::shared_ptr<std::string>& response) -> void
            {
              if (error)
                std::cerr << "Error calling service: " << error.ToString() << std::endl;
              else
                std::cout << "Received response: "  << *response << std::endl;
            };

  // Event callbacks (empty)
  auto client_event_callback = [](ecal_service::ClientEventType /*event*/, const std::string& /*message*/) {};

  // Create client
  // The client will connect to the server on the given port.
  auto client = client_manager->create_client(protocol_version, server_list, client_event_callback);

  // Call the service non-blocking. The response will be passed to the callback.
  int counter = 1;
  while(true)
  {
    const auto request = std::make_shared<std::string>("Hello World " + std::to_string(counter));
    client->async_call_service(request, client_response_callback);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    ++counter;
  }

  std::cout << "Shutting down :)" << std::endl;

  // Use managers to stop servers and clients
  client_manager->stop();

  // Join the io_context thread
  io_context_thread.join();

}
