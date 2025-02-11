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
#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <future>
#include <cmath>

#include "math.pb.h"
#include "ping.pb.h"

#include "math_service_impl.h"
#include "ping_service_impl.h"

//
// Untyped Math Service Tests
//
TEST(core_cpp_clientserver_proto, UntypedBlocking_Math)
{
  eCAL::Initialize("UntypedBlocking_Math");

  auto math_service = std::make_shared<MathServiceImpl>();
  eCAL::protobuf::CServiceServer<MathService> math_server(math_service);
  eCAL::protobuf::CServiceClient<MathService> math_client;

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  SFloatTuple request;
  request.set_inp1(10.0);
  request.set_inp2(5.0);

  auto result = math_client.CallWithResponse("Add", request);
  EXPECT_TRUE(result.first);
  ASSERT_FALSE(result.second.empty());

  SFloat response;
  response.ParseFromString(result.second[0].response);
  EXPECT_DOUBLE_EQ(response.out(), 15.0);

  eCAL::Finalize();
}

TEST(core_cpp_clientserver_proto, UntypedCallback_Math)
{
  eCAL::Initialize("UntypedCallback_Math");

  auto math_service = std::make_shared<MathServiceImpl>();
  eCAL::protobuf::CServiceServer<MathService> math_server(math_service);
  eCAL::protobuf::CServiceClient<MathService> math_client;

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  SFloatTuple request;
  request.set_inp1(6.0);
  request.set_inp2(7.0);

  std::promise<double> prom;
  auto fut = prom.get_future();

  auto callback = [&prom](const eCAL::SServiceResponse& resp)
  {
    SFloat response;
    response.ParseFromString(resp.response);
    prom.set_value(response.out());
  };

  bool initiated = math_client.CallWithCallback("Multiply", request, callback);
  EXPECT_TRUE(initiated);

  double result = fut.get();
  EXPECT_DOUBLE_EQ(result, 42.0);

  eCAL::Finalize();
}

TEST(core_cpp_clientserver_proto, UntypedCallbackAsync_Math)
{
  eCAL::Initialize("UntypedCallbackAsync_Math");

  auto math_service = std::make_shared<MathServiceImpl>();
  eCAL::protobuf::CServiceServer<MathService> math_server(math_service);
  eCAL::protobuf::CServiceClient<MathService> math_client;

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  SFloatTuple request;
  request.set_inp1(6.0);
  request.set_inp2(7.0);

  std::promise<double> prom;
  auto fut = prom.get_future();

  auto callback = [&prom](const eCAL::SServiceResponse& resp)
    {
      SFloat response;
      response.ParseFromString(resp.response);
      prom.set_value(response.out());
    };

  auto start = std::chrono::steady_clock::now();

  bool initiated = math_client.CallWithCallbackAsync("Multiply", request, callback);
  EXPECT_TRUE(initiated);

  auto async_call_end = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(async_call_end - start);
  EXPECT_LT(duration.count(), 10);

  double result = fut.get();
  EXPECT_DOUBLE_EQ(result, 42.0);

  eCAL::Finalize();
}

//
// Untyped Ping Service Tests
//
TEST(core_cpp_clientserver_proto, UntypedBlocking_Ping)
{
  eCAL::Initialize("UntypedBlocking_Ping");

  auto ping_service = std::make_shared<PingServiceImpl>();
  eCAL::protobuf::CServiceServer<PingService> ping_server(ping_service);
  eCAL::protobuf::CServiceClient<PingService> ping_client;

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  PingRequest request;
  request.set_message("PING");

  auto result = ping_client.CallWithResponse("Ping", request);
  EXPECT_TRUE(result.first);
  ASSERT_FALSE(result.second.empty());

  PingResponse response;
  response.ParseFromString(result.second[0].response);
  EXPECT_EQ(response.answer(), "PONG");

  eCAL::Finalize();
}
