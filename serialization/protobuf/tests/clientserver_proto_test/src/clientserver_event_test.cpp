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

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/client.h>
#include <ecal/msg/protobuf/server.h>

#include <gtest/gtest.h>
#include <memory>
#include <chrono>
#include <thread>

#include "atomic_signalable.h"

#include "math.pb.h"
#include "ping.pb.h"

#include "math_service_impl.h"
#include "ping_service_impl.h"

enum { CMN_REGISTRATION_REFRESH_MS = 1000 };

TEST(core_cpp_clientserver_proto, ClientConnectEvent)
{
  eCAL::Initialize("client connect event test");

  atomic_signalable<int> event_connected_fired(0);
  atomic_signalable<int> event_disconnected_fired(0);
  auto client_event_callback = [&](const eCAL::SServiceId& /*id*/, const eCAL::SClientEventCallbackData& data)
  {
    switch(data.type)
    {
      case eCAL::eClientEvent::connected:
        ++event_connected_fired;
        break;
      case eCAL::eClientEvent::disconnected:
        ++event_disconnected_fired;
        break;
      default:
        break;
    }
  };

  // Create a client with the event callback.
  eCAL::protobuf::CServiceClient<MathService> client(client_event_callback);

  std::this_thread::sleep_for(std::chrono::milliseconds(CMN_REGISTRATION_REFRESH_MS));
  EXPECT_EQ(0, event_connected_fired.get());
  EXPECT_EQ(0, event_disconnected_fired.get());

  {
    eCAL::protobuf::CServiceServer<MathService> server1(std::make_shared<MathServiceImpl>());
    event_connected_fired.wait_for([](int v){ return v >= 1; }, std::chrono::milliseconds(3 * CMN_REGISTRATION_REFRESH_MS));
    EXPECT_EQ(1, event_connected_fired.get());
    EXPECT_EQ(0, event_disconnected_fired.get());
    {
      eCAL::protobuf::CServiceServer<MathService> server2(std::make_shared<MathServiceImpl>());
      event_connected_fired.wait_for([](int v){ return v >= 2; }, std::chrono::milliseconds(3 * CMN_REGISTRATION_REFRESH_MS));
      EXPECT_EQ(2, event_connected_fired.get());
      EXPECT_EQ(0, event_disconnected_fired.get());
    }
    event_disconnected_fired.wait_for([](int v){ return v >= 1; }, std::chrono::milliseconds(3 * CMN_REGISTRATION_REFRESH_MS));
    EXPECT_EQ(1, event_disconnected_fired.get());
  }
  event_disconnected_fired.wait_for([](int v){ return v >= 2; }, std::chrono::milliseconds(3 * CMN_REGISTRATION_REFRESH_MS));
  EXPECT_EQ(2, event_disconnected_fired.get());

  eCAL::Finalize();
}

TEST(core_cpp_clientserver_proto, ServerConnectEvent)
{
  eCAL::Initialize("server connect event test");

  atomic_signalable<int> event_connected_fired(0);
  atomic_signalable<int> event_disconnected_fired(0);
  auto server_event_callback = [&](const eCAL::SServiceId& /*id*/, const eCAL::SServerEventCallbackData& data)
  {
    switch(data.type)
    {
      case eCAL::eServerEvent::connected:
        ++event_connected_fired;
        break;
      case eCAL::eServerEvent::disconnected:
        ++event_disconnected_fired;
        break;
      default:
        break;
    }
  };

  // Create a server with the event callback.
  auto ping_service = std::make_shared<PingServiceImpl>();
  eCAL::protobuf::CServiceServer<PingService> server(ping_service, server_event_callback);

  std::this_thread::sleep_for(std::chrono::milliseconds(CMN_REGISTRATION_REFRESH_MS));
  EXPECT_EQ(0, event_connected_fired.get());
  EXPECT_EQ(0, event_disconnected_fired.get());
  {
    eCAL::protobuf::CServiceClient<PingService> client1;
    event_connected_fired.wait_for([](int v){ return v >= 1; }, std::chrono::milliseconds(3 * CMN_REGISTRATION_REFRESH_MS));
    EXPECT_EQ(1, event_connected_fired.get());
    EXPECT_EQ(0, event_disconnected_fired.get());
    {
      eCAL::protobuf::CServiceClient<PingService> client2;
      event_connected_fired.wait_for([](int v){ return v >= 2; }, std::chrono::milliseconds(3 * CMN_REGISTRATION_REFRESH_MS));
      EXPECT_EQ(2, event_connected_fired.get());
      EXPECT_EQ(0, event_disconnected_fired.get());
    }
    event_disconnected_fired.wait_for([](int v){ return v >= 1; }, std::chrono::milliseconds(3 * CMN_REGISTRATION_REFRESH_MS));
    EXPECT_EQ(1, event_disconnected_fired.get());
  }
  event_disconnected_fired.wait_for([](int v){ return v >= 2; }, std::chrono::milliseconds(3 * CMN_REGISTRATION_REFRESH_MS));
  EXPECT_EQ(2, event_disconnected_fired.get());

  eCAL::Finalize();
}
