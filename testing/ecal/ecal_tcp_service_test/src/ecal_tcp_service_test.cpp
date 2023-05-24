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

#include <service/ecal_service_tcp_server.h>
#include <service/ecal_service_tcp_session_v1_client.h>

TEST(TcpServer, Communication)
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

  const eCAL::service::ClientSessionV1::ResponseCallbackT response_callback
          = [](const std::shared_ptr<std::string>& response) -> void
            {
              std::cout << "Client got Response: " << *response << std::endl;
            };

  auto server = eCAL::service::Server::create(io_context, 1, 0, service_callback, event_callback);

  EXPECT_NE(server->get_port(), 0);

  auto client_v1 = eCAL::service::ClientSessionV1::create(io_context, "127.0.0.1", server->get_port());

  // Run the io_service. The server will always have work for the io_service, so we don't need a stub object
  std::thread io_thread([&io_context]()
                        {
                          io_context.run();
                          std::cout << "==================== IoContext is exiting" << std::endl;
                        });

  // Wait a short time for the client to connect
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  
  client_v1->send_service_reqest(std::make_shared<std::string>("Hello World"), response_callback);

  // Wait a short time for the server to respond
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  // delete all objects
  server = nullptr;
  client_v1 = nullptr;

  // join the io_thread
  io_context.stop();
  io_thread.join();

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}








//TEST(TcpServer, RAII)
//{
//  asio::io_context io_context;
//
//  const eCAL::CAsioTcpServer::RequestCallbackT request_callback
//          = [](const std::string& request, std::string& response) -> int
//            {
//              std::cout << "Request: " << request << std::endl;
//              response = "Response";
//              return 0;
//            };
//
//  const eCAL::CAsioTcpServer::EventCallbackT event_callback
//          = [](eCAL_Server_Event event, const std::string& message) -> void
//            {
//              std::cout << "Event " << event << ": " << message << std::endl;
//            };
//
//  std::thread io_thread([&io_context]()
//                        {
//                          io_context.run();
//                        });
//
//  // Test auto-destruction when the shared_ptr goes out of scope
//
//  {
//    std::weak_ptr<eCAL::CAsioTcpServer> tcp_server_weak;
//
//    {
//      std::shared_ptr<eCAL::CAsioTcpServer> tcp_server = eCAL::CAsioTcpServer::create(io_context, 1, 0, request_callback, event_callback);
//      tcp_server_weak = tcp_server;
//
//      EXPECT_NE(nullptr, tcp_server);
//      EXPECT_NE(nullptr, tcp_server_weak.lock());
//    }
//
//    EXPECT_EQ(nullptr, tcp_server_weak.lock());
//  }
//
//  // Test stopping the io_context while the server is running
//  
//  {
//    std::weak_ptr<eCAL::CAsioTcpServer> tcp_server_weak;
//
//    {
//      std::shared_ptr<eCAL::CAsioTcpServer> tcp_server = eCAL::CAsioTcpServer::create(io_context, 1, 0, request_callback, event_callback);
//      tcp_server_weak = tcp_server;
//
//      io_context.stop();
//      io_thread.join();
//
//      EXPECT_NE(nullptr, tcp_server);
//      EXPECT_NE(nullptr, tcp_server_weak.lock());
//    }
//
//    EXPECT_EQ(nullptr, tcp_server_weak.lock());
//  }
//
//
//}
