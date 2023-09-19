# ecal-service

The eCAL Service lib encapsulates the binary data exchange of eCAL Servcies

## API Concept

- All classes are `std::shared_ptr` based. The constructors cannot be accessed, but an instanace of the class can be created by the static `::create(...)` methods.
- ASIO is exposed through the public API. It is the reponsibility of the user to create an `io_context` and have a thread executing it.
- Servers and Client should be created through the `ServerManager` and `ClientManager` classes. Those classes automatically keep the `io_context` alive.
- Having a running `io_context` is crucial for proper operation. Stopping the `io_context` can cause deadlocks.
- Instead of stopping the io_context, the user should stop their `ServerManager` and `ClientManager`. This will properly shutdown all Servers and Clients and let the `io_context` run out of work. The threads executing it can then be joined.
- There are no interal threads, only the thread(s) that the user created for executing the `io_context`. All callbacks are executed by the `io_context`, meaning that a long running callback can have a bad impact on everything else that was created with that `io_context`.

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
          = [](const std::shared_ptr<const std::string>& request, const std::shared_ptr<std::string>& response) -> void
            {
                *response = "Response on \"" + *request + "\"";
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

Currently, 2 protocols are supported:

1. **Version 0**: This is a buggy legacy version, that is only kept for compatibility. It cannot be fixed while staying compatible.
2. **Version 1**: This is the fixed proper version, that is incompatible to version 0, though. It incorporates a protocol handshake while establishing the connection and communicates the version of the used protocol. Therefore, this version is expected to be downward compatible in the future.

The user has to select manually, which protocol has be to used.

All native messages are described in [`protocol_layout.h`](ecal_service/src/protocol_layout.h). Multi-byte datatypes are always sent in network-byte-order (Big Endian).

## Version 1

- Client connects to Server.
- Client sends a ProtocolHandshakeRequest containing the maximum and minimum supported protocol version.
- Server selects one version and returns the selected protocol version as ProtocolHandshakeResponse.
    - If no common protocol version is found, the server closes the connection, instead.
- The Client now sends Requests to the Server, which is always followed by a Response sent by the Server to the Client.

All messages are prepended whith a header telling the other party about the message type and payload size. The request and response payloads are plain binary data.

```
Server                           Client 
   |                               |
   |  <- ProtocolHandshakeReq  <-  |
   |  -> ProtocolHandshakeResp ->  |
   |                               |
   |  <-------  Request ---------  |
   |  -------- Response -------->  |
   |                               |
   |  <-------  Request ---------  |
   |  -------- Response -------->  |
   |              ...              |
```

## Version 0

- Client connects to Server.
- _There is no handshake_
- The Client now sends Requests to the Server, which is always followed by a Response sent by the Server to the Client.

The Request message is prepended by a header telling the server about the payload size.
The Response message does not have any header, making it virtually impossible to tell whether more data is to be expected or not.

```
Server                           Client 
   |                               |
   |  <-------  Request ---------  |
   |  -------- Response -------->  |
   |                               |
   |  <-------  Request ---------  |
   |  -------- Response -------->  |
   |              ...              |
```