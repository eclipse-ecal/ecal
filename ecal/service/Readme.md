# ecal-service

The eCAL Service lib encapsulates the binary data exchange of eCAL Servcies

## Core concept

## Example Code

The following code illustrates the usage of eCAL::service in a separate server and client program.

### Service Server

```cpp
#include <ecal/service/server_manager.h>
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char** argv)
{
  // Create an io_context
  auto io_context = std::make_shared<asio::io_context>();

  // Create a server manager
  auto server_manager = eCAL::service::ServerManager::create(io_context);

  // Create and start an io_context thread.
  // The io_context will be stopped, when the server_manager is stopped.
  std::thread io_context_thread([&io_context]() { io_context->run(); });

  // Server Service callback
  // 
  // This callback will be called, when a client calls the service.
  // It is responsible for filling the response object.
  auto server_service_callback
          = [](const std::shared_ptr<const std::string>& request, const std::shared_ptr<std::string>& response)
            {
                *response = "Response on \"" + *request + "\"";
                return 0;
            };

  // Event callback (empty)
  auto server_event_callback = [](eCAL::service::ServerEventType /*event*/, const std::string& /*message*/) {};

  // Create server on port 1589
  auto server = server_manager->create_server(1, 1589, server_service_callback, true, server_event_callback);

  // Just don't exit
  while(true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // Use managers to stop servers and clients
  server_manager->stop_servers();

  // Join the io_context thread
  io_context_thread.join();
}
```

### Service Client

```cpp
#include <ecal/service/client_manager.h>
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char** argv)
{
  // Create an io_context
  auto io_context = std::make_shared<asio::io_context>();

  // Create a client manager
  auto client_manager = eCAL::service::ClientManager::create(io_context);

  // Create and start an io_context thread.
  // The io_context will be stopped, when the client_manager is stopped.
  std::thread io_context_thread([&io_context]() { io_context->run(); });


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

  // Event callback (empty)
  auto client_event_callback = [](eCAL::service::ClientEventType /*event*/, const std::string& /*message*/) {};

  // Create client that connects to port 1589
  auto client = client_manager->create_client(1, "127.0.0.1", 1589, client_event_callback);

  // Call the service non-blocking. The response will be passed to the callback.
  for (int i = 1; i <= 10; i++)
  {
    const auto request = std::make_shared<std::string>("Hello World " + std::to_string(i));
    client->async_call_service(request, client_response_callback);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // Use managers to stop the clients
  client_manager->stop_clients();

  // Join the io_context thread
  io_context_thread.join();
}
```

## The protocol