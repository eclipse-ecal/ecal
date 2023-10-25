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
#include <thread>
#include <chrono>

#include <ecal/service/server.h> // Should not be needed, when I use the server manager / client manager
#include <ecal/service/client_session.h> // Should not be needed, when I use the server manager / client manager

#include <ecal/service/client_manager.h>
#include <ecal/service/server_manager.h>

#include "atomic_signalable.h"

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

constexpr std::uint8_t min_protocol_version = 0;
constexpr std::uint8_t max_protocol_version = 1;



#if 1
TEST(RAII, TcpServiceServer) // NOLINT
{
  for (std::uint8_t protocol_version = min_protocol_version; protocol_version <= max_protocol_version; protocol_version++)
  {
    const auto io_context = std::make_shared<asio::io_context>();
    const asio::io_context::work dummy_work(*io_context);

    const eCAL::service::Server::ServiceCallbackT service_callback
            = [](const std::shared_ptr<const std::string>& request, const std::shared_ptr<std::string>& response) -> void
              {
                std::cout << "Server got request: " << *request << std::endl;
                *response = "Response on \"" + *request + "\"";
              };


    const eCAL::service::Server::EventCallbackT event_callback
            = [](eCAL::service::ServerEventType event, const std::string& message) -> void
              {
                std::cout << "Event " << static_cast<int>(event) << ": " << message << std::endl;
              };

    std::unique_ptr<std::thread> io_thread;

    // Test auto-destruction when the shared_ptr goes out of scope

    {
      std::weak_ptr<eCAL::service::Server> tcp_server_weak;

      {
        const std::shared_ptr<eCAL::service::Server> tcp_server = eCAL::service::Server::create(io_context, protocol_version, 0, service_callback, true, event_callback);
        tcp_server_weak = tcp_server;

        EXPECT_NE(nullptr, tcp_server);
        EXPECT_NE(nullptr, tcp_server_weak.lock());

        io_thread = std::make_unique<std::thread>([&io_context]()
                                                  {
                                                    io_context->run();
                                                  });
      }

      EXPECT_EQ(nullptr, tcp_server_weak.lock());
    }

    io_context->stop();
    io_thread->join();
  }
}
#endif

#if 1
TEST(RAII, TcpServiceClient) // NOLINT
{
  for (std::uint8_t protocol_version = min_protocol_version; protocol_version <= max_protocol_version; protocol_version++)
  {
    const auto io_context = std::make_shared<asio::io_context>();
    const asio::io_context::work dummy_work(*io_context);

    const eCAL::service::ClientSession::EventCallbackT client_event_callback
            = []
              (eCAL::service::ClientEventType /*event*/, const std::string& /*message*/) -> void
              {};


    std::unique_ptr<std::thread> io_thread;

    // Test auto-destruction when the shared_ptr goes out of scope

    io_thread = std::make_unique<std::thread>([&io_context]()
                                              {
                                                io_context->run();
                                              });

    auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version, "127.0.0.1", 12345, client_event_callback);

    io_context->stop();
    io_thread->join();
  }
}
#endif

#if 1
TEST(RAII, TcpServiceServerAndClient) // NOLINT
{
  for (std::uint8_t protocol_version = min_protocol_version; protocol_version <= max_protocol_version; protocol_version++)
  {
    const auto io_context = std::make_shared<asio::io_context>();
    const asio::io_context::work dummy_work(*io_context);

    std::atomic<bool> response_callback_called(false);

    const eCAL::service::Server::ServiceCallbackT server_service_callback
              = [](const std::shared_ptr<const std::string>& request, const std::shared_ptr<std::string>& response) -> void
                {
                  std::cout << "Server got request: " << *request << std::endl;
                  *response = "Response on \"" + *request + "\"";
                };

    const eCAL::service::Server::EventCallbackT server_event_callback
              = [](eCAL::service::ServerEventType event, const std::string& message) -> void
                {
                  std::cout << "Event " << static_cast<int>(event) << ": " << message << std::endl;
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
              (eCAL::service::ClientEventType /*event*/, const std::string& /*message*/) -> void
              {};


    std::unique_ptr<std::thread> io_thread;

    // Test auto-destruction when the shared_ptr goes out of scope

    {
      std::weak_ptr<eCAL::service::Server>          tcp_server_weak;
      std::weak_ptr<eCAL::service::ClientSession>   tcp_client_weak;

      std::chrono::steady_clock::time_point         start_time;

      {
        const std::shared_ptr<eCAL::service::Server> tcp_server = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, true, server_event_callback);
        tcp_server_weak = tcp_server;

        io_thread = std::make_unique<std::thread>([&io_context]()
                                                  {
                                                    io_context->run();
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

    io_context->stop();
    io_thread->join();

    // Now the response callback should be finished
    EXPECT_TRUE(response_callback_called);
  }
}
#endif

#if 1
TEST(RAII, StopDuringServiceCall) // NOLINT
{
  for (std::uint8_t protocol_version = min_protocol_version; protocol_version <= max_protocol_version; protocol_version++)
  {
    const auto io_context = std::make_shared<asio::io_context>();
    const asio::io_context::work dummy_work(*io_context);

    std::atomic<bool> response_callback_called(false);

    const eCAL::service::Server::ServiceCallbackT server_service_callback
              = [](const std::shared_ptr<const std::string>& request, const std::shared_ptr<std::string>& response) -> void
                {
                  std::cout << "Server got request: " << *request << std::endl;
                  *response = "Response on \"" + *request + "\"";
                };

    const eCAL::service::Server::EventCallbackT server_event_callback
              = [](eCAL::service::ServerEventType event, const std::string& message) -> void
                {
                  std::cout << "Event " << static_cast<int>(event) << ": " << message << std::endl;
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
              (eCAL::service::ClientEventType /*event*/, const std::string& /*message*/) -> void
              {};

    std::unique_ptr<std::thread> io_thread;

    // Test auto-destruction when the shared_ptr goes out of scope

    {
      std::weak_ptr<eCAL::service::Server>        tcp_server_weak;
      std::weak_ptr<eCAL::service::ClientSession> tcp_client_weak;

      {
        const std::shared_ptr<eCAL::service::Server> tcp_server = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, true, server_event_callback);
        tcp_server_weak = tcp_server;

        io_thread = std::make_unique<std::thread>([&io_context]()
                                                  {
                                                    io_context->run();
                                                  });

        auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", tcp_server->get_port(), client_event_callback);
        tcp_client_weak = client_v1;

        client_v1->async_call_service(std::make_shared<std::string>("Hello World"), client_slow_response_callback);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }

      io_context->stop();

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
}
#endif

#if 1
TEST(Communication, SlowCommunication) // NOLINT
{
  for (std::uint8_t protocol_version = min_protocol_version; protocol_version <= max_protocol_version; protocol_version++)
  {
    const auto io_context = std::make_shared<asio::io_context>();
    const asio::io_context::work dummy_work(*io_context);

    std::atomic<int> num_server_service_callback_called           (0);
    std::atomic<int> num_client_response_callback_called          (0);

    const eCAL::service::Server::ServiceCallbackT server_service_callback
            = [&num_server_service_callback_called]
              (const std::shared_ptr<const std::string>& request, const std::shared_ptr<std::string>& response) -> void
              {
                num_server_service_callback_called++;
                std::cout << "Server got request: " << *request << std::endl;
                *response = "Response on \"" + *request + "\"";
              };

    const eCAL::service::Server::EventCallbackT server_event_callback
            = []
              (eCAL::service::ServerEventType /*event*/, const std::string& /*message*/) -> void
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
              (eCAL::service::ClientEventType /*event*/, const std::string& /*message*/) -> void
              {};

    auto server = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, true, server_event_callback);

    EXPECT_NE(server->get_port(), 0);

    {
      EXPECT_EQ(num_server_service_callback_called           , 0);
      EXPECT_EQ(num_client_response_callback_called          , 0);

      EXPECT_EQ(server->get_connection_count(), 0);
    }

    auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback);

    std::thread io_thread([&io_context]()
                          {
                            io_context->run();
                          });

    // Wait a short time for the client to connect
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    {
      EXPECT_EQ(num_server_service_callback_called           , 0);
      EXPECT_EQ(num_client_response_callback_called          , 0);

      EXPECT_EQ(server->get_connection_count(), 1);
      EXPECT_EQ(client_v1->get_state(), eCAL::service::State::CONNECTED);
      EXPECT_EQ(client_v1->get_accepted_protocol_version(), protocol_version);
      EXPECT_EQ(client_v1->get_queue_size(), 0);
    }

  
    // Call service and wait a short time
    client_v1->async_call_service(std::make_shared<std::string>("Hello World"), client_response_callback);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    {
      EXPECT_EQ(num_server_service_callback_called           , 1);
      EXPECT_EQ(num_client_response_callback_called          , 1);

      EXPECT_EQ(server->get_connection_count(), 1);
      EXPECT_EQ(client_v1->get_state(), eCAL::service::State::CONNECTED);
      EXPECT_EQ(client_v1->get_accepted_protocol_version(), protocol_version);
      EXPECT_EQ(client_v1->get_queue_size(), 0);
    }

    // Call service again and wait a short time
    client_v1->async_call_service(std::make_shared<std::string>("Called again"), client_response_callback);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    {
      EXPECT_EQ(num_server_service_callback_called           , 2);
      EXPECT_EQ(num_client_response_callback_called          , 2);

      EXPECT_EQ(server->get_connection_count(), 1);
      EXPECT_EQ(client_v1->get_state(), eCAL::service::State::CONNECTED);
      EXPECT_EQ(client_v1->get_accepted_protocol_version(), protocol_version);
      EXPECT_EQ(client_v1->get_queue_size(), 0);
    }

    // delete all objects
    client_v1 = nullptr;
    server    = nullptr;

    // join the io_thread
    io_context->stop();
    io_thread.join();
  }
}
#endif

#if 1
TEST(CallbacksConnectDisconnect, ClientDisconnectsFirst) // NOLINT
{
  for (std::uint8_t protocol_version = min_protocol_version; protocol_version <= max_protocol_version; protocol_version++)
  {
    const auto io_context = std::make_shared<asio::io_context>();
    const asio::io_context::work dummy_work(*io_context);

    std::atomic<int> num_server_event_callback_called             (0);
    std::atomic<int> num_server_event_callback_called_connected   (0);
    std::atomic<int> num_server_event_callback_called_disconnected(0);
  
    std::atomic<int> num_client_event_callback_called             (0);
    std::atomic<int> num_client_event_callback_called_connected   (0);
    std::atomic<int> num_client_event_callback_called_disconnected(0);

    const eCAL::service::Server::ServiceCallbackT server_service_callback
            = []
              (const std::shared_ptr<const std::string>& /*request*/, const std::shared_ptr<std::string>& /*response*/) -> void
              {};

    const eCAL::service::Server::EventCallbackT server_event_callback
            = [&num_server_event_callback_called, &num_server_event_callback_called_connected, &num_server_event_callback_called_disconnected]
              (eCAL::service::ServerEventType event, const std::string& /*message*/) -> void
              {
                if (event == eCAL::service::ServerEventType::Connected)
                  num_server_event_callback_called_connected++;
                else if (event == eCAL::service::ServerEventType::Disconnected)
                  num_server_event_callback_called_disconnected++;

                num_server_event_callback_called++; 
              };

    const eCAL::service::ClientSession::ResponseCallbackT client_response_callback
            = []
              (const eCAL::service::Error& /*error*/, const std::shared_ptr<std::string>& /*response*/) -> void
              {};

    const eCAL::service::ClientSession::EventCallbackT client_event_callback
            = [&num_client_event_callback_called, &num_client_event_callback_called_connected, &num_client_event_callback_called_disconnected]
              (eCAL::service::ClientEventType event, const std::string& /*message*/) -> void
              {
                if (event == eCAL::service::ClientEventType::Connected)
                  num_client_event_callback_called_connected++;
                else if (event == eCAL::service::ClientEventType::Disconnected)
                  num_client_event_callback_called_disconnected++;

                num_client_event_callback_called++; 
              };

    auto server = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, true, server_event_callback);
    auto client = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback);

    std::thread io_thread([&io_context]()
                          {
                            io_context->run();
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
    client = nullptr;

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
    io_context->stop();
    io_thread.join();
  }
}
#endif

#if 1
TEST(CommunicationAndCallbacks, ClientsDisconnectFirst) // NOLINT
{
  for (std::uint8_t protocol_version = min_protocol_version; protocol_version <= max_protocol_version; protocol_version++)
  {
    const auto io_context = std::make_shared<asio::io_context>();
    const asio::io_context::work dummy_work(*io_context);

    atomic_signalable<int> num_server_service_callback_called           (0);
    atomic_signalable<int> num_server_event_callback_called             (0);
    std::atomic<int>       num_server_event_callback_called_connected   (0);
    std::atomic<int>       num_server_event_callback_called_disconnected(0);
  
    atomic_signalable<int> num_client_response_callback_called          (0);
    atomic_signalable<int> num_client_event_callback_called             (0);
    std::atomic<int>       num_client_event_callback_called_connected   (0);
    std::atomic<int>       num_client_event_callback_called_disconnected(0);

    const eCAL::service::Server::ServiceCallbackT server_service_callback
            = [&num_server_service_callback_called]
              (const std::shared_ptr<const std::string>& request, const std::shared_ptr<std::string>& response) -> void
              {
                std::cout << "Server got request: " << *request << std::endl;
                *response = "Response on \"" + *request + "\"";
                num_server_service_callback_called++;
              };

    const eCAL::service::Server::EventCallbackT server_event_callback
            = [&num_server_event_callback_called, &num_server_event_callback_called_connected, &num_server_event_callback_called_disconnected]
              (eCAL::service::ServerEventType event, const std::string& /*message*/) -> void
              {              
                if (event == eCAL::service::ServerEventType::Connected)
                  num_server_event_callback_called_connected++;
                else if (event == eCAL::service::ServerEventType::Disconnected)
                  num_server_event_callback_called_disconnected++;

                num_server_event_callback_called++; 
              };

    const eCAL::service::ClientSession::ResponseCallbackT client_response_callback
            = [&num_client_response_callback_called]
              (const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
              {
                EXPECT_FALSE(bool(error));
                std::cout << "Client got Response: " << *response << std::endl;
                num_client_response_callback_called++;
              };

    const eCAL::service::ClientSession::EventCallbackT client_event_callback
            = [&num_client_event_callback_called, &num_client_event_callback_called_connected, &num_client_event_callback_called_disconnected]
              (eCAL::service::ClientEventType event, const std::string& /*message*/) -> void
              {
                if (event == eCAL::service::ClientEventType::Connected)
                    num_client_event_callback_called_connected++;
                else if (event == eCAL::service::ClientEventType::Disconnected)
                    num_client_event_callback_called_disconnected++;

                num_client_event_callback_called++; 
              };

    auto server = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, true, server_event_callback);

    EXPECT_NE(server->get_port(), 0);

    {
      EXPECT_EQ(num_server_service_callback_called.get()     , 0);
      EXPECT_EQ(num_server_event_callback_called.get()       , 0);
      EXPECT_EQ(num_server_event_callback_called_connected   , 0);
      EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

      EXPECT_EQ(num_client_response_callback_called.get()    , 0);
      EXPECT_EQ(num_client_event_callback_called.get()       , 0);
      EXPECT_EQ(num_client_event_callback_called_connected   , 0);
      EXPECT_EQ(num_client_event_callback_called_disconnected, 0);

      EXPECT_EQ(server->get_connection_count(), 0);
    }

    auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback);

    std::thread io_thread([&io_context]()
                          {
                            io_context->run();
                          });

    // Wait a short time for the client to connect
    num_server_event_callback_called.wait_for([](int v) { return v >= 1; }, std::chrono::milliseconds(100));
    num_client_event_callback_called.wait_for([](int v) { return v >= 1; }, std::chrono::milliseconds(100));

    {
      EXPECT_EQ(num_server_service_callback_called.get()     , 0);
      EXPECT_EQ(num_server_event_callback_called.get()       , 1);
      EXPECT_EQ(num_server_event_callback_called_connected   , 1);
      EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

      EXPECT_EQ(num_client_response_callback_called.get()    , 0);
      EXPECT_EQ(num_client_event_callback_called.get()       , 1);
      EXPECT_EQ(num_client_event_callback_called_connected   , 1);
      EXPECT_EQ(num_client_event_callback_called_disconnected, 0);

      EXPECT_EQ(server->get_connection_count(), 1);
      EXPECT_EQ(client_v1->get_state(), eCAL::service::State::CONNECTED);
      EXPECT_EQ(client_v1->get_accepted_protocol_version(), protocol_version);
      EXPECT_EQ(client_v1->get_queue_size(), 0);
    }

  
    // Call service and wait a short time
    client_v1->async_call_service(std::make_shared<std::string>("Hello World"), client_response_callback);
    num_server_service_callback_called .wait_for([](int v) { return v >= 1; }, std::chrono::milliseconds(100));
    num_client_response_callback_called.wait_for([](int v) { return v >= 1; }, std::chrono::milliseconds(100));

    {
      EXPECT_EQ(num_server_service_callback_called.get()     , 1);
      EXPECT_EQ(num_server_event_callback_called.get()       , 1);
      EXPECT_EQ(num_server_event_callback_called_connected   , 1);
      EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

      EXPECT_EQ(num_client_response_callback_called.get()    , 1);
      EXPECT_EQ(num_client_event_callback_called.get()       , 1);
      EXPECT_EQ(num_client_event_callback_called_connected   , 1);
      EXPECT_EQ(num_client_event_callback_called_disconnected, 0);

      EXPECT_EQ(server->get_connection_count(), 1);
      EXPECT_EQ(client_v1->get_state(), eCAL::service::State::CONNECTED);
      EXPECT_EQ(client_v1->get_accepted_protocol_version(), protocol_version);
      EXPECT_EQ(client_v1->get_queue_size(), 0);
    }

    // Call service again and wait a short time
    client_v1->async_call_service(std::make_shared<std::string>("Called again"), client_response_callback);
    num_server_service_callback_called .wait_for([](int v) { return v >= 2; }, std::chrono::milliseconds(100));
    num_client_response_callback_called.wait_for([](int v) { return v >= 2; }, std::chrono::milliseconds(100));

    {
      EXPECT_EQ(num_server_service_callback_called.get()     , 2);
      EXPECT_EQ(num_server_event_callback_called.get()       , 1);
      EXPECT_EQ(num_server_event_callback_called_connected   , 1);
      EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

      EXPECT_EQ(num_client_response_callback_called.get()    , 2);
      EXPECT_EQ(num_client_event_callback_called.get()       , 1);
      EXPECT_EQ(num_client_event_callback_called_connected   , 1);
      EXPECT_EQ(num_client_event_callback_called_disconnected, 0);

      EXPECT_EQ(server->get_connection_count(), 1);
      EXPECT_EQ(client_v1->get_state(), eCAL::service::State::CONNECTED);
      EXPECT_EQ(client_v1->get_accepted_protocol_version(), protocol_version);
      EXPECT_EQ(client_v1->get_queue_size(), 0);
    }

    // delete all objects
    client_v1 = nullptr;

    num_server_event_callback_called.wait_for([](int v) { return v >= 2; }, std::chrono::milliseconds(100));
    num_client_event_callback_called.wait_for([](int v) { return v >= 2; }, std::chrono::milliseconds(100));

    {
      EXPECT_EQ(num_server_service_callback_called.get()     , 2);
      EXPECT_EQ(num_server_event_callback_called.get()       , 2);
      EXPECT_EQ(num_server_event_callback_called_connected   , 1);
      EXPECT_EQ(num_server_event_callback_called_disconnected, 1);

      EXPECT_EQ(num_client_response_callback_called          , 2);
      EXPECT_EQ(num_client_event_callback_called             , 2);
      EXPECT_EQ(num_client_event_callback_called_connected   , 1);
      EXPECT_EQ(num_client_event_callback_called_disconnected, 1);

      EXPECT_EQ(server->get_connection_count(), 0);
      EXPECT_EQ(server->is_connected()        , false);
    }

    server = nullptr;

    {
      EXPECT_EQ(num_server_service_callback_called.get()     , 2);
      EXPECT_EQ(num_server_event_callback_called.get()       , 2);
      EXPECT_EQ(num_server_event_callback_called_connected   , 1);
      EXPECT_EQ(num_server_event_callback_called_disconnected, 1);

      EXPECT_EQ(num_client_response_callback_called.get()    , 2);
      EXPECT_EQ(num_client_event_callback_called.get()       , 2);
      EXPECT_EQ(num_client_event_callback_called_connected   , 1);
      EXPECT_EQ(num_client_event_callback_called_disconnected, 1);
    }

    // join the io_thread
    io_context->stop();
    io_thread.join();
  }
}
#endif

#if 1
TEST(CommunicationAndCallbacks, ServerDisconnectsFirst) // NOLINT
{
  for (std::uint8_t protocol_version = min_protocol_version; protocol_version <= max_protocol_version; protocol_version++)
  {
    const auto io_context = std::make_shared<asio::io_context>();
    const asio::io_context::work dummy_work(*io_context);

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
              (const std::shared_ptr<const std::string>& /*request*/, const std::shared_ptr<std::string>& /*response*/) -> void
              {
                num_server_service_callback_called++;
              };

    const eCAL::service::Server::EventCallbackT server_event_callback
            = [&num_server_event_callback_called, &num_server_event_callback_called_connected, &num_server_event_callback_called_disconnected]
              (eCAL::service::ServerEventType event, const std::string& /*message*/) -> void
              {
                if (event == eCAL::service::ServerEventType::Connected)
                  num_server_event_callback_called_connected++;
                else if (event == eCAL::service::ServerEventType::Disconnected)
                  num_server_event_callback_called_disconnected++;

                num_server_event_callback_called++; 
              };

    const eCAL::service::ClientSession::ResponseCallbackT client_response_callback
            = [&num_client_response_callback_called]
              (const eCAL::service::Error& error, const std::shared_ptr<std::string>& /*response*/) -> void
              {
                EXPECT_FALSE(bool(error));
                num_client_response_callback_called++;
              };

    const eCAL::service::ClientSession::EventCallbackT client_event_callback
            = [&num_client_event_callback_called, &num_client_event_callback_called_connected, &num_client_event_callback_called_disconnected]
              (eCAL::service::ClientEventType event, const std::string& /*message*/) -> void
              {
                if (event == eCAL::service::ClientEventType::Connected)
                    num_client_event_callback_called_connected++;
                else if (event == eCAL::service::ClientEventType::Disconnected)
                    num_client_event_callback_called_disconnected++;

                num_client_event_callback_called++; 
              };

    auto server    = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, true, server_event_callback);
    auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback);

    std::thread io_thread([&io_context]()
                          {
                            io_context->run();
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
      EXPECT_EQ(client_v1->get_state(), eCAL::service::State::CONNECTED);
      EXPECT_EQ(client_v1->get_accepted_protocol_version(), protocol_version);
      EXPECT_EQ(client_v1->get_queue_size(), 0);
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

      EXPECT_EQ(client_v1->get_state(), eCAL::service::State::CONNECTED);
      EXPECT_EQ(client_v1->get_accepted_protocol_version(), protocol_version);
      EXPECT_EQ(client_v1->get_queue_size(), 0);
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

      EXPECT_EQ(client_v1->get_state(), eCAL::service::State::FAILED);
      EXPECT_EQ(client_v1->get_accepted_protocol_version(), protocol_version);
      EXPECT_EQ(client_v1->get_queue_size(), 0);
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
    io_context->stop();
    io_thread.join();
  }
}
#endif

#if 1
TEST(CommunicationAndCallbacks, StressfulCommunication) // NOLINT
{
  for (std::uint8_t protocol_version = min_protocol_version; protocol_version <= max_protocol_version; protocol_version++)
  {
    constexpr int num_io_threads       = 10;
    constexpr int num_clients          = 10;
    constexpr int num_calls_per_client = 15;

    const auto io_context = std::make_shared<asio::io_context>();
    const asio::io_context::work dummy_work(*io_context);

    atomic_signalable<int> num_server_service_callback_called           (0);
    atomic_signalable<int> num_server_event_callback_called             (0);
    std::atomic<int>       num_server_event_callback_called_connected   (0);
    std::atomic<int>       num_server_event_callback_called_disconnected(0);

    atomic_signalable<int> num_client_response_callback_called          (0);
    atomic_signalable<int> num_client_event_callback_called             (0);
    std::atomic<int>       num_client_event_callback_called_connected   (0);
    std::atomic<int>       num_client_event_callback_called_disconnected(0);

    const eCAL::service::Server::ServiceCallbackT service_callback
            = [&num_server_service_callback_called](const std::shared_ptr<const std::string>& request, const std::shared_ptr<std::string>& response) -> void
              {
                num_server_service_callback_called++;
                *response = "Response on \"" + *request + "\"";
              };

    const eCAL::service::Server::EventCallbackT server_event_callback
            = [&num_server_event_callback_called, &num_server_event_callback_called_connected, &num_server_event_callback_called_disconnected]
              (eCAL::service::ServerEventType event, const std::string& /*message*/) -> void
              {              
                if (event == eCAL::service::ServerEventType::Connected)
                  num_server_event_callback_called_connected++;
                else if (event == eCAL::service::ServerEventType::Disconnected)
                  num_server_event_callback_called_disconnected++;

                num_server_event_callback_called++; 
              };

    auto server = eCAL::service::Server::create(io_context, protocol_version, 0, service_callback, true, server_event_callback, critical_logger("Server"));

    {
      EXPECT_EQ(num_server_service_callback_called.get()     , 0);
      EXPECT_EQ(num_server_event_callback_called.get()       , 0);
      EXPECT_EQ(num_server_event_callback_called_connected   , 0);
      EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

      EXPECT_EQ(num_client_response_callback_called.get()    , 0);
      EXPECT_EQ(num_client_event_callback_called.get()       , 0);
      EXPECT_EQ(num_client_event_callback_called_connected   , 0);
      EXPECT_EQ(num_client_event_callback_called_disconnected, 0);
    }

    // Run the io service a bunch of times, so we hopefully trigger any race condition that may exist
    std::vector<std::unique_ptr<std::thread>> io_threads;
    io_threads.reserve(num_io_threads);
    for (int i = 0; i < num_io_threads; i++)
    {
      io_threads.emplace_back(std::make_unique<std::thread>([&io_context]() { io_context->run(); }));
    }

    // Create all the clients
    std::vector<std::shared_ptr<eCAL::service::ClientSession>> client_list;
    client_list.reserve(num_clients);
    for (int c = 0; c < num_clients; c++)
    {
      const eCAL::service::ClientSession::EventCallbackT client_event_callback
            = [&num_client_event_callback_called, &num_client_event_callback_called_connected, &num_client_event_callback_called_disconnected]
              (eCAL::service::ClientEventType event, const std::string& /*message*/) -> void
              {                
                if (event == eCAL::service::ClientEventType::Connected)
                  num_client_event_callback_called_connected++;
                else if (event == eCAL::service::ClientEventType::Disconnected)
                  num_client_event_callback_called_disconnected++;

                num_client_event_callback_called++; 
              };
      client_list.push_back(eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback, critical_logger("Client " + std::to_string(c))));
    }
    
    // Directly run a bunch of clients and call each client a bunch of times
    for (size_t c = 0; c < client_list.size(); c++)
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
    num_client_response_callback_called.wait_for([num_clients, num_calls_per_client](int v) { return v == num_clients * num_calls_per_client; }, std::chrono::milliseconds(5000));
    num_server_service_callback_called .wait_for([num_clients, num_calls_per_client](int v) { return v == num_clients * num_calls_per_client; }, std::chrono::milliseconds(500));
    num_server_event_callback_called   .wait_for([num_clients](int v) { return v == num_clients; }, std::chrono::milliseconds(500));
    num_client_event_callback_called   .wait_for([num_clients](int v) { return v == num_clients; }, std::chrono::milliseconds(500));

    {
      EXPECT_EQ(num_server_service_callback_called.get()     , num_clients * num_calls_per_client);
      EXPECT_EQ(num_server_event_callback_called.get()       , num_clients);
      EXPECT_EQ(num_server_event_callback_called_connected   , num_clients);
      EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

      EXPECT_EQ(num_client_response_callback_called.get()    , num_clients * num_calls_per_client);
      EXPECT_EQ(num_client_event_callback_called.get()       , num_clients);
      EXPECT_EQ(num_client_event_callback_called_connected   , num_clients);
      EXPECT_EQ(num_client_event_callback_called_disconnected, 0);

      EXPECT_EQ(server->get_connection_count(), num_clients);
    }

    // delete all objects
    server = nullptr;

    // Wait for disconnection callbacks
    num_server_event_callback_called.wait_for([num_clients](int v) { return v == (2 * num_clients); }, std::chrono::milliseconds(5000));
    num_client_event_callback_called.wait_for([num_clients](int v) { return v == (2 * num_clients); }, std::chrono::milliseconds(5000));

    {
      EXPECT_EQ(num_server_service_callback_called.get()     , num_clients* num_calls_per_client);
      EXPECT_EQ(num_server_event_callback_called.get()       , num_clients * 2);
      EXPECT_EQ(num_server_event_callback_called_connected   , num_clients);
      EXPECT_EQ(num_server_event_callback_called_disconnected, num_clients);

      EXPECT_EQ(num_client_response_callback_called.get()    , num_clients * num_calls_per_client);
      EXPECT_EQ(num_client_event_callback_called.get()       , num_clients * 2);
      EXPECT_EQ(num_client_event_callback_called_connected   , num_clients);
      EXPECT_EQ(num_client_event_callback_called_disconnected, num_clients);
    }

    client_list.clear();

    // join all io_threads
    io_context->stop();
    for (const auto& io_thread : io_threads)
    {
      io_thread->join();
    }
    io_threads.clear();
  }
}
#endif

#if 1
TEST(CommunicationAndCallbacks, StressfulCommunicationNoParallelCalls) // NOLINT
{
  for (std::uint8_t protocol_version = min_protocol_version; protocol_version <= max_protocol_version; protocol_version++)
  {
    constexpr int num_io_threads       = 50;
    constexpr int num_clients          = 10;
    constexpr int num_calls_per_client = 2;
    constexpr std::chrono::milliseconds server_time_to_waste(50);

    const auto io_context = std::make_shared<asio::io_context>();
    auto       dummy_work = std::make_unique<asio::io_context::work>(*io_context);

    std::atomic<int> num_server_service_callback_started           (0);
    std::atomic<int> num_server_service_callback_finished          (0);

    atomic_signalable<int> num_client_response_callback_called    (0);
    atomic_signalable<int> num_clients_connected                  (0);

    const eCAL::service::Server::ServiceCallbackT service_callback
            = [&num_server_service_callback_started, &num_server_service_callback_finished, server_time_to_waste]
              (const std::shared_ptr<const std::string>& request, const std::shared_ptr<std::string>& response) -> void
              {
                EXPECT_EQ(num_server_service_callback_started, num_server_service_callback_finished);
                num_server_service_callback_started++;
                std::this_thread::sleep_for(server_time_to_waste);
                *response = "Response on \"" + *request + "\"";
                num_server_service_callback_finished++;
              };

    const eCAL::service::Server::EventCallbackT server_event_callback
            = []
              (eCAL::service::ServerEventType /*event*/, const std::string& /*message*/) -> void
              {};

    auto server = eCAL::service::Server::create(io_context, protocol_version, 0, service_callback, false, server_event_callback, critical_logger("Server"));

    {
      EXPECT_EQ(num_server_service_callback_started   , 0);
      EXPECT_EQ(num_server_service_callback_finished  , 0);
      EXPECT_EQ(num_client_response_callback_called   , 0);
    }

    // Run the io service a bunch of times, so we hopefully trigger any race condition that may exist
    std::vector<std::unique_ptr<std::thread>> io_threads;
    io_threads.reserve(num_io_threads);
    for (int i = 0; i < num_io_threads; i++)
    {
      io_threads.emplace_back(std::make_unique<std::thread>([&io_context]() { io_context->run(); }));
    }

    // Create all the clients
    std::vector<std::shared_ptr<eCAL::service::ClientSession>> client_list;
    client_list.reserve(num_clients);
    for (int c = 0; c < num_clients; c++)
    {
      const eCAL::service::ClientSession::EventCallbackT client_event_callback
            = [&num_clients_connected]
              (eCAL::service::ClientEventType event, const std::string& /*message*/) -> void
              {
                if (event == eCAL::service::ClientEventType::Connected)
                {
                  num_clients_connected++;
                }
              };
      client_list.push_back(eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback, critical_logger("Client " + std::to_string(c))));
    }

    // wait for the clients to connect
    num_clients_connected.wait_for([num_clients](int v) { return v >= num_clients; }, std::chrono::seconds(5));
    {
      EXPECT_EQ (num_clients_connected, num_clients);
    }

    auto start = std::chrono::steady_clock::now();

    // Directly run a bunch of clients and call each client a bunch of times
    for (size_t c = 0; c < client_list.size(); c++)
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

    // Now wait until all calls are finished
    const int num_total_calls = num_clients * num_calls_per_client;
    const std::chrono::milliseconds expected_time = num_total_calls * server_time_to_waste;
    num_client_response_callback_called.wait_for([num_total_calls](int v) { return v >= num_total_calls; }, expected_time * 2);

    auto end = std::chrono::steady_clock::now();

    auto total_time = end - start;

    {
      EXPECT_EQ(num_server_service_callback_started   , num_total_calls);
      EXPECT_EQ(num_server_service_callback_finished  , num_total_calls);
      EXPECT_EQ(num_client_response_callback_called   , num_total_calls);
      EXPECT_TRUE(total_time >= expected_time);
    }

    // delete all objects
    server = nullptr;
    client_list.clear();

    dummy_work.reset();
    // join all io_threads
    for (const auto& io_thread : io_threads)
    {
      io_thread->join();
    }
  }
}
#endif

#if 1
TEST(CommunicationAndCallbacks, StressfulCommunicationMassivePayload) // NOLINT
{
  // This test does not work for Protocol version 0 and there is no way to fix that (which is the reason why we invented protocol version 1)

  constexpr std::uint8_t protocol_version = 1;
  
  constexpr int num_io_threads       = 5;
  constexpr int num_clients          = 3;
  constexpr int num_calls_per_client = 5;
  constexpr int payload_size_bytes   = 32 * 1024 * 1024;

  const auto payload = std::make_shared<const std::string>(payload_size_bytes, 'e');

  const auto io_context = std::make_shared<asio::io_context>();
  const asio::io_context::work dummy_work(*io_context);

  atomic_signalable<int> num_server_service_callback_called           (0);
  atomic_signalable<int> num_server_event_callback_called             (0);
  std::atomic<int>       num_server_event_callback_called_connected   (0);
  std::atomic<int>       num_server_event_callback_called_disconnected(0);

  atomic_signalable<int> num_client_response_callback_called          (0);
  atomic_signalable<int> num_client_event_callback_called             (0);
  std::atomic<int>       num_client_event_callback_called_connected   (0);
  std::atomic<int>       num_client_event_callback_called_disconnected(0);

  const eCAL::service::Server::ServiceCallbackT service_callback
          = [&num_server_service_callback_called, payload_size_bytes](const std::shared_ptr<const std::string>& request, const std::shared_ptr<std::string>& response) -> void
            {
              EXPECT_EQ(request->size(), payload_size_bytes);
              num_server_service_callback_called++;
              *response = *request;
            };

  const eCAL::service::Server::EventCallbackT server_event_callback
          = [&num_server_event_callback_called, &num_server_event_callback_called_connected, &num_server_event_callback_called_disconnected]
            (eCAL::service::ServerEventType event, const std::string& /*message*/) -> void
            {
              num_server_event_callback_called++; 
              
              if (event == eCAL::service::ServerEventType::Connected)
                num_server_event_callback_called_connected++;
              else if (event == eCAL::service::ServerEventType::Disconnected)
                num_server_event_callback_called_disconnected++;
            };

  auto server = eCAL::service::Server::create(io_context, protocol_version, 0, service_callback, true, server_event_callback, critical_logger("Server"));

  {
    EXPECT_EQ(num_server_service_callback_called.get()     , 0);
    EXPECT_EQ(num_server_event_callback_called.get()       , 0);
    EXPECT_EQ(num_server_event_callback_called_connected   , 0);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_response_callback_called.get()    , 0);
    EXPECT_EQ(num_client_event_callback_called.get()       , 0);
    EXPECT_EQ(num_client_event_callback_called_connected   , 0);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);
  }

  // Run the io service a bunch of times, so we hopefully trigger any race condition that may exist
  std::vector<std::unique_ptr<std::thread>> io_threads;
  io_threads.reserve(num_io_threads);
  for (int i = 0; i < num_io_threads; i++)
  {
    io_threads.emplace_back(std::make_unique<std::thread>([&io_context]() { io_context->run(); }));
  }

  // Create all the clients
  std::vector<std::shared_ptr<eCAL::service::ClientSession>> client_list;
  client_list.reserve(num_clients);
  for (int c = 0; c < num_clients; c++)
  {
    const eCAL::service::ClientSession::EventCallbackT client_event_callback
          = [&num_client_event_callback_called, &num_client_event_callback_called_connected, &num_client_event_callback_called_disconnected]
            (eCAL::service::ClientEventType event, const std::string& /*message*/) -> void
            {  
              if (event == eCAL::service::ClientEventType::Connected)
                num_client_event_callback_called_connected++;
              else if (event == eCAL::service::ClientEventType::Disconnected)
                num_client_event_callback_called_disconnected++;

              num_client_event_callback_called++; 
            };
    client_list.push_back(eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback, critical_logger("Client " + std::to_string(c))));
  }

  // Directly run a bunch of clients and call each client a bunch of times
  for (auto & client : client_list)
  {
    for (int i = 0; i < num_calls_per_client; i++)
    {
      const eCAL::service::ClientSession::ResponseCallbackT response_callback
            = [&num_client_response_callback_called, payload_size_bytes]
              (const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
              {
                ASSERT_FALSE(error);
                EXPECT_EQ(response->size(), payload_size_bytes);
                num_client_response_callback_called++;
              };

      client->async_call_service(payload, response_callback);
    }
  }

  // Now wait a short time for the clients to connect to the server and all requests getting executed
  auto num_total_calls = num_clients * num_calls_per_client;
  num_client_response_callback_called.wait_for([num_total_calls](int v) { return v >= num_total_calls; }, std::chrono::milliseconds(5000));
  num_server_service_callback_called .wait_for([num_total_calls](int v) { return v >= num_total_calls; }, std::chrono::milliseconds(500));
  num_client_event_callback_called   .wait_for([num_clients](int v) { return v >= num_clients; }, std::chrono::milliseconds(500));
  num_server_event_callback_called   .wait_for([num_clients](int v) { return v >= num_clients; }, std::chrono::milliseconds(500));

  {
    EXPECT_EQ(num_server_service_callback_called.get()     , num_clients * num_calls_per_client);
    EXPECT_EQ(num_server_event_callback_called.get()       , num_clients);
    EXPECT_EQ(num_server_event_callback_called_connected   , num_clients);
    EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

    EXPECT_EQ(num_client_response_callback_called.get()    , num_clients * num_calls_per_client);
    EXPECT_EQ(num_client_event_callback_called.get()       , num_clients);
    EXPECT_EQ(num_client_event_callback_called_connected   , num_clients);
    EXPECT_EQ(num_client_event_callback_called_disconnected, 0);

    EXPECT_EQ(server->get_connection_count(), num_clients);
  }

  // delete all objects
  server = nullptr;

  num_client_event_callback_called   .wait_for([num_clients](int v) { return v >= (2 * num_clients); }, std::chrono::milliseconds(5000));
  num_server_event_callback_called   .wait_for([num_clients](int v) { return v >= (2 * num_clients); }, std::chrono::milliseconds(500));

  {
    EXPECT_EQ(num_server_service_callback_called.get()     , num_clients * num_calls_per_client);
    EXPECT_EQ(num_server_event_callback_called.get()       , num_clients * 2);
    EXPECT_EQ(num_server_event_callback_called_connected   , num_clients);
    EXPECT_EQ(num_server_event_callback_called_disconnected, num_clients);

    EXPECT_EQ(num_client_response_callback_called.get()    , num_clients * num_calls_per_client);
    EXPECT_EQ(num_client_event_callback_called.get()       , num_clients * 2);
    EXPECT_EQ(num_client_event_callback_called_connected   , num_clients);
    EXPECT_EQ(num_client_event_callback_called_disconnected, num_clients);
  }

  client_list.clear();

  // join all io_threads
  io_context->stop();
  for (const auto& io_thread : io_threads)
  {
    io_thread->join();
  }
  io_threads.clear();
}
#endif

#if 1
TEST(Callback, ServerAndClientManagers) // NOLINT
{
  for (std::uint8_t protocol_version = min_protocol_version; protocol_version <= max_protocol_version; protocol_version++)
  {
    constexpr int num_io_threads         = 5;
    std::vector<std::unique_ptr<std::thread>> io_threads;
    io_threads.reserve(num_io_threads);

    const auto io_context = std::make_shared<asio::io_context>();
    auto client_manager = eCAL::service::ClientManager::create(io_context);
    auto server_manager = eCAL::service::ServerManager::create(io_context);

    atomic_signalable<int> server1_event_callback_called(0);
    atomic_signalable<int> server2_event_callback_called(0);

    atomic_signalable<int> client1_1_event_callback_called(0);
    atomic_signalable<int> client1_2_event_callback_called(0);
    atomic_signalable<int> client2_1_event_callback_called(0);
    atomic_signalable<int> client2_2_event_callback_called(0);

    for (int i = 0; i < num_io_threads; ++i)
    {
      io_threads.emplace_back(std::make_unique<std::thread>([&io_context]()
                              {
                                io_context->run();
                              }));
    }

    const eCAL::service::Server::ServiceCallbackT         server_service_callback  = [](auto, auto) -> void {};
    const eCAL::service::ClientSession::ResponseCallbackT client_response_callback = [](auto, auto) -> void {};

    // Lambda function that on call returns another lambda function that will increment the given atomic_signalable
    auto increment_atomic_signalable = [](auto& atomic_signalable) -> auto
    {
      return [&atomic_signalable](auto, auto) -> void
      {
        atomic_signalable++;
      };
    };


    auto server1 = server_manager->create_server(protocol_version, 0, server_service_callback, true, increment_atomic_signalable(server1_event_callback_called));
    auto server2 = server_manager->create_server(protocol_version, 0, server_service_callback, true, increment_atomic_signalable(server2_event_callback_called));

    auto client1_1 = client_manager->create_client(protocol_version, "127.0.0.1", server1->get_port(), increment_atomic_signalable(client1_1_event_callback_called));
    auto client1_2 = client_manager->create_client(protocol_version, "127.0.0.1", server1->get_port(), increment_atomic_signalable(client1_2_event_callback_called));
    auto client2_1 = client_manager->create_client(protocol_version, "127.0.0.1", server2->get_port(), increment_atomic_signalable(client2_1_event_callback_called));
    auto client2_2 = client_manager->create_client(protocol_version, "127.0.0.1", server2->get_port(), increment_atomic_signalable(client2_2_event_callback_called));

    // Wait for the clients to be connected
    client1_1_event_callback_called.wait_for([&](int value) { return value >= 1; }, std::chrono::seconds(5));
    client1_2_event_callback_called.wait_for([&](int value) { return value >= 1; }, std::chrono::seconds(1));
    client2_1_event_callback_called.wait_for([&](int value) { return value >= 1; }, std::chrono::seconds(1));
    client2_2_event_callback_called.wait_for([&](int value) { return value >= 1; }, std::chrono::seconds(1));

    server1_event_callback_called.wait_for([&](int value) { return value >= 2; }, std::chrono::seconds(1));
    server2_event_callback_called.wait_for([&](int value) { return value >= 2; }, std::chrono::seconds(1));

    {
      EXPECT_EQ(server1_event_callback_called,   2);
      EXPECT_EQ(server2_event_callback_called,   2);
      EXPECT_EQ(client1_1_event_callback_called, 1);
      EXPECT_EQ(client1_2_event_callback_called, 1);
      EXPECT_EQ(client2_1_event_callback_called, 1);
      EXPECT_EQ(client2_2_event_callback_called, 1);

      EXPECT_EQ(server_manager->server_count(), 2);
      EXPECT_EQ(client_manager->client_count(), 4);
    }

    // Test what happens when the user deletes the client on his own
    client1_1.reset();
    client1_1_event_callback_called.wait_for([&](int value) { return value >= 2; }, std::chrono::seconds(1));
    server1_event_callback_called.wait_for([&](int value) { return value >= 3; }, std::chrono::seconds(1));

    {
      EXPECT_EQ(server1_event_callback_called,   3);
      EXPECT_EQ(server2_event_callback_called,   2);
      EXPECT_EQ(client1_1_event_callback_called, 2);
      EXPECT_EQ(client1_2_event_callback_called, 1);
      EXPECT_EQ(client2_1_event_callback_called, 1);
      EXPECT_EQ(client2_2_event_callback_called, 1);

      EXPECT_EQ(server_manager->server_count(), 2);
      EXPECT_EQ(client_manager->client_count(), 3);
    }

    // Test what happens when the user deletes the server on his own
    server1.reset();
    server1_event_callback_called.wait_for([&](int value) { return value >= 4; }, std::chrono::seconds(1));
    client1_2_event_callback_called.wait_for([&](int value) { return value >= 2; }, std::chrono::seconds(1));

    {
      EXPECT_EQ(server1_event_callback_called,   4);
      EXPECT_EQ(server2_event_callback_called,   2);
      EXPECT_EQ(client1_1_event_callback_called, 2);
      EXPECT_EQ(client1_2_event_callback_called, 2);
      EXPECT_EQ(client2_1_event_callback_called, 1);
      EXPECT_EQ(client2_2_event_callback_called, 1);

      EXPECT_EQ(server_manager->server_count(), 1);
      EXPECT_EQ(client_manager->client_count(), 3);
    }

    // Test what happens when the system stops all clients and servers

    server_manager->stop();
    client_manager->stop();

    server2_event_callback_called.wait_for([&](int value) { return value >= 4; }, std::chrono::seconds(1));
    client2_1_event_callback_called.wait_for([&](int value) { return value >= 2; }, std::chrono::seconds(1));
    client2_2_event_callback_called.wait_for([&](int value) { return value >= 2; }, std::chrono::seconds(1));

    {
      EXPECT_EQ(server1_event_callback_called,   4);
      EXPECT_EQ(server2_event_callback_called,   4);
      EXPECT_EQ(client1_1_event_callback_called, 2);
      EXPECT_EQ(client1_2_event_callback_called, 2);
      EXPECT_EQ(client2_1_event_callback_called, 2);
      EXPECT_EQ(client2_2_event_callback_called, 2);

      // The servers may be stopped, but they are still in the server manager
      EXPECT_EQ(server_manager->server_count(), 1);
      EXPECT_EQ(client_manager->client_count(), 3);
    }

    for (const auto& io_thread : io_threads)
    {
      io_thread->join();
    }
  }
}
#endif

#if 1
TEST(Callback, ServiceCallFromCallback) // NOLINT
{
  for (std::uint8_t protocol_version = min_protocol_version; protocol_version <= max_protocol_version; protocol_version++)
  {
    const auto io_context = std::make_shared<asio::io_context>();
    const asio::io_context::work dummy_work(*io_context);

    std::atomic<int> num_server_service_callback_called(0);
    std::atomic<int> num_client_response_callback1_called(0);
    std::atomic<int> num_client_response_callback2_called(0);

    const eCAL::service::Server::ServiceCallbackT server_service_callback
            = [&num_server_service_callback_called](const std::shared_ptr<const std::string>& /*request*/, const std::shared_ptr<std::string>& /*response*/) -> void
              {
                num_server_service_callback_called++;
              };

    const eCAL::service::Server::EventCallbackT server_event_callback
            = [](eCAL::service::ServerEventType /*event*/, const std::string& /*message*/) -> void
              {};

    const eCAL::service::ClientSession::EventCallbackT client_event_callback
            = []
              (eCAL::service::ClientEventType /*event*/, const std::string& /*message*/) -> void
              {};

    auto server = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, true, server_event_callback);

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
                            io_context->run();
                          });

    // Call service and wait a short time
    client_v1->async_call_service(std::make_shared<std::string>("1"), response_callback);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_EQ(num_server_service_callback_called, 2);
    EXPECT_EQ(num_client_response_callback1_called, 1);
    EXPECT_EQ(num_client_response_callback2_called, 1);

    // join the io_thread
    io_context->stop();
    io_thread.join();
  }
}
#endif

#if 1
TEST(Callback, SerializedServiceCallbacks) // NOLINT
{
  for (std::uint8_t protocol_version = min_protocol_version; protocol_version <= max_protocol_version; protocol_version++)
  {
    constexpr std::chrono::milliseconds server_callback_wait_time(50);
    constexpr int num_clients = 5;
    constexpr int num_threads = 5;

    const auto io_context = std::make_shared<asio::io_context>();
    auto server_manager = eCAL::service::ServerManager::create(io_context);
    auto client_manager = eCAL::service::ClientManager::create(io_context);

    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    
    for (int i = 0; i < num_clients; i++)
    {
      threads.emplace_back([&io_context]()
                          {
                            io_context->run();
                          });
    }

    atomic_signalable<int> num_server_service_callback_called  (0);
    atomic_signalable<int> num_client_response_callback_called (0);
    atomic_signalable<int> num_client_event_callback_called    (0);

    const eCAL::service::Server::ServiceCallbackT server_service_callback
            = [&num_server_service_callback_called, server_callback_wait_time]
              (const std::shared_ptr<const std::string>& request, const std::shared_ptr<std::string>& response) -> void
              {
                std::this_thread::sleep_for(server_callback_wait_time);
                *response = "Response on \"" + *request + "\"";
                num_server_service_callback_called++;
              };

    const eCAL::service::Server::EventCallbackT server_event_callback
            = []
              (eCAL::service::ServerEventType /*event*/, const std::string& /*message*/) -> void
              {};

    const eCAL::service::ClientSession::EventCallbackT client_event_callback
            = [&num_client_event_callback_called]
              (eCAL::service::ClientEventType /*event*/, const std::string& /*message*/) -> void
              {
                num_client_event_callback_called++;
              };

    auto server = server_manager->create_server(protocol_version, 0, server_service_callback, false, server_event_callback);
    std::vector<std::shared_ptr<eCAL::service::ClientSession>> clients;
    clients.reserve(num_clients);
    for (int i = 0; i < num_clients; i++)
    {
      clients.push_back(client_manager->create_client(protocol_version, "127.0.0.1", server->get_port(), client_event_callback));
    }
    
    num_client_event_callback_called.wait_for([&num_clients](int value) -> bool { return value >= num_clients; }, std::chrono::milliseconds(500));

    auto start = std::chrono::steady_clock::now();
    for (const auto& client : clients)
    {
      const auto request = std::make_shared<std::string>("Request");
      auto response      = std::make_shared<std::string>();

      auto client_response_callback = [&num_client_response_callback_called, response]
                                      (const eCAL::service::Error& error, const std::shared_ptr<std::string>& /*response*/) -> void
                                      {
                                        EXPECT_FALSE(bool(error));
                                        num_client_response_callback_called++;
                                      };

      client->async_call_service(request, client_response_callback);
    }

    num_client_response_callback_called.wait_for([num_clients](int v) {return v >= num_clients;}, num_clients * server_callback_wait_time * 2);

    auto end = std::chrono::steady_clock::now();
    auto duration = end - start;

    EXPECT_GE(duration, num_clients * server_callback_wait_time);

    server_manager->stop();
    client_manager->stop();

    // join all threads
    for (auto& thread : threads)
    {
      thread.join();
    }
  }
}
#endif

#if 1
TEST(ErrorCallback, ErrorCallbackNoServer) // NOLINT
{
  for (std::uint8_t protocol_version = min_protocol_version; protocol_version <= max_protocol_version; protocol_version++)
  {
    const auto io_context = std::make_shared<asio::io_context>();
    const asio::io_context::work dummy_work(*io_context);

    atomic_signalable<int> num_client_response_callback_called(0);
    std::atomic<int>       num_client_event_callback_called   (0);

    const eCAL::service::ClientSession::ResponseCallbackT response_callback
            = [&num_client_response_callback_called](const eCAL::service::Error& error, const std::shared_ptr<std::string>& /*response*/) -> void
              {
                EXPECT_TRUE(bool(error));
                num_client_response_callback_called++;
              };

  
    const eCAL::service::ClientSession::EventCallbackT client_event_callback
            = [&num_client_event_callback_called]
              (eCAL::service::ClientEventType /*event*/, const std::string& /*message*/) -> void
              {
                num_client_event_callback_called++;
              };


    EXPECT_EQ(num_client_response_callback_called, 0);

    auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version, "NonExistingEndpoint", 12345, client_event_callback);

    // Run the io_service
    std::thread io_thread([&io_context]()
                          {
                            io_context->run();
                          });

    // Call service and wait a short time
    client_v1->async_call_service(std::make_shared<std::string>("Hello World"), response_callback);
    
    num_client_response_callback_called.wait_for([](int v) { return v >= 1;} , std::chrono::milliseconds(5000));

    EXPECT_EQ(num_client_response_callback_called, 1);
    EXPECT_EQ(num_client_event_callback_called,    0);

    // join the io_thread
    io_context->stop();
    io_thread.join();
  }
}
#endif

#if 1
TEST(ErrorCallback, ErrorCallbackServerHasDisconnected) // NOLINT
{
  for (std::uint8_t protocol_version = min_protocol_version; protocol_version <= max_protocol_version; protocol_version++)
  {
    const auto io_context = std::make_shared<asio::io_context>();
    auto dummy_work = std::make_unique<asio::io_context::work>(*io_context);

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
              (const std::shared_ptr<const std::string>& /*request*/, const std::shared_ptr<std::string>& response) -> void
              {
                *response = "Server running!";
                num_server_service_callback_called++; 
              };

    const eCAL::service::Server::EventCallbackT server_event_callback
            = [&num_server_event_callback_called, &num_server_event_callback_called_connected, &num_server_event_callback_called_disconnected]
              (eCAL::service::ServerEventType event, const std::string& /*message*/) -> void
              {
                if (event == eCAL::service::ServerEventType::Connected)
                  num_server_event_callback_called_connected++;
                else if (event == eCAL::service::ServerEventType::Disconnected)
                  num_server_event_callback_called_disconnected++;

                num_server_event_callback_called++; 
              };


    const eCAL::service::ClientSession::EventCallbackT client_event_callback
            = [&num_client_event_callback_called, &num_client_event_callback_called_connected, &num_client_event_callback_called_disconnected]
              (eCAL::service::ClientEventType event, const std::string& /*message*/) -> void
              {
                if (event == eCAL::service::ClientEventType::Connected)
                    num_client_event_callback_called_connected++;
                else if (event == eCAL::service::ClientEventType::Disconnected)
                    num_client_event_callback_called_disconnected++;

                num_client_event_callback_called++; 
              };

    auto server = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, true, server_event_callback);
    auto client = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback);

    std::thread io_thread([&io_context]()
                          {
                            io_context->run();
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
      client->async_call_service(std::make_shared<std::string>("Everything fine?"), client_response_callback);
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

      EXPECT_EQ(client->get_state(), eCAL::service::State::CONNECTED);
      EXPECT_EQ(client->get_accepted_protocol_version(), protocol_version);
      EXPECT_EQ(client->get_queue_size(), 0);
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

      EXPECT_EQ(client->get_state(), eCAL::service::State::FAILED);
      EXPECT_EQ(client->get_accepted_protocol_version(), protocol_version);
      EXPECT_EQ(client->get_queue_size(), 0);
    }

    // Service call on the dead server.
    {
      const eCAL::service::ClientSession::ResponseCallbackT client_response_callback
            = [&num_client_response_callback_called]
              (const eCAL::service::Error& error, const std::shared_ptr<std::string>& /*response*/) -> void
              {
                EXPECT_TRUE(error);
                num_client_response_callback_called++;
              };
      client->async_call_service(std::make_shared<std::string>("Everything fine?"), client_response_callback);
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

      EXPECT_EQ(client->get_state(), eCAL::service::State::FAILED);
      EXPECT_EQ(client->get_accepted_protocol_version(), protocol_version);
      EXPECT_EQ(client->get_queue_size(), 0);
    }

    dummy_work.reset();
    client->stop();

    // join the io_thread
    io_thread.join();
  }
}
#endif

#if 1
TEST(ErrorCallback, ErrorCallbackClientDisconnects) // NOLINT
{
  for (std::uint8_t protocol_version = min_protocol_version; protocol_version <= max_protocol_version; protocol_version++)
  {
    const auto io_context = std::make_shared<asio::io_context>();
    const asio::io_context::work dummy_work(*io_context);

    std::atomic<int> num_server_service_callback_called           (0);
    std::atomic<int> num_client_response_callback_called          (0);

    const eCAL::service::Server::ServiceCallbackT server_service_callback
            = [&num_server_service_callback_called]
              (const std::shared_ptr<const std::string>& /*request*/, const std::shared_ptr<std::string>& response) -> void
              {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                *response = "Server running!";
                num_server_service_callback_called++; 
              };

    const eCAL::service::Server::EventCallbackT server_event_callback
            = []
              (eCAL::service::ServerEventType /*event*/, const std::string& /*message*/) -> void
              {};


    const eCAL::service::ClientSession::EventCallbackT client_event_callback
            = []
              (eCAL::service::ClientEventType /*event*/, const std::string& /*message*/) -> void
              {};

    auto server    = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, true, server_event_callback);
    auto client_v1 = eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback);

    std::thread io_thread([&io_context]()
                          {
                            io_context->run();
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
    io_context->stop();
    io_thread.join();
  }
}
#endif

#if 1
TEST(ErrorCallback, StressfulErrorsHalfwayThrough) // NOLINT
{
  for (std::uint8_t protocol_version = min_protocol_version; protocol_version <= max_protocol_version; protocol_version++)
  {
    constexpr int num_io_threads       = 50;
    constexpr int num_clients          = 50;
    constexpr int num_calls_per_client = 50;
    constexpr std::chrono::milliseconds server_time_to_waste(10);

    //constexpr std::chrono::milliseconds wait_time_for_destroying_server = server_time_to_waste * total_calls / 2;
    constexpr std::chrono::milliseconds wait_time_for_destroying_server = server_time_to_waste * num_calls_per_client / 2;

    const auto io_context = std::make_shared<asio::io_context>();
    const asio::io_context::work dummy_work(*io_context);

    std::atomic<int>       num_server_service_callback_called           (0);
    atomic_signalable<int> num_server_event_callback_called             (0);
    std::atomic<int>       num_server_event_callback_called_connected   (0);
    std::atomic<int>       num_server_event_callback_called_disconnected(0);

    atomic_signalable<int> num_client_response_callback_called              (0);
    std::atomic<int>       num_client_response_callback_called_with_error   (0);
    std::atomic<int>       num_client_response_callback_called_without_error(0);
    atomic_signalable<int> num_client_event_callback_called                 (0);
    std::atomic<int>       num_client_event_callback_called_connected       (0);
    std::atomic<int>       num_client_event_callback_called_disconnected    (0);

    const eCAL::service::Server::ServiceCallbackT service_callback
            = [server_time_to_waste, &num_server_service_callback_called](const std::shared_ptr<const std::string>& request, const std::shared_ptr<std::string>& response) -> void
              {
                *response = "Response on \"" + *request + "\"";
                std::this_thread::sleep_for(server_time_to_waste);
                num_server_service_callback_called++;
              };

    const eCAL::service::Server::EventCallbackT server_event_callback
            = [&num_server_event_callback_called, &num_server_event_callback_called_connected, &num_server_event_callback_called_disconnected]
              (eCAL::service::ServerEventType event, const std::string& /*message*/) -> void
              {              
                if (event == eCAL::service::ServerEventType::Connected)
                  num_server_event_callback_called_connected++;
                else if (event == eCAL::service::ServerEventType::Disconnected)
                  num_server_event_callback_called_disconnected++;

                num_server_event_callback_called++; 
              };

    auto server = eCAL::service::Server::create(io_context, protocol_version, 0, service_callback, true, server_event_callback, critical_logger("Server"));

    {
      EXPECT_EQ(num_server_service_callback_called           , 0);
      EXPECT_EQ(num_server_event_callback_called             , 0);
      EXPECT_EQ(num_server_event_callback_called_connected   , 0);
      EXPECT_EQ(num_server_event_callback_called_disconnected, 0);

      EXPECT_EQ(num_client_response_callback_called              , 0);
      EXPECT_EQ(num_client_response_callback_called_with_error   , 0);
      EXPECT_EQ(num_client_response_callback_called_without_error, 0);
      EXPECT_EQ(num_client_event_callback_called.get()           , 0);
      EXPECT_EQ(num_client_event_callback_called_connected       , 0);
      EXPECT_EQ(num_client_event_callback_called_disconnected    , 0);
    }

    // Run the io service a bunch of times, so we hopefully trigger any race condition that may exist
    std::vector<std::unique_ptr<std::thread>> io_threads;
    io_threads.reserve(num_io_threads);
    for (int i = 0; i < num_io_threads; i++)
    {
      io_threads.emplace_back(std::make_unique<std::thread>([&io_context]() { io_context->run(); }));
    }

    // Create all the clients
    std::vector<std::shared_ptr<eCAL::service::ClientSession>> client_list;
    client_list.reserve(num_clients);
    for (int c = 0; c < num_clients; c++)
    {
      const eCAL::service::ClientSession::EventCallbackT client_event_callback
            = [&num_client_event_callback_called, &num_client_event_callback_called_connected, &num_client_event_callback_called_disconnected]
              (eCAL::service::ClientEventType event, const std::string& /*message*/) -> void
              {                
                if (event == eCAL::service::ClientEventType::Connected)
                  num_client_event_callback_called_connected++;
                else if (event == eCAL::service::ClientEventType::Disconnected)
                  num_client_event_callback_called_disconnected++;

                num_client_event_callback_called++; 
              };
      client_list.push_back(eCAL::service::ClientSession::create(io_context, protocol_version,"127.0.0.1", server->get_port(), client_event_callback, critical_logger("Client " + std::to_string(c))));
    }

    // Directly run a bunch of clients and call each client a bunch of times
    for (size_t c = 0; c < client_list.size(); c++)
    {
      for (int i = 0; i < num_calls_per_client; i++)
      {
        const std::shared_ptr<std::string> request_string = std::make_shared<std::string>("Client " + std::to_string(c) + ", Call " + std::to_string(i));

        const eCAL::service::ClientSession::ResponseCallbackT response_callback
              = [&num_client_response_callback_called, &num_client_response_callback_called_with_error, &num_client_response_callback_called_without_error, request_string]
                (const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
                {
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
                  num_client_response_callback_called++;
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

      EXPECT_EQ(num_client_event_callback_called.get()       , num_clients);
      EXPECT_EQ(num_client_event_callback_called_connected   , num_clients);
      EXPECT_EQ(num_client_event_callback_called_disconnected, 0);

      EXPECT_EQ(server->get_connection_count(), num_clients);
    }

    // delete server
    server->stop();
    server = nullptr;

    num_server_event_callback_called   .wait_for([num_clients](int v) { return v >= (2 * num_clients); }, std::chrono::milliseconds(5000));
    num_client_event_callback_called   .wait_for([num_clients](int v) { return v >= (2 * num_clients); }, std::chrono::milliseconds(500));
    num_client_response_callback_called.wait_for([num_clients, num_calls_per_client](int v) { return v >= (num_clients* num_calls_per_client); }, std::chrono::milliseconds(500));

    {
      EXPECT_TRUE(num_server_service_callback_called > 0);
      EXPECT_TRUE(num_server_service_callback_called < (num_clients * num_calls_per_client));

      EXPECT_EQ(num_server_event_callback_called.get()       , num_clients * 2);
      EXPECT_EQ(num_server_event_callback_called_connected   , num_clients);
      EXPECT_EQ(num_server_event_callback_called_disconnected, num_clients);

      EXPECT_EQ(num_client_response_callback_called.get(), num_clients* num_calls_per_client);

      EXPECT_TRUE(num_client_response_callback_called_with_error    > 0);
      EXPECT_TRUE(num_client_response_callback_called_without_error > 0);

      EXPECT_EQ(num_client_event_callback_called             , num_clients * 2);
      EXPECT_EQ(num_client_event_callback_called_connected   , num_clients);
      EXPECT_EQ(num_client_event_callback_called_disconnected, num_clients);
    }

    client_list.clear();

    // join all io_threads
    io_context->stop();
    for (const auto& io_thread : io_threads)
    {
      io_thread->join();
    }
    io_threads.clear();
  }
}
#endif

#if 1
TEST(ErrorCallback, StressfulErrorsHalfwayThroughWithManagers) // NOLINT
{
  for (std::uint8_t protocol_version = min_protocol_version; protocol_version <= max_protocol_version; protocol_version++)
  {
    constexpr int num_io_threads       = 50;
    constexpr int num_clients          = 50;
    constexpr int num_calls_per_client = 50;
    constexpr std::chrono::milliseconds server_time_to_waste(10);

    constexpr std::chrono::milliseconds wait_time_for_destroying_server = server_time_to_waste * num_calls_per_client / 2;

    const auto io_context = std::make_shared<asio::io_context>();

    auto client_manager = eCAL::service::ClientManager::create(io_context, critical_logger("Client"));
    auto server_manager = eCAL::service::ServerManager::create(io_context, critical_logger("Server"));

    std::atomic<int> num_server_service_callback_called           (0);
    atomic_signalable<int> num_server_event_callback_called       (0);
    std::atomic<int> num_server_event_callback_called_connected   (0);
    std::atomic<int> num_server_event_callback_called_disconnected(0);

    atomic_signalable<int> num_client_response_callback_called        (0);
    std::atomic<int> num_client_response_callback_called_with_error   (0);
    std::atomic<int> num_client_response_callback_called_without_error(0);
    atomic_signalable<int> num_client_event_callback_called           (0);
    std::atomic<int> num_client_event_callback_called_connected       (0);
    std::atomic<int> num_client_event_callback_called_disconnected    (0);

    const eCAL::service::Server::ServiceCallbackT service_callback
            = [server_time_to_waste, &num_server_service_callback_called](const std::shared_ptr<const std::string>& request, const std::shared_ptr<std::string>& response) -> void
              {
                *response = "Response on \"" + *request + "\"";
                std::this_thread::sleep_for(server_time_to_waste);
                num_server_service_callback_called++;
              };

    const eCAL::service::Server::EventCallbackT server_event_callback
            = [&num_server_event_callback_called, &num_server_event_callback_called_connected, &num_server_event_callback_called_disconnected]
              (eCAL::service::ServerEventType event, const std::string& /*message*/) -> void
              {              
                if (event == eCAL::service::ServerEventType::Connected)
                  num_server_event_callback_called_connected++;
                else if (event == eCAL::service::ServerEventType::Disconnected)
                  num_server_event_callback_called_disconnected++;

                num_server_event_callback_called++; 
              };

    auto server = server_manager->create_server(protocol_version, 0, service_callback, true, server_event_callback);

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
      io_threads.emplace_back(std::make_unique<std::thread>([&io_context]() { io_context->run(); }));
    }

    // Create all the clients
    std::vector<std::shared_ptr<eCAL::service::ClientSession>> client_list;
    client_list.reserve(num_clients);
    for (int c = 0; c < num_clients; c++)
    {
      const eCAL::service::ClientSession::EventCallbackT client_event_callback
            = [&num_client_event_callback_called, &num_client_event_callback_called_connected, &num_client_event_callback_called_disconnected]
              (eCAL::service::ClientEventType event, const std::string& /*message*/) -> void
              {                
                if (event == eCAL::service::ClientEventType::Connected)
                  num_client_event_callback_called_connected++;
                else if (event == eCAL::service::ClientEventType::Disconnected)
                  num_client_event_callback_called_disconnected++;

                num_client_event_callback_called++; 
              };
      client_list.push_back(client_manager->create_client(protocol_version,"127.0.0.1", server->get_port(), client_event_callback));
    }

    // Directly run a bunch of clients and call each client a bunch of times
    for (size_t c = 0; c < client_list.size(); c++)
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
    server_manager->stop();
    
    num_client_event_callback_called.wait_for([num_clients](int v) { return v >= num_clients * 2; }, std::chrono::seconds(5));
    num_server_event_callback_called.wait_for([num_clients](int v) { return v >= num_clients * 2; }, std::chrono::seconds(5));
    num_client_response_callback_called.wait_for([num_clients, num_calls_per_client](int v) { return v >= num_clients * num_calls_per_client; }, std::chrono::seconds(5));

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
    client_manager->stop();

    // join all io_threads
    for (const auto& io_thread : io_threads)
    {
      io_thread->join();
    }
  }
}
#endif

#if 1
TEST(BlockingCall, RegularBlockingCall) // NOLINT
{
  for (std::uint8_t protocol_version = min_protocol_version; protocol_version <= max_protocol_version; protocol_version++)
  {
    constexpr std::chrono::milliseconds server_callback_wait_time(50);
    constexpr int num_calls = 3;

    const auto io_context = std::make_shared<asio::io_context>();
    const asio::io_context::work dummy_work(*io_context);

    std::atomic<int> num_server_service_callback_called           (0);

    const eCAL::service::Server::ServiceCallbackT server_service_callback
            = [&num_server_service_callback_called, server_callback_wait_time]
              (const std::shared_ptr<const std::string>& request, const std::shared_ptr<std::string>& response) -> void
              {
                std::this_thread::sleep_for(server_callback_wait_time);
                num_server_service_callback_called++;
                *response = "Response on \"" + *request + "\"";
              };

    const eCAL::service::Server::EventCallbackT server_event_callback
            = []
              (eCAL::service::ServerEventType /*event*/, const std::string& /*message*/) -> void
              {};

    const eCAL::service::ClientSession::EventCallbackT client_event_callback
            = []
              (eCAL::service::ClientEventType /*event*/, const std::string& /*message*/) -> void
              {};

    auto server = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, true, server_event_callback);
    auto client = eCAL::service::ClientSession::create(io_context, protocol_version, "127.0.0.1", server->get_port(), client_event_callback);

    std::thread io_thread([&io_context]()
                          {
                            io_context->run();
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
    io_context->stop();
    io_thread.join();
  }
}
#endif

#if 1
TEST(BlockingCall, BlockingCallWithErrorHalfwayThrough) // NOLINT
{
  for (std::uint8_t protocol_version = min_protocol_version; protocol_version <= max_protocol_version; protocol_version++)
  {
    constexpr std::chrono::milliseconds server_callback_wait_time(100);
    constexpr int num_calls_before_shutdown = 3;
    constexpr int num_calls_after_shutdown  = 3;

    const auto io_context = std::make_shared<asio::io_context>();
    auto       dummy_work = std::make_unique<asio::io_context::work>(*io_context);

    std::atomic<int> num_server_service_callback_called           (0);

    const eCAL::service::Server::ServiceCallbackT server_service_callback
            = [&num_server_service_callback_called, server_callback_wait_time]
              (const std::shared_ptr<const std::string>& request, const std::shared_ptr<std::string>& response) -> void
              {
                std::this_thread::sleep_for(server_callback_wait_time);
                num_server_service_callback_called++;
                *response = "Response on \"" + *request + "\"";
              };

    const eCAL::service::Server::EventCallbackT server_event_callback
            = []
              (eCAL::service::ServerEventType /*event*/, const std::string& /*message*/) -> void
              {};

    const eCAL::service::ClientSession::EventCallbackT client_event_callback
            = []
              (eCAL::service::ClientEventType /*event*/, const std::string& /*message*/) -> void
              {};

    auto server = eCAL::service::Server::create(io_context, protocol_version, 0, server_service_callback, true, server_event_callback);
    auto client = eCAL::service::ClientSession::create(io_context, protocol_version, "127.0.0.1", server->get_port(), client_event_callback);

    std::thread io_thread([&io_context]()
                          {
                            io_context->run();
                          });

    // Wait shortly for the client to connects
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Successful calls
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
                            server->stop();
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

    client->stop();
    server->stop();
    dummy_work.reset();

    // join the io_thread
    io_thread.join();

    stop_thread.join();

    // delete all objects
    client    = nullptr;
    server    = nullptr;
  }
}
#endif

#if 1
TEST(BlockingCall, Stopped)  // NOLINT // This test shows the proper way to stop everything. I should adapt all other tests, too
{
  for (std::uint8_t protocol_version = min_protocol_version; protocol_version <= max_protocol_version; protocol_version++)
  {
    constexpr std::chrono::milliseconds server_callback_wait_time(500);

    const auto io_context = std::make_shared<asio::io_context>();
    auto server_manager = eCAL::service::ServerManager::create(io_context);
    auto client_manager = eCAL::service::ClientManager::create(io_context);

    std::atomic<int> num_server_service_callback_called           (0);

    const eCAL::service::Server::ServiceCallbackT server_service_callback
                    = [&num_server_service_callback_called, server_callback_wait_time]
                      (const std::shared_ptr<const std::string>& request, const std::shared_ptr<std::string>& response) -> void
                      {
                        std::this_thread::sleep_for(server_callback_wait_time);
                        num_server_service_callback_called++;
                        *response = "Response on \"" + *request + "\"";
                      };

    const eCAL::service::Server::EventCallbackT server_event_callback
                    = []
                      (eCAL::service::ServerEventType /*event*/, const std::string& /*message*/) -> void
                      {};

    const eCAL::service::ClientSession::EventCallbackT client_event_callback
                    = []
                      (eCAL::service::ClientEventType /*event*/, const std::string& /*message*/) -> void
                      {};

    auto server = server_manager->create_server(protocol_version, 0, server_service_callback, true, server_event_callback);
    auto client = client_manager->create_client(protocol_version, "127.0.0.1", server->get_port(), client_event_callback);

    std::thread io_thread([&io_context]()
                          {
                            io_context->run();
                          });

    // Wait shortly for the client to connects
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Shutdown server in a few milliseconds (i.e. during the next call)
    std::thread stop_thread([&client_manager, &server_manager, &io_thread, server_callback_wait_time]()
                            {
                              auto sleep_time = 0.5 * server_callback_wait_time;
                              std::this_thread::sleep_for(sleep_time);

                              client_manager->stop();
                              server_manager->stop();

                              io_thread.join();

                              std::cerr << "io_context stopped" << std::endl;
                            });

    constexpr int num_calls = 5;

    std::vector<std::unique_ptr<std::thread>> call_threads;
    call_threads.reserve(num_calls);
    std::atomic<int> callbacks_completed(0);
  
    for (int i = 0; i < num_calls; i++)
    {
      call_threads.push_back(std::make_unique<std::thread>([&client, &callbacks_completed, i]()
                            {
                              const auto request = std::make_shared<std::string>("Request");
                              auto response      = std::make_shared<std::string>();
                              auto error         = client->call_service(request, response);

                              EXPECT_TRUE(bool(error));
                              callbacks_completed++;
                              std::cerr << "Callback " + std::to_string(i) + " completed\n";
                            }));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Join all call threads
    for (auto& call_thread : call_threads)
    {
      call_thread->join();
    }

    EXPECT_EQ(callbacks_completed, num_calls);

    stop_thread.join();

    // Call the client again! it must not block...
    {
      auto request  = std::make_shared<std::string>("Request");
      auto response = std::make_shared<std::string>();

      auto error = client->call_service(request, response);

      EXPECT_TRUE(bool(error));
      EXPECT_EQ(*response.get(), "");
    }
  }
}
#endif