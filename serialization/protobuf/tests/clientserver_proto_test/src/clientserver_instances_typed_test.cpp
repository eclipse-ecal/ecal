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

#include "math.pb.h"

#include "math_service_impl.h"

enum {
  CMN_REGISTRATION_REFRESH_MS = 1000
};

//
// Iterative Client Instance Tests (Typed Math - Blocking)
//
TEST(core_cpp_clientserver_proto, IterativeClientInstances_Typed_Math)
{
  eCAL::Initialize("IterativeClientInstances_Typed_Math");

  // Create two MathService servers to simulate multiple endpoints.
  auto math_service1 = std::make_shared<MathServiceImpl>();
  auto math_service2 = std::make_shared<MathServiceImpl>();

  eCAL::protobuf::CServiceServer<MathService> math_server1(math_service1);
  eCAL::protobuf::CServiceServer<MathService> math_server2(math_service2);

  eCAL::protobuf::CServiceClientTyped<MathService> math_client;

  std::this_thread::sleep_for(std::chrono::milliseconds(CMN_REGISTRATION_REFRESH_MS));

  auto instances = math_client.GetClientInstances();
  EXPECT_GE(instances.size(), 2);

  SFloatTuple request;
  request.set_inp1(100.0);
  request.set_inp2(50.0);

  auto result = math_client.CallWithResponse<SFloatTuple, SFloat>("Multiply", request);
  EXPECT_EQ(result.second.size(), instances.size());
  for (const auto &resp : result.second)
  {
    EXPECT_DOUBLE_EQ(resp.response.out(), 5000.0);
  }

  eCAL::Finalize();
}

//
// Iterative Client Instance Tests (Typed Math - Callback)
//
TEST(core_cpp_clientserver_proto, IterativeClientInstances_Typed_Math_Callback)
{
  eCAL::Initialize("IterativeClientInstances_Typed_Math_Callback");

  auto math_service1 = std::make_shared<MathServiceImpl>();
  auto math_service2 = std::make_shared<MathServiceImpl>();

  eCAL::protobuf::CServiceServer<MathService> math_server1(math_service1);
  eCAL::protobuf::CServiceServer<MathService> math_server2(math_service2);

  eCAL::protobuf::CServiceClientTyped<MathService> math_client;

  std::this_thread::sleep_for(std::chrono::milliseconds(CMN_REGISTRATION_REFRESH_MS));

  auto instances = math_client.GetClientInstances();
  EXPECT_GE(instances.size(), 2);

  SFloatTuple request;
  request.set_inp1(100.0);
  request.set_inp2(50.0);

  std::mutex mtx;
  std::vector<double> responses;
  size_t expected_responses = instances.size();
  std::promise<std::vector<double>> prom;
  auto fut = prom.get_future();

  auto callback = [&mtx, &responses, expected_responses, &prom](const eCAL::protobuf::SMsgServiceResponse<SFloat>& resp)
  {
    {
      std::lock_guard<std::mutex> lock(mtx);
      responses.push_back(resp.response.out());
      if (responses.size() == expected_responses)
      {
        prom.set_value(responses);
      }
    }
  };

  bool overall_success = true;
  for (auto& instance : instances)
  {
    overall_success &= instance.CallWithCallback<SFloatTuple, SFloat>("Multiply", request, callback, eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT);
  }
  EXPECT_TRUE(overall_success);

  auto all_responses = fut.get();
  EXPECT_EQ(all_responses.size(), expected_responses);
  for (const auto& value : all_responses)
  {
    EXPECT_DOUBLE_EQ(value, 5000.0);
  }

  eCAL::Finalize();
}
