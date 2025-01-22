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
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <asio.hpp>

#include <ecal_service/server_manager.h>
#include <ecal_service/server_session_types.h>
#include <ecal_service/logger.h>

void print_usage(const std::string& arg0)
{
  std::cout << "Usage: " << arg0  << " [--port <Port>] [--protocol-version <Version>]" << std::endl;
}

int main(int argc, char** argv)
{
  uint16_t     port             = 0;
  std::uint8_t protocol_version = 1;

  // convert command line arguments into vector<std::string>
  std::vector<std::string> args;
  args.reserve(argc);
  for (int i = 0; i < argc; ++i)
  {
    args.emplace_back(argv[i]);
  }

  // parse command line arguments
  for (size_t i = 1; i < args.size(); ++i)
  {
    if (args[i] == "-h" || args[i] == "--help")
    {
      print_usage(args[0]);
      return 0;
    }
    else if (args[i] == "--port")
    {
      if (i + 1 < args.size())
      {
        port = static_cast<uint16_t>(std::stoi(args[i + 1]));
        ++i;
      }
      else
      {
        print_usage(args[0]);
        return 1;
      }
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
      print_usage(args[0]);
      return 1;
    }
  }

  // Create an io_context
  auto io_context = std::make_shared<asio::io_context>();

  // Create a server manager
  auto server_manager = ecal_service::ServerManager::create(io_context, ecal_service::default_logger("Server", ecal_service::LogLevel::DebugVerbose));

  // Server Service callback
  // 
  // This callback will be called, when a client calls the service.
  // It is responsible for filling the response object.
  auto server_service_callback
          = [](const std::shared_ptr<const std::string>& request, const std::shared_ptr<std::string>& response) -> void
            {
              *response = "Response on \"" + *request + "\"";
            };

  // Event callbacks (empty)
  auto server_event_callback = [](ecal_service::ServerEventType /*event*/, const std::string& /*message*/) {};

  // Create server
  // If the port is 0, the server will choose a port automatically
  auto server = server_manager->create_server(protocol_version, port, server_service_callback, true, server_event_callback);

  // Print server port
  std::cout << "Started Service Server on port: " << server->get_port() << std::endl;
  std::cout << "Using protocol version:         " << std::to_string(protocol_version) << std::endl;

  // Start io_context in main thread
  io_context->run();

  // Use managers to stop server
  server_manager->stop();
}
