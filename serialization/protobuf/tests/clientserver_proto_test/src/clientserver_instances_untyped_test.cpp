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
#include <chrono>
#include <thread>
#include <mutex>
#include <future>

#include "math_service_impl.h"

#include "math.pb.h"

//
// Iterative Client Instance Tests (Untyped Math - Blocking)
//
TEST(core_cpp_clientserver_proto, IterativeClientInstances_Untyped_Math_Blocking)
{
  eCAL::Initialize("IterativeClientInstances_Untyped_Math_Blocking");

  // Create two MathService servers to simulate multiple endpoints.
  auto math_service1 = std::make_shared<MathServiceImpl>();
  auto math_service2 = std::make_shared<MathServiceImpl>();

  eCAL::protobuf::CServiceServer<MathService> math_server1(math_service1);
  eCAL::protobuf::CServiceServer<MathService> math_server2(math_service2);

  // Create an untyped MathService client.
  eCAL::protobuf::CServiceClient<MathService> math_client;

  // Wait for servers to register.
  std::this_thread::sleep_for(std::chrono::milliseconds(3000));

  // Get client instances (should be at least 2).
  auto instances = math_client.GetClientInstances();
  EXPECT_GE(instances.size(), 2);

  SFloatTuple request;
  request.set_inp1(100.0);
  request.set_inp2(50.0);

  // Iterate over instances and call "Multiply" on each.
  std::vector<double> outputs;
  for (auto& instance : instances)
  {
    auto ret = instance.CallWithResponse("Multiply", request, eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT);
    EXPECT_TRUE(ret.first);
    SFloat response;
    response.ParseFromString(ret.second.response);
    outputs.push_back(response.out());
  }

  // Verify all outputs equal 5000.0.
  for (const auto& val : outputs)
  {
    EXPECT_DOUBLE_EQ(val, 5000.0);
  }

  eCAL::Finalize();
}

//
// Iterative Client Instance Tests (Untyped Math - Callback)
//
TEST(core_cpp_clientserver_proto, IterativeClientInstances_Untyped_Math_Callback)
{
  eCAL::Initialize("IterativeClientInstances_Untyped_Math_Callback");

  auto math_service1 = std::make_shared<MathServiceImpl>();
  auto math_service2 = std::make_shared<MathServiceImpl>();

  eCAL::protobuf::CServiceServer<MathService> math_server1(math_service1);
  eCAL::protobuf::CServiceServer<MathService> math_server2(math_service2);

  eCAL::protobuf::CServiceClient<MathService> math_client;

  std::this_thread::sleep_for(std::chrono::milliseconds(3000));

  auto instances = math_client.GetClientInstances();
  EXPECT_GE(instances.size(), 2);

  SFloatTuple request;
  request.set_inp1(100.0);
  request.set_inp2(50.0);

  std::mutex mtx;
  std::vector<double> responses;
  size_t expected = instances.size();
  std::promise<std::vector<double>> prom;
  auto fut = prom.get_future();

  auto callback = [&mtx, &responses, expected, &prom](const eCAL::SServiceResponse& resp)
  {
    SFloat response;
    response.ParseFromString(resp.response);
    {
      std::lock_guard<std::mutex> lock(mtx);
      responses.push_back(response.out());
      if (responses.size() == expected)
      {
        prom.set_value(responses);
      }
    }
  };

  bool overall_success = true;
  for (auto& instance : instances)
  {
    overall_success &= instance.CallWithCallback("Multiply", request, callback, eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT);
  }
  EXPECT_TRUE(overall_success);

  auto all_responses = fut.get();
  EXPECT_EQ(all_responses.size(), expected);
  for (const auto& value : all_responses)
  {
    EXPECT_DOUBLE_EQ(value, 5000.0);
  }

  eCAL::Finalize();
}
