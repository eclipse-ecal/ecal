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

//#include <service/asio_tcp_server.h>
#include <asio.hpp>

#include <iostream>
#include <atomic>

#include <service/ecal_service_tcp_server.h>
#include <service/ecal_service_tcp_session_v1_client.h>

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

#if 1
TEST(RAII, TcpServiceServer)
{
  asio::io_context io_context;

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
      std::shared_ptr<eCAL::service::Server> tcp_server = eCAL::service::Server::create(io_context, 1, 0, service_callback, event_callback);
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

  int todo_remove = 1;
  // Test stopping the io_context while the server is running
  //
  //{
  //  std::weak_ptr<eCAL::CAsioTcpServer> tcp_server_weak;

  //  {
  //    std::shared_ptr<eCAL::CAsioTcpServer> tcp_server = eCAL::CAsioTcpServer::create(io_context, 1, 0, request_callback, event_callback);
  //    tcp_server_weak = tcp_server;

  //    io_context.stop();
  //    io_thread.join();

  //    EXPECT_NE(nullptr, tcp_server);
  //    EXPECT_NE(nullptr, tcp_server_weak.lock());
  //  }

  //  EXPECT_EQ(nullptr, tcp_server_weak.lock());
  //}


}
#endif

#if 1
TEST(RAII, TcpServiceServerAndClient)
{
  asio::io_context io_context;

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

  const eCAL::service::ClientSessionBase::ResponseCallbackT client_slow_response_callback
            = [](const eCAL::service::Error& /*error*/, const std::shared_ptr<std::string>& /*response*/) -> void
              {
                // This callback just wastes some time
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
              };

  const eCAL::service::ClientSessionBase::EventCallbackT client_event_callback
          = []
            (eCAL_Client_Event event, const std::string& message) -> void
            {};


  std::unique_ptr<std::thread> io_thread;

  // Test auto-destruction when the shared_ptr goes out of scope

  {
    std::weak_ptr<eCAL::service::Server>          tcp_server_weak;
    std::weak_ptr<eCAL::service::ClientSessionV1> tcp_client_weak;

    {
      std::shared_ptr<eCAL::service::Server> tcp_server = eCAL::service::Server::create(io_context, 1, 0, server_service_callback, server_event_callback);
      tcp_server_weak = tcp_server;

      io_thread = std::make_unique<std::thread>([&io_context]()
                                                {
                                                  io_context.run();
                                                });

      auto client_v1 = eCAL::service::ClientSessionV1::create(io_context, "127.0.0.1", tcp_server->get_port(), client_event_callback);
      tcp_client_weak = client_v1;

      client_v1->async_call_service(std::make_shared<std::string>("Hello World"), client_slow_response_callback);
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // The server should be deleted already
    EXPECT_EQ(nullptr, tcp_server_weak.lock());

    // The client should not be deleted, as the callback should keep the reference up
    EXPECT_NE(nullptr, tcp_client_weak.lock());

    // The callback should be finished by now and the session should be deleted
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(nullptr, tcp_client_weak.lock());
  }

  io_context.stop();
  io_thread->join();
}
#endif

#if 1
TEST(RAII, StopDuringServiceCall)
{
  asio::io_context io_context;

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

  const eCAL::service::ClientSessionBase::ResponseCallbackT client_slow_response_callback
            = [](const eCAL::service::Error /*error*/, const std::shared_ptr<std::string>& /*response*/) -> void
              {
                // This callback just wastes some time
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
              };

  const eCAL::service::ClientSessionBase::EventCallbackT client_event_callback
          = []
            (eCAL_Client_Event event, const std::string& message) -> void
            {};

  std::unique_ptr<std::thread> io_thread;

  // Test auto-destruction when the shared_ptr goes out of scope

  {
    std::weak_ptr<eCAL::service::Server>          tcp_server_weak;
    std::weak_ptr<eCAL::service::ClientSessionV1> tcp_client_weak;

    {
      std::shared_ptr<eCAL::service::Server> tcp_server = eCAL::service::Server::create(io_context, 1, 0, server_service_callback, server_event_callback);
      tcp_server_weak = tcp_server;

      io_thread = std::make_unique<std::thread>([&io_context]()
                                                {
                                                  io_context.run();
                                                });

      auto client_v1 = eCAL::service::ClientSessionV1::create(io_context, "127.0.0.1", tcp_server->get_port(), client_event_callback);
      tcp_client_weak = client_v1;

      client_v1->async_call_service(std::make_shared<std::string>("Hello World"), client_slow_response_callback);
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    io_context.stop();

    // The server should be deleted already
    EXPECT_EQ(nullptr, tcp_server_weak.lock());

    // The client should not be deleted, as the callback should keep the reference up
    EXPECT_NE(nullptr, tcp_client_weak.lock());

    // The callback should be finished by now and the session should be deleted
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(nullptr, tcp_client_weak.lock());
  }

  io_thread->join();
}
#endif

#if 1
TEST(Communication, SlowCommunication)
{
  asio::io_context io_context;

  std::atomic<int> num_server_service_callback_called             (0);
  std::atomic<int> num_server_event_callback_called               (0);
  std::atomic<int> num_server_event_callback_called_connected     (0);
  std::atomic<int> num_server_event_callback_called_disconnected  (0);
  
  std::atomic<int> num_client_response_callback_called            (0);
  std::atomic<int> num_client_service_callback_called             (0);
  std::atomic<int> num_client_service_callback_called_connected   (0);
  std::atomic<int> num_client_service_callback_called_disconnected(0);

  const eCAL::service::Server::ServiceCallbackT service_callback
          = [&num_server_service_callback_called]
            (const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              num_server_service_callback_called++;
              std::cout << "Server got request: " << *request << std::endl;
              *response = "Response on \"" + *request + "\"";
              return 0;
            };

  const eCAL::service::Server::EventCallbackT event_callback
          = [&num_server_event_callback_called, &num_server_event_callback_called_connected, &num_server_event_callback_called_disconnected]
            (eCAL_Server_Event event, const std::string& message) -> void
            {
              num_server_event_callback_called++; 
              
              if (event == eCAL_Server_Event::server_event_connected)
                num_server_event_callback_called_connected++;
              else if (event == eCAL_Server_Event::server_event_disconnected)
                num_server_event_callback_called_disconnected++;
            };

  const eCAL::service::ClientSessionBase::ResponseCallbackT client_response_callback
          = [&num_client_response_callback_called]
            (const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
            {
              EXPECT_FALSE(bool(error));
              num_client_response_callback_called++;
              std::cout << "Client got Response: " << *response << std::endl;
            };

  const eCAL::service::ClientSessionBase::EventCallbackT client_event_callback
          = [&num_client_service_callback_called, &num_client_service_callback_called_connected, &num_client_service_callback_called_disconnected]
            (eCAL_Client_Event event, const std::string& message) -> void
            {
              num_client_service_callback_called++; 
                
              if (event == eCAL_Client_Event::client_event_connected)
                  num_client_service_callback_called_connected++;
              else if (event == eCAL_Client_Event::client_event_disconnected)
                  num_client_service_callback_called_disconnected++;
            };

  auto server = eCAL::service::Server::create(io_context, 1, 0, service_callback, event_callback);

  EXPECT_NE(server->get_port(), 0);

  {
    EXPECT_EQ(num_server_service_callback_called             , 0);
    EXPECT_EQ(num_server_event_callback_called               , 0);
    EXPECT_EQ(num_server_event_callback_called_connected     , 0);
    EXPECT_EQ(num_server_event_callback_called_disconnected  , 0);

    EXPECT_EQ(num_client_response_callback_called            , 0);
    EXPECT_EQ(num_client_service_callback_called             , 0);
    EXPECT_EQ(num_client_service_callback_called_connected   , 0);
    EXPECT_EQ(num_client_service_callback_called_disconnected, 0);
  }

  auto client_v1 = eCAL::service::ClientSessionV1::create(io_context, "127.0.0.1", server->get_port(), client_event_callback);

  // Run the io_service. The server will always have work for the io_service, so we don't need a stub object
  std::thread io_thread([&io_context]()
                        {
                          io_context.run();
                        });

  // Wait a short time for the client to connect
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called             , 0);
    EXPECT_EQ(num_server_event_callback_called               , 1);
    EXPECT_EQ(num_server_event_callback_called_connected     , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected  , 0);

    EXPECT_EQ(num_client_response_callback_called            , 0);
    EXPECT_EQ(num_client_service_callback_called             , 1);
    EXPECT_EQ(num_client_service_callback_called_connected   , 1);
    EXPECT_EQ(num_client_service_callback_called_disconnected, 0);
  }

  
  // Call service and wait a short time
  client_v1->async_call_service(std::make_shared<std::string>("Hello World"), client_response_callback);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called             , 1);
    EXPECT_EQ(num_server_event_callback_called               , 1);
    EXPECT_EQ(num_server_event_callback_called_connected     , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected  , 0);

    EXPECT_EQ(num_client_response_callback_called            , 1);
    EXPECT_EQ(num_client_service_callback_called             , 1);
    EXPECT_EQ(num_client_service_callback_called_connected   , 1);
    EXPECT_EQ(num_client_service_callback_called_disconnected, 0);
  }

  // Call service again and wait a short time
  client_v1->async_call_service(std::make_shared<std::string>("Called again"), client_response_callback);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    EXPECT_EQ(num_server_service_callback_called             , 2);
    EXPECT_EQ(num_server_event_callback_called               , 1);
    EXPECT_EQ(num_server_event_callback_called_connected     , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected  , 0);

    EXPECT_EQ(num_client_response_callback_called            , 2);
    EXPECT_EQ(num_client_service_callback_called             , 1);
    EXPECT_EQ(num_client_service_callback_called_connected   , 1);
    EXPECT_EQ(num_client_service_callback_called_disconnected, 0);
  }

  // delete all objects
  client_v1 = nullptr;

  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  {
    EXPECT_EQ(num_server_service_callback_called             , 2);
    EXPECT_EQ(num_server_event_callback_called               , 2);
    EXPECT_EQ(num_server_event_callback_called_connected     , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected  , 1);

    EXPECT_EQ(num_client_response_callback_called            , 2);
    EXPECT_EQ(num_client_service_callback_called             , 2);
    EXPECT_EQ(num_client_service_callback_called_connected   , 1);
    EXPECT_EQ(num_client_service_callback_called_disconnected, 1);
  }

  server = nullptr;

  {
    EXPECT_EQ(num_server_service_callback_called             , 1);
    EXPECT_EQ(num_server_event_callback_called               , 2);
    EXPECT_EQ(num_server_event_callback_called_connected     , 1);
    EXPECT_EQ(num_server_event_callback_called_disconnected  , 1);

    EXPECT_EQ(num_client_response_callback_called            , 2);
    EXPECT_EQ(num_client_service_callback_called             , 2);
    EXPECT_EQ(num_client_service_callback_called_connected   , 1);
    EXPECT_EQ(num_client_service_callback_called_disconnected, 1);
  }

  // join the io_thread
  io_context.stop();
  io_thread.join();
}
#endif

#if 0
TEST(Communication, StressfulCommunication)
{
  constexpr int num_io_threads       = 10;
  constexpr int num_clients          = 10;
  constexpr int num_calls_per_client = 15;

  asio::io_context io_context;

  // TODO: test event callback
  std::atomic<int> num_server_service_callback_called(0);
  std::atomic<int> num_client_response_callback_called_noerror(0);
  std::atomic<int> num_client_response_callback_called_witherror(0);

  const eCAL::service::Server::ServiceCallbackT service_callback
          = [&num_server_service_callback_called](const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              num_server_service_callback_called++;
              *response = "Response on \"" + *request + "\"";
              return 0;
            };

  const eCAL::service::Server::EventCallbackT event_callback
          = [](eCAL_Server_Event event, const std::string& message) -> void
            {
              // TODO: include this in the test
            };

  auto server = eCAL::service::Server::create(io_context, 1, 0, service_callback, event_callback, critical_logger("Server"));

  EXPECT_EQ(num_server_service_callback_called, 0);
  EXPECT_EQ(num_client_response_callback_called_noerror, 0);
  EXPECT_EQ(num_client_response_callback_called_witherror, 0);


  // Run the io service a bunch of times, so we hopefully trigger any race condition that may exist
  std::vector<std::unique_ptr<std::thread>> io_threads;
  io_threads.reserve(num_io_threads);
  for (int i = 0; i < num_io_threads; i++)
  {
    io_threads.emplace_back(std::make_unique<std::thread>([&io_context]() { io_context.run(); }));
  }

  // Create all the clients
  std::vector<std::shared_ptr<eCAL::service::ClientSessionV1>> client_list;
  client_list.reserve(num_clients);
  for (int c = 0; c < num_clients; c++)
  {
    client_list.push_back(eCAL::service::ClientSessionV1::create(io_context, "127.0.0.1", server->get_port(), critical_logger("Client " + std::to_string(c))));
  }

  // Directly run a bunch of clients and call each client a bunch of times
  for (int c = 0; c < client_list.size(); c++)
  {
    for (int i = 0; i < num_calls_per_client; i++)
    {
      const std::shared_ptr<std::string> request_string = std::make_shared<std::string>("Client " + std::to_string(c) + ", Call " + std::to_string(i));

      const eCAL::service::ClientSessionBase::ResponseCallbackT response_callback
            = [&num_client_response_callback_called_noerror, &num_client_response_callback_called_witherror, request_string]
              (const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
              {
                if (error)
                {
                  std::cerr << "Client received error: " << error.ToString();
                  num_client_response_callback_called_witherror++;
                }
                else
                {
                  num_client_response_callback_called_noerror++;
                }
                const std::string expected_response = "Response on \"" + *request_string + "\"";
                ASSERT_EQ(*response, expected_response);
              };

      client_list[c]->async_call_service(request_string, response_callback);
    }
  }

  // Now wait a short time for the clients to connect to the server and all requests getting executed
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  EXPECT_EQ(num_server_service_callback_called,  num_clients * num_calls_per_client);
  EXPECT_EQ(num_client_response_callback_called_noerror, num_clients * num_calls_per_client);
  EXPECT_EQ(num_client_response_callback_called_witherror, 0);

  // delete all objects
  server = nullptr;
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

  // TODO: test event callback
  std::atomic<int> num_server_service_callback_called(0);
  std::atomic<int> num_client_response_callback1_called(0);
  std::atomic<int> num_client_response_callback2_called(0);

  const eCAL::service::Server::ServiceCallbackT service_callback
          = [&num_server_service_callback_called](const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              num_server_service_callback_called++;
              return 0;
            };

  const eCAL::service::Server::EventCallbackT event_callback
          = [](eCAL_Server_Event event, const std::string& message) -> void
            {
              std::cout << "Event " << event << ": " << message << std::endl;
            };

  auto server = eCAL::service::Server::create(io_context, 1, 0, service_callback, event_callback);

  EXPECT_EQ(num_server_service_callback_called, 0);
  EXPECT_EQ(num_client_response_callback1_called, 0);
  EXPECT_EQ(num_client_response_callback2_called, 0);

  auto client_v1 = eCAL::service::ClientSessionV1::create(io_context, "127.0.0.1", server->get_port());

  const eCAL::service::ClientSessionV1::ResponseCallbackT response_callback
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


  // Run the io_service. The server will always have work for the io_service, so we don't need a stub object
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
TEST(Communication, ErrorCallback)
{
  asio::io_context io_context;

  // TODO: test event callback
  std::atomic<int> num_server_service_callback_called(0);
  std::atomic<int> num_client_response_callback_called(0);

  const eCAL::service::Server::ServiceCallbackT service_callback
          = [&num_server_service_callback_called](const std::shared_ptr<std::string>& request, const std::shared_ptr<std::string>& response) -> int
            {
              num_server_service_callback_called++;
              return 0;
            };

  const eCAL::service::Server::EventCallbackT event_callback
          = [](eCAL_Server_Event event, const std::string& message) -> void
            {
            };

  const eCAL::service::ClientSessionV1::ResponseCallbackT response_callback
          = [&num_client_response_callback_called](const eCAL::service::Error& error, const std::shared_ptr<std::string>& response) -> void
            {
              EXPECT_TRUE(bool(error));
              num_client_response_callback_called++;
            };

  auto server = eCAL::service::Server::create(io_context, 1, 0, service_callback, event_callback);

  EXPECT_NE(server->get_port(), 0);
  EXPECT_EQ(num_server_service_callback_called, 0);
  EXPECT_EQ(num_client_response_callback_called, 0);

  auto client_v1 = eCAL::service::ClientSessionV1::create(io_context, "NonExistingEndpoint", 12345);

  // Run the io_service. The server will always have work for the io_service, so we don't need a stub object
  std::thread io_thread([&io_context]()
                        {
                          io_context.run();
                        });

  // Call service and wait a short time
  client_v1->async_call_service(std::make_shared<std::string>("Hello World"), response_callback);
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  EXPECT_EQ(num_server_service_callback_called, 0);
  EXPECT_EQ(num_client_response_callback_called, 1);

  // delete all objects
  server = nullptr;
  client_v1 = nullptr;

  // join the io_thread
  io_context.stop();
  io_thread.join();
}
#endif


