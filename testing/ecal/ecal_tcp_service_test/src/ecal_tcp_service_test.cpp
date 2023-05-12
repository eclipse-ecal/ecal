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

#include <service/asio_tcp_server.h>
#include <asio.hpp>

#include <iostream>

TEST(TcpServer, RAII)
{
  asio::io_context io_context;

  const eCAL::CAsioTcpServer::RequestCallbackT request_callback
          = [](const std::string& request, std::string& response) -> int
            {
              std::cout << "Request: " << request << std::endl;
              response = "Response";
              return 0;
            };

  const eCAL::CAsioTcpServer::EventCallbackT event_callback
          = [](eCAL_Server_Event event, const std::string& message) -> void
            {
              std::cout << "Event " << event << ": " << message << std::endl;
            };

  std::thread io_thread([&io_context]()
                        {
                          io_context.run();
                        });

  // Test auto-destruction when the shared_ptr goes out of scope

  {
    std::weak_ptr<eCAL::CAsioTcpServer> tcp_server_weak;

    {
      std::shared_ptr<eCAL::CAsioTcpServer> tcp_server = eCAL::CAsioTcpServer::create(io_context, 1, 0, request_callback, event_callback);
      tcp_server_weak = tcp_server;

      EXPECT_NE(nullptr, tcp_server);
      EXPECT_NE(nullptr, tcp_server_weak.lock());
    }

    EXPECT_EQ(nullptr, tcp_server_weak.lock());
  }

  // Test stopping the io_context while the server is running
  
  {
    std::weak_ptr<eCAL::CAsioTcpServer> tcp_server_weak;

    {
      std::shared_ptr<eCAL::CAsioTcpServer> tcp_server = eCAL::CAsioTcpServer::create(io_context, 1, 0, request_callback, event_callback);
      tcp_server_weak = tcp_server;

      io_context.stop();
      io_thread.join();

      EXPECT_NE(nullptr, tcp_server);
      EXPECT_NE(nullptr, tcp_server_weak.lock());
    }

    EXPECT_EQ(nullptr, tcp_server_weak.lock());
  }


}
