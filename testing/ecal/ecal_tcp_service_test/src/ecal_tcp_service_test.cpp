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

#include <gtest/gtest.h>

#include <asio.hpp>

#include <iostream>
#include <atomic>

#include <ecal/service/ecal_service_server.h>
#include <ecal/service/ecal_service_client_session.h>

eCAL::service::LoggerT critical_logger(const std::string& node_name)
{
      return [node_name](const eCAL::service::LogLevel log_level, const std::string& message)
                        {
                          switch (log_level)
                          {
                          case eCAL::service::LogLevel::Warning:
                            std::cerr << "[" + node_name + "] [Warning] " + message + "\n";
                            break;
                          case eCAL::service::LogLevel::Error:
                            std::cerr << "[" + node_name + "] [Error]   " + message + "\n";
                            break;
                          case eCAL::service::LogLevel::Fatal:
                            std::cerr << "[" + node_name + "] [Fatal]   " + message + "\n";
                            break;
                          default:
                            break;
                          }
                        };
}

constexpr std::uint8_t protocol_version = 1;

#if 0
TEST(RAII, TcpServiceServer)
{
  asio::io_context io_context;
  asio::io_context::work dummy_work(io_context);

  const eCAL::service::Server::ServiceCallbackT service_callback
          = [](const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              std::cout << "Server got request: " << *request << std::endl;
              *response = "Response on \"" + *request + "\"";
              return 0;
            };


  const eCAL::service::Server::EventCallbackT event_callback
          = [](eCAL_Server_Event event, const std::string& message) -> void
            {
              std::cout << "Event " << event << ": " << message << std::endl;
            };

  std::unique_ptr<std::thread> io_thread;

  // Test auto-destruction when the shared_ptr goes out of scope

  {
    std::weak_ptr<eCAL::service::Server> tcp_server_weak;

    {
      std::shared_ptr<eCAL::service::Server> tcp_server = eCAL::service::Server::create(io_context, protocol_version, 0, service_callback, event_callback);
      tcp_server_weak = tcp_server;

      EXPECT_NE(nullptr, tcp_server);
      EXPECT_NE(nullptr, tcp_server_weak.lock());

      io_thread = std::make_unique<std::thread>([&io_context]()
                                                {
                                                  io_context.run();
                                                });
    }

    EXPECT_EQ(nullptr, tcp_server_weak.lock());
  }

  io_context.stop();
  io_thread->join();
}
#endif

#if 0
TEST(RAII, TcpServiceClient)
{
  asio::io_context io_context;
  asio::io_context::work dummy_work(io_context);

  const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = []
            (eCAL_Client_Event event, const std::string& message) -> void
            {};


  std::unique_ptr<std::thread> io_thread;

  // Test auto-destruction when the shared_ptr goes out of scope

  io_thread = std::make_unique<std::thread>([&io_context]()
                                            {
                                              io_context.run();
                                            });

  auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version, "127.0.0.1", 12345, client_event_callback);

  io_context.stop();
  io_thread->join();
}
#endif

#if 0
TEST(RAII, TcpServiceServerAndClient)
{
  asio::io_context io_context;
  asio::io_context::work dummy_work(io_context);

  std::atomic<bool> response_callback_called(false);

  const eCAL::service::Server::ServiceCallbackT server_service_callback
            = [](const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
              {
                std::cout << "Server got request: " << *request << std::endl;
                *response = "Response on \"" + *request + "\"";
                return 0;
              };

  const eCAL::service::Server::EventCallbackT server_event_callback
            = [](eCAL_Server_Event event, const std::string& message) -> void
              {
                std::cout << "Event " << event << ": " << message << std::endl;
              };

  const eCAL::service::ClientSession::ResponseCallbackT client_slow_response_callback
            = [&response_callback_called](const eCAL::service::Error& /*error*/, const std::shared_ptr<std::string>& /*response*/) -> void
              {
                // This callback just wastes some time
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                response_callback_called = true;
              };

  const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = []
            (eCAL_Client_Event event, const std::string& message) -> void
            {};


  std::unique_ptr<std::thread> io_thread;

  // Test auto-destruction when the shared_ptr goes out of scope

  {
    std::weak_ptr<eCAL::service::Server>          tcp_server_weak;
    std::weak_ptr<eCAL::service::ClientSession>   tcp_client_weak;

    std::chrono::steady_clock::time_point         start_time;

    {
      std::shared_ptr<eCAL::service::Server> tcp_server = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, server_event_callback);
      tcp_server_weak = tcp_server;

      io_thread = std::make_unique<std::thread>([&io_context]()
                                                {
                                                  io_context.run();
                                                });

      EXPECT_EQ(tcp_server->get_connection_count(), 0);

      auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", tcp_server->get_port(), client_event_callback);
      tcp_client_weak = client_v1;

      client_v1->async_call_service(std::make_shared<std::string>("Hello World"), client_slow_response_callback);
      std::this_thread::sleep_for(std::chrono::milliseconds(50));

      EXPECT_EQ(tcp_server->get_connection_count(), 1);

      // Start the "timer", when we are starting to delete the server and client
      start_time = std::chrono::steady_clock::now();
    }

    // The Client and server should both be deleted now.
    EXPECT_EQ(nullptr, tcp_server_weak.lock());
    EXPECT_EQ(nullptr, tcp_client_weak.lock());

    // The response callback should be finished, yet
    EXPECT_FALSE(response_callback_called);
  }

  io_context.stop();
  io_thread->join();

  // Now the response callback should be finished
  EXPECT_TRUE(response_callback_called);
}
#endif

#if 0
TEST(RAII, StopDuringServiceCall)
{
  asio::io_context io_context;
  asio::io_context::work dummy_work(io_context);

  std::atomic<bool> response_callback_called(false);

  const eCAL::service::Server::ServiceCallbackT server_service_callback
            = [](const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
              {
                std::cout << "Server got request: " << *request << std::endl;
                *response = "Response on \"" + *request + "\"";
                return 0;
              };

  const eCAL::service::Server::EventCallbackT server_event_callback
            = [](eCAL_Server_Event event, const std::string& message) -> void
              {
                std::cout << "Event " << event << ": " << message << std::endl;
              };

  const eCAL::service::ClientSession::ResponseCallbackT client_slow_response_callback
            = [&response_callback_called](const eCAL::service::Error /*error*/, const std::shared_ptr<std::string>& /*response*/) -> void
              {
                // This callback just wastes some time
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                response_callback_called = true;
              };

  const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = []
            (eCAL_Client_Event event, const std::string& message) -> void
            {};

  std::unique_ptr<std::thread> io_thread;

  // Test auto-destruction when the shared_ptr goes out of scope

  {
    std::weak_ptr<eCAL::service::Server>          tcp_server_weak;
    std::weak_ptr<eCAL::service::ClientSession> tcp_client_weak;

    {
      std::shared_ptr<eCAL::service::Server> tcp_server = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, server_event_callback);
      tcp_server_weak = tcp_server;

      io_thread = std::make_unique<std::thread>([&io_context]()
                                                {
                                                  io_context.run();
                                                });

      auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", tcp_server->get_port(), client_event_callback);
      tcp_client_weak = client_v1;

      client_v1->async_call_service(std::make_shared<std::string>("Hello World"), client_slow_response_callback);
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    io_context.stop();

    // The client and server should be deleted already
    EXPECT_EQ(nullptr, tcp_server_weak.lock());
    EXPECT_EQ(nullptr, tcp_client_weak.lock());

    // The callback is probably still running
    EXPECT_FALSE(response_callback_called);

    // The callback should be finished by now and the session should be deleted
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(response_callback_called);
  }

  io_thread->join();
}
#endif

#if 0
TEST(Communication, SlowCommunication)
{
  asio::io_context io_context;
  asio::io_context::work dummy_work(io_context);

  std::atomic<int> num_server_service_callback_called           (0);
  std::atomic<int> num_client_response_callback_called          (0);

  const eCAL::service::Server::ServiceCallbackT server_service_callback
          = [&num_server_service_callback_called]
            (const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              num_server_service_callback_called++;
              std::cout << "Server got request: " << *request << std::endl;
              *response = "Response on \"" + *request + "\"";
              return 0;
            };

  const eCAL::service::Server::EventCallbackT server_event_callback
          = []
            (eCAL_Server_Event event, const std::string& message) -> void
            {};

  const eCAL::service::ClientSession::ResponseCallbackT client_response_callback
          = [&num_client_response_callback_called]
            (const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
            {
              EXPECT_FALSE(bool(error));
              num_client_response_callback_called++;
              std::cout << "Client got Response: " << *response << std::endl;
            };

  const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = []
            (eCAL_Client_Event event, const std::string& message) -> void
            {};

  auto server = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, server_event_callback);

  EXPECT_NE(server->get_port(), 0);

  {
    EXPECT_EQ(num_server_service_callback_called           , 0);
    EXPECT_EQ(num_client_response_callback_called          , 0);

    EXPECT_EQ(server->get_connection_count(), 0);
  }

  auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback);

  std::thread io_thread([&io_context]()
                        {
                          io_context.run();
                        });

  // Wait a short time for the client to connect
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called           , 0);
    EXPECT_EQ(num_client_response_callback_called          , 0);

    EXPECT_EQ(server->get_connection_count(), 1);
  }

  
  // Call service and wait a short time
  client_v1->async_call_service(std::make_shared<std::string>("Hello World"), client_response_callback);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called           , 1);
    EXPECT_EQ(num_client_response_callback_called          , 1);

    EXPECT_EQ(server->get_connection_count(), 1);
  }

  // Call service again and wait a short time
  client_v1->async_call_service(std::make_shared<std::string>("Called again"), client_response_callback);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called           , 2);
    EXPECT_EQ(num_client_response_callback_called          , 2);

    EXPECT_EQ(server->get_connection_count(), 1);
  }

  // delete all objects
  client_v1 = nullptr;
  server    = nullptr;

  // join the io_thread
  io_context.stop();
  io_thread.join();
}
#endif

#if 0
TEST(CallbacksConnectDisconnect, ClientDisconnectsFirst)
{
  asio::io_context io_context;
  asio::io_context::work dummy_work(io_context);

  std::atomic<int> num_server_event_callback_called             (0);
  std::atomic<int> num_server_event_callback_called_connected   (0);
  std::atomic<int> num_server_event_callback_called_disconnected(0);
  
  std::atomic<int> num_client_event_callback_called             (0);
  std::atomic<int> num_client_event_callback_called_connected   (0);
  std::atomic<int> num_client_event_callback_called_disconnected(0);

  const eCAL::service::Server::ServiceCallbackT server_service_callback
          = []
            (const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              return 0;
            };

  const eCAL::service::Server::EventCallbackT server_event_callback
          = [&num_server_event_callback_called, &num_server_event_callback_called_connected, &num_server_event_callback_called_disconnected]
            (eCAL_Server_Event event, const std::string& message) -> void
            {
              num_server_event_callback_called++; 
              if (event == eCAL_Server_Event::server_event_connected)
                num_server_event_callback_called_connected++;
              else if (event == eCAL_Server_Event::server_event_disconnected)
                num_server_event_callback_called_disconnected++;
            };

  const eCAL::service::ClientSession::ResponseCallbackT client_response_callback
          = []
            (const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
            {};

  const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = [&num_client_event_callback_called, &num_client_event_callback_called_connected, &num_client_event_callback_called_disconnected]
            (eCAL_Client_Event event, const std::string& message) -> void
            {
              num_client_event_callback_called++; 
              if (event == eCAL_Client_Event::client_event_connected)
                num_client_event_callback_called_connected++;
              else if (event == eCAL_Client_Event::client_event_disconnected)
                num_client_event_callback_called_disconnected++;
            };

  auto server    = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, server_event_callback);
  auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback);

  std::thread io_thread([&io_context]()
                        {
                          io_context.run();
                        });

  // Wait a short time for the client to connect
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  {
    EXPECT_EQ(num_server_event_callback_called             , 1);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_event_callback_called             , 1);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);
  }

  // Client goes away
  client_v1 = nullptr;

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  {
    EXPECT_EQ(num_server_event_callback_called             , 2);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 1);

    EXPECT_EQ(num_client_event_callback_called             , 2);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 1);
  }

  server = nullptr;

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  {
    EXPECT_EQ(num_server_event_callback_called             , 2);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 1);

    EXPECT_EQ(num_client_event_callback_called             , 2);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 1);
  }

  // join the io_thread
  io_context.stop();
  io_thread.join();
}
#endif

#if 0
TEST(CommunicationAndCallbacks, ClientsDisconnectFirst)
{
  asio::io_context io_context;
  asio::io_context::work dummy_work(io_context);

  std::atomic<int> num_server_service_callback_called           (0);
  std::atomic<int> num_server_event_callback_called             (0);
  std::atomic<int> num_server_event_callback_called_connected   (0);
  std::atomic<int> num_server_event_callback_called_disconnected(0);
  
  std::atomic<int> num_client_response_callback_called          (0);
  std::atomic<int> num_client_event_callback_called             (0);
  std::atomic<int> num_client_event_callback_called_connected   (0);
  std::atomic<int> num_client_event_callback_called_disconnected(0);

  const eCAL::service::Server::ServiceCallbackT server_service_callback
          = [&num_server_service_callback_called]
            (const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              num_server_service_callback_called++;
              std::cout << "Server got request: " << *request << std::endl;
              *response = "Response on \"" + *request + "\"";
              return 0;
            };

  const eCAL::service::Server::EventCallbackT server_event_callback
          = [&num_server_event_callback_called, &num_server_event_callback_called_connected, &num_server_event_callback_called_disconnected]
            (eCAL_Server_Event event, const std::string& message) -> void
            {
              num_server_event_callback_called++; 
              
              if (event == eCAL_Server_Event::server_event_connected)
                num_server_event_callback_called_connected++;
              else if (event == eCAL_Server_Event::server_event_disconnected)
                num_server_event_callback_called_disconnected++;
            };

  const eCAL::service::ClientSession::ResponseCallbackT client_response_callback
          = [&num_client_response_callback_called]
            (const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
            {
              EXPECT_FALSE(bool(error));
              num_client_response_callback_called++;
              std::cout << "Client got Response: " << *response << std::endl;
            };

  const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = [&num_client_event_callback_called, &num_client_event_callback_called_connected, &num_client_event_callback_called_disconnected]
            (eCAL_Client_Event event, const std::string& message) -> void
            {
              num_client_event_callback_called++; 
                
              if (event == eCAL_Client_Event::client_event_connected)
                  num_client_event_callback_called_connected++;
              else if (event == eCAL_Client_Event::client_event_disconnected)
                  num_client_event_callback_called_disconnected++;
            };

  auto server = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, server_event_callback);

  EXPECT_NE(server->get_port(), 0);

  {
    EXPECT_EQ(num_server_service_callback_called           , 0);
    EXPECT_EQ(num_server_event_callback_called             , 0);
    EXPECT_EQ(num_server_event_callback_called_connected   , 0);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_response_callback_called          , 0);
    EXPECT_EQ(num_client_event_callback_called             , 0);
    EXPECT_EQ(num_client_event_callback_called_connected   , 0);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);

    EXPECT_EQ(server->get_connection_count(), 0);
  }

  auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback);

  std::thread io_thread([&io_context]()
                        {
                          io_context.run();
                        });

  // Wait a short time for the client to connect
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called           , 0);
    EXPECT_EQ(num_server_event_callback_called             , 1);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_response_callback_called          , 0);
    EXPECT_EQ(num_client_event_callback_called             , 1);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);

    EXPECT_EQ(server->get_connection_count(), 1);
  }

  
  // Call service and wait a short time
  client_v1->async_call_service(std::make_shared<std::string>("Hello World"), client_response_callback);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called           , 1);
    EXPECT_EQ(num_server_event_callback_called             , 1);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_response_callback_called          , 1);
    EXPECT_EQ(num_client_event_callback_called             , 1);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);

    EXPECT_EQ(server->get_connection_count(), 1);
  }

  // Call service again and wait a short time
  client_v1->async_call_service(std::make_shared<std::string>("Called again"), client_response_callback);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called           , 2);
    EXPECT_EQ(num_server_event_callback_called             , 1);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_response_callback_called          , 2);
    EXPECT_EQ(num_client_event_callback_called             , 1);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);

    EXPECT_EQ(server->get_connection_count(), 1);
  }

  // delete all objects
  client_v1 = nullptr;

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  {
    EXPECT_EQ(num_server_service_callback_called           , 2);
    EXPECT_EQ(num_server_event_callback_called             , 2);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 1);

    EXPECT_EQ(num_client_response_callback_called          , 2);
    EXPECT_EQ(num_client_event_callback_called             , 2);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 1);

    EXPECT_EQ(server->get_connection_count(), 0);
  }

  server = nullptr;

  {
    EXPECT_EQ(num_server_service_callback_called           , 2);
    EXPECT_EQ(num_server_event_callback_called             , 2);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 1);

    EXPECT_EQ(num_client_response_callback_called          , 2);
    EXPECT_EQ(num_client_event_callback_called             , 2);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 1);
  }

  // join the io_thread
  io_context.stop();
  io_thread.join();
}
#endif

#if 0
TEST(CommunicationAndCallbacks, ServerDisconnectsFirst)
{
  asio::io_context io_context;
  asio::io_context::work dummy_work(io_context);

  std::atomic<int> num_server_service_callback_called           (0);
  std::atomic<int> num_server_event_callback_called             (0);
  std::atomic<int> num_server_event_callback_called_connected   (0);
  std::atomic<int> num_server_event_callback_called_disconnected(0);
  
  std::atomic<int> num_client_response_callback_called          (0);
  std::atomic<int> num_client_event_callback_called             (0);
  std::atomic<int> num_client_event_callback_called_connected   (0);
  std::atomic<int> num_client_event_callback_called_disconnected(0);

  const eCAL::service::Server::ServiceCallbackT server_service_callback
          = [&num_server_service_callback_called]
            (const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              num_server_service_callback_called++;
              return 0;
            };

  const eCAL::service::Server::EventCallbackT server_event_callback
          = [&num_server_event_callback_called, &num_server_event_callback_called_connected, &num_server_event_callback_called_disconnected]
            (eCAL_Server_Event event, const std::string& message) -> void
            {
              num_server_event_callback_called++; 
              if (event == eCAL_Server_Event::server_event_connected)
                num_server_event_callback_called_connected++;
              else if (event == eCAL_Server_Event::server_event_disconnected)
                num_server_event_callback_called_disconnected++;
            };

  const eCAL::service::ClientSession::ResponseCallbackT client_response_callback
          = [&num_client_response_callback_called]
            (const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
            {
              EXPECT_FALSE(bool(error));
              num_client_response_callback_called++;
            };

  const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = [&num_client_event_callback_called, &num_client_event_callback_called_connected, &num_client_event_callback_called_disconnected]
            (eCAL_Client_Event event, const std::string& message) -> void
            {
              num_client_event_callback_called++; 
              if (event == eCAL_Client_Event::client_event_connected)
                  num_client_event_callback_called_connected++;
              else if (event == eCAL_Client_Event::client_event_disconnected)
                  num_client_event_callback_called_disconnected++;
            };

  auto server    = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, server_event_callback);
  auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback);

  std::thread io_thread([&io_context]()
                        {
                          io_context.run();
                        });

  // Wait a short time for the client to connect
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called           , 0);
    EXPECT_EQ(num_server_event_callback_called             , 1);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_response_callback_called          , 0);
    EXPECT_EQ(num_client_event_callback_called             , 1);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);

    EXPECT_EQ(server->get_connection_count(), 1);
  }

  
  // Call service and wait a short time
  client_v1->async_call_service(std::make_shared<std::string>("Hello World"), client_response_callback);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called           , 1);
    EXPECT_EQ(num_server_event_callback_called             , 1);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_response_callback_called          , 1);
    EXPECT_EQ(num_client_event_callback_called             , 1);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);
  }

  // Server goes away
  server = nullptr;

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called           , 1);
    EXPECT_EQ(num_server_event_callback_called             , 2);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 1);

    EXPECT_EQ(num_client_response_callback_called          , 1);
    EXPECT_EQ(num_client_event_callback_called             , 2);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 1);
  }

  client_v1 = nullptr;

  {
    EXPECT_EQ(num_server_service_callback_called           , 1);
    EXPECT_EQ(num_server_event_callback_called             , 2);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 1);

    EXPECT_EQ(num_client_response_callback_called          , 1);
    EXPECT_EQ(num_client_event_callback_called             , 2);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 1);
  }

  // join the io_thread
  io_context.stop();
  io_thread.join();
}
#endif

#if 0
TEST(CommunicationAndCallbacks, StressfulCommunication)
{
  constexpr int num_io_threads       = 10;
  constexpr int num_clients          = 10;
  constexpr int num_calls_per_client = 15;

  asio::io_context io_context;
  asio::io_context::work dummy_work(io_context);

  std::atomic<int> num_server_service_callback_called           (0);
  std::atomic<int> num_server_event_callback_called             (0);
  std::atomic<int> num_server_event_callback_called_connected   (0);
  std::atomic<int> num_server_event_callback_called_disconnected(0);

  std::atomic<int> num_client_response_callback_called          (0);
  std::atomic<int> num_client_event_callback_called             (0);
  std::atomic<int> num_client_event_callback_called_connected   (0);
  std::atomic<int> num_client_event_callback_called_disconnected(0);

  const eCAL::service::Server::ServiceCallbackT service_callback
          = [&num_server_service_callback_called](const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              num_server_service_callback_called++;
              *response = "Response on \"" + *request + "\"";
              return 0;
            };

  const eCAL::service::Server::EventCallbackT server_event_callback
          = [&num_server_event_callback_called, &num_server_event_callback_called_connected, &num_server_event_callback_called_disconnected]
            (eCAL_Server_Event event, const std::string& message) -> void
            {
              num_server_event_callback_called++; 
              
              if (event == eCAL_Server_Event::server_event_connected)
                num_server_event_callback_called_connected++;
              else if (event == eCAL_Server_Event::server_event_disconnected)
                num_server_event_callback_called_disconnected++;
            };

  auto server = eCAL::service::Server::create(io_context, protocol_version, 0, service_callback, server_event_callback, critical_logger("Server"));

  {
    EXPECT_EQ(num_server_service_callback_called           , 0);
    EXPECT_EQ(num_server_event_callback_called             , 0);
    EXPECT_EQ(num_server_event_callback_called_connected   , 0);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_response_callback_called          , 0);
    EXPECT_EQ(num_client_event_callback_called             , 0);
    EXPECT_EQ(num_client_event_callback_called_connected   , 0);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);
  }

  // Run the io service a bunch of times, so we hopefully trigger any race condition that may exist
  std::vector<std::unique_ptr<std::thread>> io_threads;
  io_threads.reserve(num_io_threads);
  for (int i = 0; i < num_io_threads; i++)
  {
    io_threads.emplace_back(std::make_unique<std::thread>([&io_context]() { io_context.run(); }));
  }

  // Create all the clients
  std::vector<std::shared_ptr<eCAL::service::ClientSession>> client_list;
  client_list.reserve(num_clients);
  for (int c = 0; c < num_clients; c++)
  {
    const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = [&num_client_event_callback_called, &num_client_event_callback_called_connected, &num_client_event_callback_called_disconnected]
            (eCAL_Client_Event event, const std::string& message) -> void
            {
              num_client_event_callback_called++; 
                
              if (event == eCAL_Client_Event::client_event_connected)
                num_client_event_callback_called_connected++;
              else if (event == eCAL_Client_Event::client_event_disconnected)
                num_client_event_callback_called_disconnected++;
            };
    client_list.push_back(eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback, critical_logger("Client " + std::to_string(c))));
  }

  // Directly run a bunch of clients and call each client a bunch of times
  for (int c = 0; c < client_list.size(); c++)
  {
    for (int i = 0; i < num_calls_per_client; i++)
    {
      const std::shared_ptr<std::string> request_string = std::make_shared<std::string>("Client " + std::to_string(c) + ", Call " + std::to_string(i));

      const eCAL::service::ClientSession::ResponseCallbackT response_callback
            = [&num_client_response_callback_called, request_string]
              (const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
              {
                ASSERT_FALSE(error);
                num_client_response_callback_called++;
                const std::string expected_response = "Response on \"" + *request_string + "\"";
                ASSERT_EQ(*response, expected_response);
              };

      client_list[c]->async_call_service(request_string, response_callback);
    }
  }

  // Now wait a short time for the clients to connect to the server and all requests getting executed
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  {
    EXPECT_EQ(num_server_service_callback_called           , num_clients * num_calls_per_client);
    EXPECT_EQ(num_server_event_callback_called             , num_clients);
    EXPECT_EQ(num_server_event_callback_called_connected   , num_clients);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_response_callback_called          , num_clients * num_calls_per_client);
    EXPECT_EQ(num_client_event_callback_called             , num_clients);
    EXPECT_EQ(num_client_event_callback_called_connected   , num_clients);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);

    EXPECT_EQ(server->get_connection_count(), num_clients);
  }

  // delete all objects
  server = nullptr;

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called           , num_clients * num_calls_per_client);
    EXPECT_EQ(num_server_event_callback_called             , num_clients * 2);
    EXPECT_EQ(num_server_event_callback_called_connected   , num_clients);
    EXPECT_EQ(num_server_event_callback_called_disconnected, num_clients);

    EXPECT_EQ(num_client_response_callback_called          , num_clients * num_calls_per_client);
    EXPECT_EQ(num_client_event_callback_called             , num_clients * 2);
    EXPECT_EQ(num_client_event_callback_called_connected   , num_clients);
    EXPECT_EQ(num_client_event_callback_called_disconnected, num_clients);
  }

  client_list.clear();

  // join all io_threads
  io_context.stop();
  for (int i = 0; i < io_threads.size(); i++)
  {
    io_threads[i]->join();
  }
  io_threads.clear();
}
#endif

#if 0
TEST(Callback, ServiceCallFromCallback)
{
  asio::io_context io_context;
  asio::io_context::work dummy_work(io_context);

  std::atomic<int> num_server_service_callback_called(0);
  std::atomic<int> num_client_response_callback1_called(0);
  std::atomic<int> num_client_response_callback2_called(0);

  const eCAL::service::Server::ServiceCallbackT server_service_callback
          = [&num_server_service_callback_called](const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              num_server_service_callback_called++;
              return 0;
            };

  const eCAL::service::Server::EventCallbackT server_event_callback
          = [](eCAL_Server_Event event, const std::string& message) -> void
            {};

  const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = []
            (eCAL_Client_Event event, const std::string& message) -> void
            {};

  auto server = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, server_event_callback);

  EXPECT_EQ(num_server_service_callback_called, 0);
  EXPECT_EQ(num_client_response_callback1_called, 0);
  EXPECT_EQ(num_client_response_callback2_called, 0);

  auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback);

  const eCAL::service::ClientSession::ResponseCallbackT response_callback
          = [&num_client_response_callback1_called, &num_client_response_callback2_called, client_v1]
            (const eCAL::service::Error& error, const std::shared_ptr<std::string>& /*response*/) -> void
            {
              EXPECT_FALSE(bool(error));
              num_client_response_callback1_called++;
              client_v1->async_call_service(std::make_shared<std::string>("2")
                                           , [&num_client_response_callback2_called](const eCAL::service::Error& error, const auto& /*response*/) -> void
                                              {
                                                EXPECT_FALSE(bool(error));
                                                num_client_response_callback2_called++;
                                              });
            };


  std::thread io_thread([&io_context]()
                        {
                          io_context.run();
                        });

  // Call service and wait a short time
  client_v1->async_call_service(std::make_shared<std::string>("1"), response_callback);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_EQ(num_server_service_callback_called, 2);
  EXPECT_EQ(num_client_response_callback1_called, 1);
  EXPECT_EQ(num_client_response_callback2_called, 1);

  // join the io_thread
  io_context.stop();
  io_thread.join();
}
#endif

#if 0
TEST(ErrorCallback, ErrorCallbackNoServer)
{
  asio::io_context io_context;
  asio::io_context::work dummy_work(io_context);

  std::atomic<int> num_client_response_callback_called(0);
  std::atomic<int> num_client_event_callback_called(0);

  const eCAL::service::ClientSession::ResponseCallbackT response_callback
          = [&num_client_response_callback_called](const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
            {
              EXPECT_TRUE(bool(error));
              num_client_response_callback_called++;
            };

  
  const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = [&num_client_event_callback_called]
            (eCAL_Client_Event event, const std::string& message) -> void
            {};


  EXPECT_EQ(num_client_response_callback_called, 0);

  auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version, "NonExistingEndpoint", 12345, client_event_callback);

  // Run the io_service
  std::thread io_thread([&io_context]()
                        {
                          io_context.run();
                        });

  // Call service and wait a short time
  client_v1->async_call_service(std::make_shared<std::string>("Hello World"), response_callback);
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  EXPECT_EQ(num_client_response_callback_called, 1);
  EXPECT_EQ(num_client_event_callback_called,    0);

  // join the io_thread
  io_context.stop();
  io_thread.join();
}
#endif

#if 0
TEST(ErrorCallback, ErrorCallbackServerHasDisconnected)
{
  asio::io_context io_context;
  asio::io_context::work dummy_work(io_context);

  std::atomic<int> num_server_service_callback_called           (0);
  std::atomic<int> num_server_event_callback_called             (0);
  std::atomic<int> num_server_event_callback_called_connected   (0);
  std::atomic<int> num_server_event_callback_called_disconnected(0);
  
  std::atomic<int> num_client_response_callback_called          (0);
  std::atomic<int> num_client_event_callback_called             (0);
  std::atomic<int> num_client_event_callback_called_connected   (0);
  std::atomic<int> num_client_event_callback_called_disconnected(0);

  const eCAL::service::Server::ServiceCallbackT server_service_callback
          = [&num_server_service_callback_called]
            (const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              *response = "Server running!";
              num_server_service_callback_called++; 
              return 0;
            };

  const eCAL::service::Server::EventCallbackT server_event_callback
          = [&num_server_event_callback_called, &num_server_event_callback_called_connected, &num_server_event_callback_called_disconnected]
            (eCAL_Server_Event event, const std::string& message) -> void
            {
              num_server_event_callback_called++; 
              if (event == eCAL_Server_Event::server_event_connected)
                num_server_event_callback_called_connected++;
              else if (event == eCAL_Server_Event::server_event_disconnected)
                num_server_event_callback_called_disconnected++;
            };


  const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = [&num_client_event_callback_called, &num_client_event_callback_called_connected, &num_client_event_callback_called_disconnected]
            (eCAL_Client_Event event, const std::string& message) -> void
            {
              num_client_event_callback_called++; 
              if (event == eCAL_Client_Event::client_event_connected)
                  num_client_event_callback_called_connected++;
              else if (event == eCAL_Client_Event::client_event_disconnected)
                  num_client_event_callback_called_disconnected++;
            };

  auto server    = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, server_event_callback);
  auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback);

  std::thread io_thread([&io_context]()
                        {
                          io_context.run();
                        });

  // Wait a short time for the client to connect
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  {
    EXPECT_EQ(num_server_service_callback_called           , 0);
    EXPECT_EQ(num_server_event_callback_called             , 1);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_response_callback_called          , 0);
    EXPECT_EQ(num_client_event_callback_called             , 1);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);
  }

  // First service call. Everything should be fine
  {
    const eCAL::service::ClientSession::ResponseCallbackT client_response_callback
          = [&num_client_response_callback_called]
            (const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
            {
              EXPECT_FALSE(error);
              EXPECT_EQ(*response, "Server running!");
              num_client_response_callback_called++;
            };
    client_v1->async_call_service(std::make_shared<std::string>("Everything fine?"), client_response_callback);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  {
    EXPECT_EQ(num_server_service_callback_called           , 1);
    EXPECT_EQ(num_server_event_callback_called             , 1);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_response_callback_called          , 1);
    EXPECT_EQ(num_client_event_callback_called             , 1);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);
  }

  // Server goes away
  server = nullptr;

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  {
    EXPECT_EQ(num_server_service_callback_called           , 1);
    EXPECT_EQ(num_server_event_callback_called             , 2);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 1);

    EXPECT_EQ(num_client_response_callback_called          , 1);
    EXPECT_EQ(num_client_event_callback_called             , 2);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 1);
  }

  // Service call on the dead server.
  {
    const eCAL::service::ClientSession::ResponseCallbackT client_response_callback
          = [&num_client_response_callback_called]
            (const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
            {
              EXPECT_TRUE(error);
              num_client_response_callback_called++;
            };
    client_v1->async_call_service(std::make_shared<std::string>("Everything fine?"), client_response_callback);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  // The client callback should be called another time, just with an error

  {
    EXPECT_EQ(num_server_service_callback_called           , 1);
    EXPECT_EQ(num_server_event_callback_called             , 2);
    EXPECT_EQ(num_server_event_callback_called_connected   , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 1);

    EXPECT_EQ(num_client_response_callback_called          , 2);
    EXPECT_EQ(num_client_event_callback_called             , 2);
    EXPECT_EQ(num_client_event_callback_called_connected   , 1);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 1);
  }

  // join the io_thread
  io_context.stop();
  io_thread.join();
}
#endif

#if 0
TEST(ErrorCallback, ErrorCallbackClientDisconnects)
{
  asio::io_context io_context;
  asio::io_context::work dummy_work(io_context);

  std::atomic<int> num_server_service_callback_called           (0);
  std::atomic<int> num_client_response_callback_called          (0);

  const eCAL::service::Server::ServiceCallbackT server_service_callback
          = [&num_server_service_callback_called]
            (const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              std::this_thread::sleep_for(std::chrono::milliseconds(100));
              *response = "Server running!";
              num_server_service_callback_called++; 
              return 0;
            };

  const eCAL::service::Server::EventCallbackT server_event_callback
          = []
            (eCAL_Server_Event event, const std::string& message) -> void
            {};


  const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = []
            (eCAL_Client_Event event, const std::string& message) -> void
            {};

  auto server    = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, server_event_callback);
  auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback);

  std::thread io_thread([&io_context]()
                        {
                          io_context.run();
                        });

  // Wait a short time for the client to connect
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  {
    EXPECT_EQ(num_server_service_callback_called           , 0);
    EXPECT_EQ(num_client_response_callback_called          , 0);
  }

  // First service call. Everything should be fine
  {
    const eCAL::service::ClientSession::ResponseCallbackT client_response_callback
          = [&num_client_response_callback_called]
            (const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
            {
              EXPECT_FALSE(error);
              EXPECT_EQ(*response, "Server running!");
              num_client_response_callback_called++;
            };
    client_v1->async_call_service(std::make_shared<std::string>("Everything fine?"), client_response_callback);
  }
  // Second and third service call that should fail, as we let the client go out of scope, before the server can answer on it.
  for (int i = 0; i < 2; i++)
  {
    const eCAL::service::ClientSession::ResponseCallbackT client_response_callback
          = [&num_client_response_callback_called]
            (const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
            {
              EXPECT_TRUE(error);
              EXPECT_EQ(response, nullptr);
              num_client_response_callback_called++;
            };
    client_v1->async_call_service(std::make_shared<std::string>("Everything fine?"), client_response_callback);
  }

  // The first service call should be executed by now.
  std::this_thread::sleep_for(std::chrono::milliseconds(150));

  {
    EXPECT_EQ(num_server_service_callback_called           , 1);
    EXPECT_EQ(num_client_response_callback_called          , 1);
  }

  // Client goes away
  client_v1 = nullptr;

  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  // Both service calls should have failed by now. The second should have reached the server, but the client is already gone.
  {
    EXPECT_EQ(num_server_service_callback_called           , 2);
    EXPECT_EQ(num_client_response_callback_called          , 3);
  }

  // join the io_thread
  io_context.stop();
  io_thread.join();
}
#endif

#if 0
TEST(ErrorCallback, StressfulErrorsHalfwayThrough)
{
  constexpr int num_io_threads       = 50;
  constexpr int num_clients          = 50;
  constexpr int num_calls_per_client = 50;
  constexpr std::chrono::milliseconds server_time_to_waste(10);

  // computed values
  constexpr int total_calls = num_clients * num_calls_per_client;

  // TODO: Ask Rex: is it OK to execute the service callback in parallel? That is the current case, as every server session is running independently.
  //constexpr std::chrono::milliseconds wait_time_for_destroying_server = server_time_to_waste * total_calls / 2;
  constexpr std::chrono::milliseconds wait_time_for_destroying_server = server_time_to_waste * num_calls_per_client / 2;

  asio::io_context io_context;
  asio::io_context::work dummy_work(io_context);

  std::atomic<int> num_server_service_callback_called           (0);
  std::atomic<int> num_server_event_callback_called             (0);
  std::atomic<int> num_server_event_callback_called_connected   (0);
  std::atomic<int> num_server_event_callback_called_disconnected(0);

  std::atomic<int> num_client_response_callback_called              (0);
  std::atomic<int> num_client_response_callback_called_with_error   (0);
  std::atomic<int> num_client_response_callback_called_without_error(0);
  std::atomic<int> num_client_event_callback_called                 (0);
  std::atomic<int> num_client_event_callback_called_connected       (0);
  std::atomic<int> num_client_event_callback_called_disconnected    (0);

  const eCAL::service::Server::ServiceCallbackT service_callback
          = [server_time_to_waste, &num_server_service_callback_called](const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              *response = "Response on \"" + *request + "\"";
              std::this_thread::sleep_for(server_time_to_waste);
              num_server_service_callback_called++;
              return 0;
            };

  const eCAL::service::Server::EventCallbackT server_event_callback
          = [&num_server_event_callback_called, &num_server_event_callback_called_connected, &num_server_event_callback_called_disconnected]
            (eCAL_Server_Event event, const std::string& message) -> void
            {
              num_server_event_callback_called++; 
              
              if (event == eCAL_Server_Event::server_event_connected)
                num_server_event_callback_called_connected++;
              else if (event == eCAL_Server_Event::server_event_disconnected)
                num_server_event_callback_called_disconnected++;
            };

  auto server = eCAL::service::Server::create(io_context, protocol_version, 0, service_callback, server_event_callback, critical_logger("Server"));

  {
    EXPECT_EQ(num_server_service_callback_called           , 0);
    EXPECT_EQ(num_server_event_callback_called             , 0);
    EXPECT_EQ(num_server_event_callback_called_connected   , 0);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_response_callback_called              , 0);
    EXPECT_EQ(num_client_response_callback_called_with_error   , 0);
    EXPECT_EQ(num_client_response_callback_called_without_error, 0);
    EXPECT_EQ(num_client_event_callback_called                 , 0);
    EXPECT_EQ(num_client_event_callback_called_connected       , 0);
    EXPECT_EQ(num_client_event_callback_called_disconnected    , 0);
  }

  // Run the io service a bunch of times, so we hopefully trigger any race condition that may exist
  std::vector<std::unique_ptr<std::thread>> io_threads;
  io_threads.reserve(num_io_threads);
  for (int i = 0; i < num_io_threads; i++)
  {
    io_threads.emplace_back(std::make_unique<std::thread>([&io_context]() { io_context.run(); }));
  }

  // Create all the clients
  std::vector<std::shared_ptr<eCAL::service::ClientSession>> client_list;
  client_list.reserve(num_clients);
  for (int c = 0; c < num_clients; c++)
  {
    const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = [&num_client_event_callback_called, &num_client_event_callback_called_connected, &num_client_event_callback_called_disconnected]
            (eCAL_Client_Event event, const std::string& message) -> void
            {
              num_client_event_callback_called++; 
                
              if (event == eCAL_Client_Event::client_event_connected)
                num_client_event_callback_called_connected++;
              else if (event == eCAL_Client_Event::client_event_disconnected)
                num_client_event_callback_called_disconnected++;
            };
    client_list.push_back(eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback, critical_logger("Client " + std::to_string(c))));
  }

  // Directly run a bunch of clients and call each client a bunch of times
  for (int c = 0; c < client_list.size(); c++)
  {
    for (int i = 0; i < num_calls_per_client; i++)
    {
      const std::shared_ptr<std::string> request_string = std::make_shared<std::string>("Client " + std::to_string(c) + ", Call " + std::to_string(i));

      const eCAL::service::ClientSession::ResponseCallbackT response_callback
            = [&num_client_response_callback_called, &num_client_response_callback_called_with_error, &num_client_response_callback_called_without_error, request_string]
              (const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
              {
                num_client_response_callback_called++;
                if (error)
                {
                  num_client_response_callback_called_with_error++;
                  ASSERT_EQ(response, nullptr);
                }
                else
                {
                  num_client_response_callback_called_without_error++;
                  const std::string expected_response = "Response on \"" + *request_string + "\"";
                  ASSERT_EQ(*response, expected_response);
                }
              };

      client_list[c]->async_call_service(request_string, response_callback);
    }
  }

  // Now wait a short time for some clients having executed their calls. It is not enough time for every call.
  std::this_thread::sleep_for(wait_time_for_destroying_server);

  {
    EXPECT_TRUE(num_server_service_callback_called > 0);
    EXPECT_TRUE(num_server_service_callback_called < (num_clients * num_calls_per_client));

    EXPECT_EQ(num_server_event_callback_called             , num_clients);
    EXPECT_EQ(num_server_event_callback_called_connected   , num_clients);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_TRUE(num_client_response_callback_called > 0);
    EXPECT_TRUE(num_client_response_callback_called < (num_clients* num_calls_per_client));

    EXPECT_TRUE(num_client_response_callback_called_with_error   == 0);
    EXPECT_TRUE(num_client_response_callback_called_without_error > 0);

    EXPECT_EQ(num_client_event_callback_called             , num_clients);
    EXPECT_EQ(num_client_event_callback_called_connected   , num_clients);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);

    EXPECT_EQ(server->get_connection_count(), num_clients);
  }

  // delete server
  server = nullptr;

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  {
    EXPECT_TRUE(num_server_service_callback_called > 0);
    EXPECT_TRUE(num_server_service_callback_called < (num_clients * num_calls_per_client));

    EXPECT_EQ(num_server_event_callback_called             , num_clients * 2);
    EXPECT_EQ(num_server_event_callback_called_connected   , num_clients);
    EXPECT_EQ(num_server_event_callback_called_disconnected, num_clients);

    EXPECT_EQ(num_client_response_callback_called, num_clients* num_calls_per_client);

    EXPECT_TRUE(num_client_response_callback_called_with_error    > 0);
    EXPECT_TRUE(num_client_response_callback_called_without_error > 0);

    EXPECT_EQ(num_client_event_callback_called             , num_clients * 2);
    EXPECT_EQ(num_client_event_callback_called_connected   , num_clients);
    EXPECT_EQ(num_client_event_callback_called_disconnected, num_clients);
  }

  client_list.clear();

  // join all io_threads
  io_context.stop();
  for (int i = 0; i < io_threads.size(); i++)
  {
    io_threads[i]->join();
  }
  io_threads.clear();
}
#endif

#if 0
TEST(BlockingCall, RegularBlockingCall)
{
  constexpr std::chrono::milliseconds server_callback_wait_time(50);
  constexpr int num_calls = 3;

  asio::io_context io_context;
  asio::io_context::work dummy_work(io_context);

  std::atomic<int> num_server_service_callback_called           (0);

  const eCAL::service::Server::ServiceCallbackT server_service_callback
          = [&num_server_service_callback_called, server_callback_wait_time]
            (const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              std::this_thread::sleep_for(server_callback_wait_time);
              num_server_service_callback_called++;
              *response = "Response on \"" + *request + "\"";
              return 0;
            };

  const eCAL::service::Server::EventCallbackT server_event_callback
          = []
            (eCAL_Server_Event event, const std::string& message) -> void
            {};

  const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = []
            (eCAL_Client_Event event, const std::string& message) -> void
            {};

  auto server = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, server_event_callback);
  auto client = eCAL::service::ClientSession::create(io_context, protocol_version, "127.0.0.1", server->get_port(), client_event_callback);

  std::thread io_thread([&io_context]()
                        {
                          io_context.run();
                        });

  for (int i = 0; i < num_calls; i++)
  {
    auto start = std::chrono::steady_clock::now();

    // Call the service blocking
    const auto request = std::make_shared<std::string>("Request " + std::to_string(i));
    auto response      = std::make_shared<std::string>();

    auto error  = client->call_service(request, response);

    {
      auto time_elapsed = (std::chrono::steady_clock::now() - start);

      EXPECT_FALSE(bool(error));
      EXPECT_TRUE (time_elapsed >= server_callback_wait_time);
      EXPECT_EQ   (num_server_service_callback_called, i + 1);
      EXPECT_EQ   (*response, "Response on \"" + *request + "\"");
    }
  }

  // delete all objects
  client    = nullptr;
  server    = nullptr;

  // join the io_thread
  io_context.stop();
  io_thread.join();
}
#endif

#if 1
TEST(BlockingCall, BlockingCallWithErrorHalfwayThrough)
{
  constexpr std::chrono::milliseconds server_callback_wait_time(100);
  constexpr int num_calls_before_shutdown = 3;
  constexpr int num_calls_after_shutdown  = 3;

  asio::io_context io_context;
  asio::io_context::work dummy_work(io_context);

  std::atomic<int> num_server_service_callback_called           (0);

  const eCAL::service::Server::ServiceCallbackT server_service_callback
          = [&num_server_service_callback_called, server_callback_wait_time]
            (const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              std::this_thread::sleep_for(server_callback_wait_time);
              num_server_service_callback_called++;
              *response = "Response on \"" + *request + "\"";
              return 0;
            };

  const eCAL::service::Server::EventCallbackT server_event_callback
          = []
            (eCAL_Server_Event event, const std::string& message) -> void
            {};

  const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = []
            (eCAL_Client_Event event, const std::string& message) -> void
            {};

  auto server = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, server_event_callback);
  auto client = eCAL::service::ClientSession::create(io_context, protocol_version, "127.0.0.1", server->get_port(), client_event_callback);

  std::thread io_thread([&io_context]()
                        {
                          io_context.run();
                        });

  // Wait shortly for the client to connects
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  auto test_start = std::chrono::steady_clock::now();

  // Successfull calls
  for (int i = 0; i < num_calls_before_shutdown; i++)
  {
    auto start = std::chrono::steady_clock::now();

    // Call the service blocking
    const auto request = std::make_shared<std::string>("Request " + std::to_string(i));
    auto response      = std::make_shared<std::string>();

    auto error  = client->call_service(request, response);

    {
      auto time_elapsed = (std::chrono::steady_clock::now() - start);

      EXPECT_FALSE(bool(error));
      EXPECT_TRUE (time_elapsed >= server_callback_wait_time);
      EXPECT_EQ   (num_server_service_callback_called, i + 1);
      EXPECT_EQ   (*response, "Response on \"" + *request + "\"");
    }
  }

  // Shutdown server in a few milliseconds (i.e. during the next call)
  std::thread stop_thread([&server, num_calls_before_shutdown, server_callback_wait_time]()
                        {
                          auto sleep_time = 0.5 * server_callback_wait_time;
                          std::this_thread::sleep_for(sleep_time);
                          server = nullptr;
                        });

  // First failed call
  // This call will reach the server, but the server will shut down before it
  // can return a response. So the client has to wait for the server callback to
  // finish, but then the server just closes the connection.
  {
    auto start = std::chrono::steady_clock::now();

    // Call the service blocking
    const auto request = std::make_shared<std::string>("Request");
    auto response      = std::make_shared<std::string>();

    auto error  = client->call_service(request, response);

    {
      auto time_elapsed = (std::chrono::steady_clock::now() - start);
      EXPECT_TRUE(bool(error));
      EXPECT_TRUE (time_elapsed >= server_callback_wait_time);                         // We had to wait for the server callback, but didn't get any result :(
      EXPECT_EQ   (num_server_service_callback_called, num_calls_before_shutdown + 1); // The first call did reach the server, but it didn't return, as the server was shut down before it was able to return a response
      EXPECT_EQ   (response, nullptr);
    }
  }

  // failed calls
  // These calls will not reach the server, as the server is already shut down,
  // Therefore, the client will not wait for the server callback to finish.
  for (int i = 0; i < num_calls_after_shutdown; i++)
  {
    auto start = std::chrono::steady_clock::now();

    // Call the service blocking
    const auto request = std::make_shared<std::string>("Request " + std::to_string(i));
    auto response      = std::make_shared<std::string>();

    auto error  = client->call_service(request, response);

    {
      auto time_elapsed = (std::chrono::steady_clock::now() - start);

      EXPECT_TRUE(bool(error));
      EXPECT_TRUE (time_elapsed < server_callback_wait_time);
      EXPECT_EQ   (num_server_service_callback_called, num_calls_before_shutdown + 1);
      EXPECT_EQ   (response, nullptr);
    }
  }

  stop_thread.join();

  // delete all objects
  client    = nullptr;
  server    = nullptr;

  // join the io_thread
  io_context.stop();
  io_thread.join();
}
#endif

// TODO: Test blocking call when the io_context is stopped and then a call is made
// TODO: Add a test for very big payloads that massively exceed the MTU
