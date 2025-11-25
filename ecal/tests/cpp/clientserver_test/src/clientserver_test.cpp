/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 * Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "atomic_signalable.h"

//TODO: Enable all tests again
#define ClientConnectEventTest                        0
#define ServerConnectEventTest                        0

#define ClientServerBaseCallbackTest                  0
#define ClientServerBaseCallbackTimeoutTest           0

#define ClientServerCallWithResponseTimeBehaviorTest  0
#define ClientServerCallWithCallbackTimeBehaviorTest  0

#define ClientServerBaseAsyncCallbackTest             0
#define ClientServerBaseAsyncTest                     0

#define NestedRPCCallTest                             0
#define NestedBlockingCallFromServerCallbackTest      1

#define ClientServerBaseBlockingTest                  0

#define DO_LOGGING                                    1

enum {
  CMN_REGISTRATION_REFRESH_MS = 1000
};

namespace
{
  typedef std::vector<std::shared_ptr<eCAL::CServiceServer>> ServiceVecT;
  typedef std::vector<std::shared_ptr<eCAL::CServiceClient>> ClientVecT;

#if DO_LOGGING
  void PrintRequest(const eCAL::SServiceMethodInformation& method_info_, const std::string& request_)
  {
    std::cout << "------ REQUEST -------" << std::endl;
    std::cout << "Method name                    : " << method_info_.method_name        << std::endl;
    std::cout << "Method request  type name      : " << method_info_.request_type.name      << std::endl;
    std::cout << "Method request  type encoding  : " << method_info_.request_type.encoding  << std::endl;
    std::cout << "Method response type name      : " << method_info_.response_type.name     << std::endl;
    std::cout << "Method response type encoding  : " << method_info_.response_type.encoding << std::endl;
    std::cout << "Method request                 : " << request_                        << std::endl;
    std::cout << std::endl;
  }
#else
  void PrintRequest(const eCAL::SServiceMethodInformation& /*method_info_*/, const std::string& /*request_*/)
  {
  }
#endif

#if DO_LOGGING
  void PrintResponse(const struct eCAL::SServiceResponse& service_response_)
  {
    std::cout << "------ RESPONSE ------" << std::endl;
    std::cout << "Executed on host name : " << service_response_.server_id.service_id.host_name         << std::endl;
    std::cout << "Executed service name : " << service_response_.server_id.service_name                 << std::endl;
    std::cout << "Executed method name  : " << service_response_.service_method_information.method_name << std::endl;
    std::cout << "Return value          : " << service_response_.ret_state                              << std::endl;
    std::cout << "Execution state       : ";
    switch (service_response_.call_state)
    {
    case eCAL::eCallState::none:
      std::cout << "call_state_none";
      break;
    case eCAL::eCallState::executed:
      std::cout << "call_state_executed";
      break;
    case eCAL::eCallState::failed:
      std::cout << "call_state_failed";
      break;
    }
    std::cout << std::endl;
    std::cout << "Execution error msg   : " << service_response_.error_msg << std::endl;
    std::cout << "Response              : " << service_response_.response  << std::endl  << std::endl;
    std::cout << std::endl;
  }
#else
  void PrintResponse(const struct eCAL::SServiceResponse& /*service_response_*/)
  {
  }
#endif
}

#if ClientConnectEventTest

TEST(core_cpp_clientserver, ClientConnectEvent)
{
  // initialize eCAL API
  eCAL::Initialize("clientserver base connect event callback");

  // client event callback for connect events
  atomic_signalable<int> event_connected_fired   (0);
  atomic_signalable<int> event_disconnected_fired(0);
  auto event_callback = [&](const eCAL::SServiceId& /*service_id_*/, const struct eCAL::SClientEventCallbackData& data_)
    {
      switch (data_.type)
      {
      case eCAL::eClientEvent::connected:
#if DO_LOGGING
        std::cout << "event connected fired" << std::endl;
#endif
        event_connected_fired++;
        break;
      case eCAL::eClientEvent::disconnected:
#if DO_LOGGING
        std::cout << "event disconnected fired" << std::endl;
#endif
        event_disconnected_fired++;
        break;
      default:
        break;
      }
    };

  // create client
  eCAL::CServiceClient client("service", eCAL::ServiceMethodInformationSetT(), event_callback);

  // check events
  eCAL::Process::SleepMS(CMN_REGISTRATION_REFRESH_MS);
  EXPECT_EQ(0, event_connected_fired.get());
  EXPECT_EQ(0, event_disconnected_fired.get());

  // create server
  {
    eCAL::CServiceServer server1("service");

    event_connected_fired.wait_for([](int v) { return v >= 1; }, std::chrono::milliseconds(3 * CMN_REGISTRATION_REFRESH_MS));
    EXPECT_EQ(1, event_connected_fired.get());
    EXPECT_EQ(0, event_disconnected_fired.get());

    eCAL::CServiceServer server2("service");

    event_connected_fired.wait_for([](int v) { return v >= 2; }, std::chrono::milliseconds(3 * CMN_REGISTRATION_REFRESH_MS));
    EXPECT_EQ(2, event_connected_fired.get());
    EXPECT_EQ(0, event_disconnected_fired.get());
  }

  // wait for disconnection
  event_disconnected_fired.wait_for([](int v) { return v >= 2; }, std::chrono::milliseconds(3 * CMN_REGISTRATION_REFRESH_MS));
  EXPECT_EQ(2, event_connected_fired.get());
  EXPECT_EQ(2, event_disconnected_fired.get());

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* ClientConnectEventTest */

#if ServerConnectEventTest

TEST(core_cpp_clientserver, ServerConnectEvent)
{
  // initialize eCAL API
  eCAL::Initialize("clientserver base connect event callback");

  // server event callback for connect events
  atomic_signalable<int> event_connected_fired   (0);
  atomic_signalable<int> event_disconnected_fired(0);
  auto event_callback = [&](const eCAL::SServiceId& /*service_id_*/, const struct eCAL::SServerEventCallbackData& data_) -> void
    {
      switch (data_.type)
      {
      case eCAL::eServerEvent::connected:
#if DO_LOGGING
        std::cout << "event connected fired" << std::endl;
#endif
        event_connected_fired++;
        break;
      case eCAL::eServerEvent::disconnected:
#if DO_LOGGING
        std::cout << "event disconnected fired" << std::endl;
#endif
        event_disconnected_fired++;
        break;
      default:
        break;
      }
    };

  // create server
  eCAL::CServiceServer server("service", event_callback);

  // check events
  eCAL::Process::SleepMS(CMN_REGISTRATION_REFRESH_MS);
  EXPECT_EQ(0, event_connected_fired.get());
  EXPECT_EQ(0, event_disconnected_fired.get());

  // create clients
  {
    eCAL::CServiceClient client1("service");

    // one client connected, no client disconnected
    event_connected_fired.wait_for([](int v) { return v >= 1; }, std::chrono::milliseconds(3 * CMN_REGISTRATION_REFRESH_MS));
    EXPECT_EQ(1, event_connected_fired.get());
    EXPECT_EQ(0, event_disconnected_fired.get());

    eCAL::CServiceClient client2("service");

    // two clients connected, no client disconnected
    event_disconnected_fired.wait_for([](int v) { return v >= 2; }, std::chrono::milliseconds(3 * CMN_REGISTRATION_REFRESH_MS));
    EXPECT_EQ(2, event_connected_fired.get());
    EXPECT_EQ(0, event_disconnected_fired.get());
  }

  // two clients connected, two clients disconnected
  event_disconnected_fired.wait_for([](int v) { return v >= 2; }, std::chrono::milliseconds(3 * CMN_REGISTRATION_REFRESH_MS));
  EXPECT_EQ(2, event_connected_fired.get());
  EXPECT_EQ(2, event_disconnected_fired.get());

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* ServerConnectEventTest */

#if ClientServerBaseCallbackTest

TEST(core_cpp_clientserver, ClientServerBaseCallback)
{
  const int num_services(2);
  const int num_clients(3);
  const int calls(5);

  // initialize eCAL API
  eCAL::Initialize("clientserver base callback test");

  // create service servers
  ServiceVecT service_vec;
  for (auto s = 0; s < num_services; ++s)
  {
    service_vec.push_back(std::make_shared<eCAL::CServiceServer>("service"));
  }

  // method callback function
  std::atomic<int> methods_executed(0);
  std::atomic<int> method_process_time(0);
  auto method_callback = [&](const eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_) -> int
    {
      eCAL::Process::SleepMS(method_process_time);
      PrintRequest(method_info_, request_);
      response_ = "I answer on " + request_;
      methods_executed++;
      return 42;
    };

  // add method callbacks
  for (const auto& service : service_vec)
  {
    eCAL::SServiceMethodInformation method1_info{"foo::method1", {"foo::req_type1", "", ""}, {"foo::resp_type1", "", ""}};
    eCAL::SServiceMethodInformation method2_info{"foo::method2", {"foo::req_type2", "", ""}, {"foo::resp_type2", "", ""} };
    service->SetMethodCallback(method1_info, method_callback);
    service->SetMethodCallback(method2_info, method_callback);
  }

  // create service clients
  ClientVecT client_vec;
  for (auto s = 0; s < num_clients; ++s)
  {
    client_vec.push_back(std::make_shared<eCAL::CServiceClient>("service"));
  }

  // response callback function
  std::atomic<int> responses_executed(0);
  auto response_callback = [&](const struct eCAL::SServiceResponse& service_response_)
    {
      PrintResponse(service_response_);
      responses_executed++;
    };

  // let's match them -> wait REGISTRATION_REFRESH_CYCLE (ecal_def.h)
  eCAL::Process::SleepMS(2000);

  // call service
  std::atomic<int> methods_called(0);
  bool success(true);

  // some calls with no sleep in the method callback
  for (auto i = 0; i < calls; ++i)
  {
    // call methods
    for (const auto& client : client_vec)
    {
      // call method 1
      success &= client->CallWithCallback("foo::method1", "my request for method 1", response_callback);
      methods_called++;

      // call method 2
      success &= client->CallWithCallback("foo::method2", "my request for method 2", response_callback);
      methods_called++;
    }
  }

  // some calls with service_callback_time_ms sleep in the method callback
  method_process_time = 100;
  for (auto i = 0; i < calls; ++i)
  {
    // call methods
    for (const auto& client : client_vec)
    {
      // call method 1
      success &= client->CallWithCallback("foo::method1", "my request for method 1", response_callback);
      methods_called++;

      // call method 2
      success &= client->CallWithCallback("foo::method2", "my request for method 2", response_callback);
      methods_called++;
    }
  }

  EXPECT_EQ(true, success);
  EXPECT_EQ(methods_called * num_services, methods_executed);
  EXPECT_EQ(methods_called * num_services, responses_executed);


  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* ClientServerBaseCallbackTest */

#if ClientServerBaseCallbackTimeoutTest

TEST(core_cpp_clientserver, ClientServerBaseCallbackTimeout)
{
  const int num_services(2);
  const int num_clients(3);
  const int calls(1);

  // initialize eCAL API
  eCAL::Initialize("clientserver base callback test with timeout");

  // create service servers
  ServiceVecT service_vec;
  for (auto s = 0; s < num_services; ++s)
  {
    service_vec.push_back(std::make_shared<eCAL::CServiceServer>("service"));
  }

  // method callback function
  std::atomic<int> methods_executed(0);
  std::atomic<int> method_process_time(0);
  auto method_callback = [&](const eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_) -> int
    {
      eCAL::Process::SleepMS(method_process_time);
      PrintRequest(method_info_, request_);
      response_ = "I answer on " + request_;
      methods_executed++;
      return 42;
    };

  // add method callbacks
  for (const auto& service : service_vec)
  {
    eCAL::SServiceMethodInformation method1_info{ "foo::method1", {"foo::req_type1", "", ""}, {"foo::resp_type1", "", ""} };
    eCAL::SServiceMethodInformation method2_info{ "foo::method2", {"foo::req_type2", "", ""}, {"foo::resp_type2", "", ""} };
    service->SetMethodCallback(method1_info, method_callback);
    service->SetMethodCallback(method2_info, method_callback);
  }

  // create service clients
  ClientVecT client_vec;
  for (auto s = 0; s < num_clients; ++s)
  {
    client_vec.push_back(std::make_shared<eCAL::CServiceClient>("service", eCAL::ServiceMethodInformationSetT()));
  }

  // response callback function
  std::atomic<int> call_executed_sucessfully(0);
  std::atomic<int> call_returned_timeout(0);
  auto response_callback = [&](const struct eCAL::SServiceResponse& service_response_)
                          {
                            if (service_response_.call_state == eCAL::eCallState::timeouted)
                            {
#if DO_LOGGING
                              std::cout << "response timeouted fired" << std::endl;
#endif
                              call_returned_timeout++;
                            }
                            else
                            {
                              PrintResponse(service_response_);
                              call_executed_sucessfully++;
                            }
                          };

  // let's match them -> wait REGISTRATION_REFRESH_CYCLE (ecal_def.h)
  eCAL::Process::SleepMS(2000);

  // call service
  std::atomic<int> methods_called(0);
  bool success(true);

  // some calls with service_callback_time_ms sleep in the method callback
  method_process_time = 100;
  for (auto i = 0; i < calls; ++i)
  {
    // call methods
    for (const auto& client : client_vec)
    {
      // call method 1
      success &= client->CallWithCallback("foo::method1", "my request for method 1", response_callback);
      methods_called++;

      // call method 2
      success &= client->CallWithCallback("foo::method2", "my request for method 2", response_callback);
      methods_called++;
    }
  }

  EXPECT_EQ(true, success);
  EXPECT_EQ(methods_called * num_services, methods_executed);
  EXPECT_EQ(methods_called * num_services, call_executed_sucessfully);
  EXPECT_EQ(0, call_returned_timeout);

  // reset all
  success = true;
  methods_called = 0;
  methods_executed = 0;
  call_executed_sucessfully = 0;
  call_returned_timeout = 0;

  // some calls with service_callback_time_ms sleep in the method callback and a proper timeout parameter
  method_process_time = 50;
  for (auto i = 0; i < calls; ++i)
  {
    // call methods
    for (const auto& client : client_vec)
    {
      // call method 1
      success &= client->CallWithCallback("foo::method1", "my request for method 1", response_callback, method_process_time * 4);
      methods_called++;

      // call method 2
      success &= client->CallWithCallback("foo::method2", "my request for method 2", response_callback, method_process_time * 4);
      methods_called++;
    }
  }

  EXPECT_EQ(true, success);
  EXPECT_EQ(methods_called * num_services, methods_executed);
  EXPECT_EQ(methods_called * num_services, call_executed_sucessfully);
  EXPECT_EQ(0, call_returned_timeout);

  // reset all
  success = true;
  methods_called = 0;
  methods_executed = 0;
  call_executed_sucessfully = 0;
  call_returned_timeout = 0;

  // some calls with service_callback_time_ms sleep in the method callback and to small timeout parameter
  method_process_time = 50;
  for (auto i = 0; i < calls; ++i)
  {
    // call methods
    for (const auto& client : client_vec)
    {
      // call method 1
      success &= client->CallWithCallback("foo::method1", "my request for method 1", response_callback, method_process_time / 10);
      eCAL::Process::SleepMS(method_process_time * 4);
      methods_called++;

      // call method 2
      success &= client->CallWithCallback("foo::method2", "my request for method 2", response_callback, method_process_time / 10);
      eCAL::Process::SleepMS(method_process_time * 4);
      methods_called++;
    }
  }
  eCAL::Process::SleepMS(CMN_REGISTRATION_REFRESH_MS);

  EXPECT_EQ(false, success);
  EXPECT_EQ(0, call_executed_sucessfully);
  EXPECT_EQ(methods_executed, call_returned_timeout);

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* ClientServerBaseCallbackTimeoutTest */

#if ClientServerCallWithResponseTimeBehaviorTest

// Test time-behavior of CallWithResponse with multiple servers when one server is slow
TEST(core_cpp_clientserver, ClientServerCallWithResponseTimeBehavior)
{
  const std::chrono::milliseconds server_2_calculation_time(200);

  // initialize eCAL API
  eCAL::Initialize("clientserver base callback timeout time behavior test");
  
  // Create 2 servers
  eCAL::CServiceServer server1("service");
  eCAL::CServiceServer server2("service");

  atomic_signalable<int> server1_callback_finished(0);
  atomic_signalable<int> server2_callback_finished(0);

  // give server1 a fast and server2 a slow method callback
  eCAL::SServiceMethodInformation method_info{ "mymethod", {}, {} };
  server1.SetMethodCallback(method_info,
                            [&server1_callback_finished](const eCAL::SServiceMethodInformation& /*method_info_*/, const std::string& request_, std::string& response_) -> int
                            {
                              response_ = "Server1 response on " + request_;
                              server1_callback_finished++;
                              return 1;
                            });

  server2.SetMethodCallback(method_info,
                            [&server2_callback_finished, server_2_calculation_time](const eCAL::SServiceMethodInformation& /*method_info_*/, const std::string& request_, std::string& response_) -> int
                            {
                              std::this_thread::sleep_for(server_2_calculation_time);
                              response_ = "Server2 response on " + request_;
                              server2_callback_finished++;
                              return 1;
                            });
  
  // Create a service client that calls both servers
  eCAL::CServiceClient client("service", eCAL::ServiceMethodInformationSetT());

  // Wait for server and client to match
  {
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() < (start + std::chrono::seconds(2)))
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      if (client.GetClientInstances().size() >= 2)
      {
        break;
      }
    }
  }

  // Call without timeout to test that it waits for all servers
  {
    server1_callback_finished = 0;
    server2_callback_finished = 0;

    eCAL::ServiceResponseVecT response_vec;

    auto start = std::chrono::steady_clock::now();
    client.CallWithResponse("mymethod", "Hello World", response_vec);
    auto end = std::chrono::steady_clock::now();

    EXPECT_EQ(server1_callback_finished.get(), 1);
    EXPECT_EQ(server2_callback_finished.get(), 1);
    EXPECT_GE(end - start, server_2_calculation_time);

    EXPECT_EQ(response_vec.size(), 2);
    // Check response verctor
    {
      bool server1_response_found = false;
      bool server2_response_found = false;
      for (const auto& response : response_vec)
      {
        if (response.response == "Server1 response on Hello World")
        {
          server1_response_found = true;
        }
        else if (response.response == "Server2 response on Hello World")
        {
          server2_response_found = true;
        }
      }
      EXPECT_EQ(server1_response_found, true);
      EXPECT_EQ(server2_response_found, true);
    }
  }

  // Call with a timeout to test that it does not wait
  {
    server1_callback_finished = 0;
    server2_callback_finished = 0;

    eCAL::ServiceResponseVecT response_vec;

    int timeout_ms = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(server_2_calculation_time / 2).count());

    auto start = std::chrono::steady_clock::now();
    client.CallWithResponse("mymethod", "Hello World", response_vec, timeout_ms);
    auto end = std::chrono::steady_clock::now();

    // Directly after service call
    EXPECT_EQ(server1_callback_finished.get(), 1);
    EXPECT_EQ(server2_callback_finished.get(), 0);
    EXPECT_GE(end - start, server_2_calculation_time / 2); // >= timeout
    EXPECT_LE(end - start, server_2_calculation_time);     // <= calculation time of server 2
    EXPECT_EQ(response_vec.size(), 2);                     // both servers returned, but one timed out

    // Check response verctor
    {
      int timeouts = 0;
      for (const auto& response : response_vec)
      {
        if (response.call_state == eCAL::eCallState::timeouted)
        {
          EXPECT_EQ(response.response, "");               // response is empty on timeout
          timeouts++;
        }
        else
        {
          EXPECT_EQ(response.response, "Server1 response on Hello World");
        }
      }
      EXPECT_EQ(timeouts, 1);                                // exactly one timeout
    }

    // Wait the remaining time and verify that the response vec still reports the timeout
    std::this_thread::sleep_for(server_2_calculation_time);

    {
      int timeouts = 0;
      for (const auto& response : response_vec)
      {
        if (response.call_state == eCAL::eCallState::timeouted)
        {
          EXPECT_EQ(response.response, "");               // response is empty on timeout
          timeouts++;
        }
        else
        {
          EXPECT_EQ(response.response, "Server1 response on Hello World");
        }
      }
      EXPECT_EQ(timeouts, 1);                                // exactly one timeout
    }
  }

  eCAL::Finalize();
}

#endif //ClientServerCallWithResponseTimeBehaviorTest

#if ClientServerCallWithCallbackTimeBehaviorTest

// Test time-behavior of CallWithCallback with multiple servers when one server is slow.
// Also tests that the function only returns AFTER the user-callback has been called for all responses, even if that goes beyond the timeout.
TEST(core_cpp_clientserver, ClientServerCallWithCallbackTimeBehavior)
{
  const std::chrono::milliseconds server_2_calculation_time(200);
    
  // initialize eCAL API
  eCAL::Initialize("clientserver call with callback time behavior test");
    
  // Create 2 servers
  eCAL::CServiceServer server1("service");
  eCAL::CServiceServer server2("service");
    
  atomic_signalable<int> server1_callback_finished(0);
  atomic_signalable<int> server2_callback_finished(0);
    
  // give server1 a fast and server2 a slow method callback
  eCAL::SServiceMethodInformation method_info{ "mymethod", {}, {} };
  server1.SetMethodCallback(method_info,
        [&server1_callback_finished](const eCAL::SServiceMethodInformation& /*method_info_*/, const std::string& request_, std::string& response_) -> int
        {
          response_ = "Server1 response on " + request_;
          server1_callback_finished++;
          return 1;
        });
    
  server2.SetMethodCallback(method_info,
        [&server2_callback_finished, server_2_calculation_time](const eCAL::SServiceMethodInformation& /*method_info_*/, const std::string& request_, std::string& response_) -> int
        {
          std::this_thread::sleep_for(server_2_calculation_time);
          response_ = "Server2 response on " + request_;
          server2_callback_finished++;
          return 1;
        });
    
  // Create a service client that calls both servers
  eCAL::CServiceClient client("service", eCAL::ServiceMethodInformationSetT());
    
  // Wait for server and client to match
  {
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() < (start + std::chrono::seconds(2)))
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      if (client.GetClientInstances().size() >= 2)
      {
        break;
      }
    }
  }
    
  // Call without a timeout to test that it waits for all servers
  {
    server1_callback_finished = 0;
    server2_callback_finished = 0;
    
    atomic_signalable<int> user_callback_finished(0);
    
    auto start = std::chrono::steady_clock::now();
    client.CallWithCallback("mymethod", "Hello World",
            [&user_callback_finished](const struct eCAL::SServiceResponse& service_response_)
            {
              ASSERT_EQ(service_response_.call_state, eCAL::eCallState::executed);
              user_callback_finished++;
            });
    auto end = std::chrono::steady_clock::now();
    
    EXPECT_EQ(server1_callback_finished.get(), 1);
    EXPECT_EQ(server2_callback_finished.get(), 1);
    EXPECT_EQ(user_callback_finished.get(), 2);
    EXPECT_GE(end - start, server_2_calculation_time);
  }

  // Call with a timeout to test that it DOES NOT wait for all servers, but still calls the user-callback for all responses
  {
    server1_callback_finished = 0;
    server2_callback_finished = 0;
    
    atomic_signalable<int> user_callback_finished_successful(0);
    atomic_signalable<int> user_callback_finished_timeout(0);
    
    int timeout_ms = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(server_2_calculation_time / 2).count());

    auto start = std::chrono::steady_clock::now();
    client.CallWithCallback("mymethod"
                , "Hello World",
                  [&user_callback_finished_successful, &user_callback_finished_timeout, server_2_calculation_time](const struct eCAL::SServiceResponse& service_response_)
                  {
                    std::this_thread::sleep_for(server_2_calculation_time * 2); // simulate long user callback to verify that CallWithCallback waits for it
                    if (service_response_.call_state == eCAL::eCallState::timeouted)
                    {
                      ASSERT_EQ(service_response_.response, ""); // response is empty on timeout
                      user_callback_finished_timeout++;
                    }
                    else
                    {
                      ASSERT_EQ(service_response_.response, "Server1 response on Hello World");
                      user_callback_finished_successful++;
                    }

                  }
                , timeout_ms);
    auto end = std::chrono::steady_clock::now();
    
    // Directly after service call
    EXPECT_EQ(server1_callback_finished.get(), 1); // We only know that server 1 finished so far
    // We cannot be sure if server 2 already finished or not (there are no guarantees), so we do not check server2_callback_finished here

    EXPECT_GE(end - start, server_2_calculation_time * 2 + server_2_calculation_time / 2); // even though we set a timeout, it had to wait for the long user callback to finish

    // 1 Callback has timeouted, the other one returned success
    EXPECT_EQ(user_callback_finished_successful.get(), 1);
    EXPECT_EQ(user_callback_finished_timeout.get(),    1);
  }

  eCAL::Finalize();
}

#endif //ClientServerCallWithCallbackTimeBehaviorTest

#if ClientServerBaseAsyncCallbackTest

TEST(core_cpp_clientserver, ClientServerBaseAsyncCallback)
{
  const int calls(1);
  const int sleep(100);

  // initialize eCAL API
  eCAL::Initialize("clientserver base async callback test");

  // create service server
  eCAL::CServiceServer server("service");

  // method callback function
  std::atomic<int> methods_executed(0);
  auto method_callback = [&](const eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_) -> int
    {
      PrintRequest(method_info_, request_);
      response_ = "I answered on " + request_;
      methods_executed++;
      return 42;
    };

  // add callback for client request
  eCAL::SServiceMethodInformation method1_info{ "foo::method1", {"foo::req_type1", "", ""}, {"foo::resp_type1", "", ""} };
  eCAL::SServiceMethodInformation method2_info{ "foo::method2", {"foo::req_type2", "", ""}, {"foo::resp_type2", "", ""} };
  server.SetMethodCallback(method1_info, method_callback);
  server.SetMethodCallback(method2_info, method_callback);

  // create service client
  eCAL::CServiceClient client("service");

  // response callback function
  std::atomic<int> responses_executed(0);
  auto response_callback = [&](const struct eCAL::SServiceResponse& service_response_)
    {
      PrintResponse(service_response_);
      responses_executed++;
    };

  // let's match them -> wait REGISTRATION_REFRESH_CYCLE (ecal_def.h)
  eCAL::Process::SleepMS(2000);

  // call service
  std::atomic<int> methods_called(0);
  std::string m1("foo::method1");
  std::string m2("foo::method2");
  std::string r1("my request for method 1");
  std::string r2("my request for method 2");

  // some calls with no sleep in the method callback
  for (auto i = 0; i < calls; ++i)
  {
    // call method 1
    client.CallWithCallbackAsync(m1, r1, response_callback);
    eCAL::Process::SleepMS(sleep);
    methods_called++;

    // call method 2
    client.CallWithCallbackAsync(m2, r2, response_callback);
    eCAL::Process::SleepMS(sleep);
    methods_called++;
  }

  EXPECT_EQ(methods_called, methods_executed);
  EXPECT_EQ(methods_called, responses_executed);

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* ClientServerBaseAsyncCallbackTest */

#if ClientServerBaseAsyncTest

TEST(core_cpp_clientserver, ClientServerBaseAsync)
{
  const int calls(5);

  // initialize eCAL API
  eCAL::Initialize("clientserver base async callback");

  // create service server
  eCAL::CServiceServer server("service");

  // method callback function
  atomic_signalable<int> num_service_callbacks_finished(0);
  int service_callback_time_ms(0);

  auto method_callback = [&](const eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_) -> int
    {
      eCAL::Process::SleepMS(service_callback_time_ms);
      PrintRequest(method_info_, request_);
      response_ = "I answered on " + request_;
      num_service_callbacks_finished++;
      return 42;
    };

  // add callback for client request
  eCAL::SServiceMethodInformation method1_info{ "foo::method1", {"foo::req_type1", "", ""}, {"foo::resp_type1", "", ""} };
  eCAL::SServiceMethodInformation method2_info{ "foo::method2", {"foo::req_type2", "", ""}, {"foo::resp_type2", "", ""} };
  server.SetMethodCallback(method1_info, method_callback);
  server.SetMethodCallback(method2_info, method_callback);

  // create service client
  eCAL::CServiceClient client("service");

  // response callback function
  atomic_signalable<int> num_client_response_callbacks_finished(0);
  auto response_callback = [&](const struct eCAL::SServiceResponse& service_response_)
    {
      PrintResponse(service_response_);
      num_client_response_callbacks_finished++;
    };

  // let's match them -> wait REGISTRATION_REFRESH_CYCLE (ecal_def.h)
  eCAL::Process::SleepMS(2000);

  // call service
  int num_service_calls(0);
  std::string m1("foo::method1");
  std::string m2("foo::method2");
  std::string r1("my request for method 1");
  std::string r2("my request for method 2");

  // some calls with service_callback_time_ms sleep in the method callback
  service_callback_time_ms = 100;
  for (auto i = 0; i < calls; ++i)
  {
    // call method 1
    client.CallWithCallbackAsync(m1, r1, response_callback);
    eCAL::Process::SleepMS(service_callback_time_ms * 2);
    num_service_calls++;

    // call method 2
    client.CallWithCallbackAsync(m2, r2, response_callback);
    eCAL::Process::SleepMS(service_callback_time_ms * 2);
    num_service_calls++;
  }

  num_client_response_callbacks_finished.wait_for([num_service_calls](int v) { return num_service_calls == v; }, std::chrono::seconds(1));

  EXPECT_EQ(num_service_calls, num_service_callbacks_finished.get());
  EXPECT_EQ(num_service_calls, num_client_response_callbacks_finished.get());

  // Call the methods directly one after another and then wait for the responses.
  // As the service callback needs some time to finish and we call it from only
  // 1 Client, they will be effectively be serialized.
  num_service_calls = 0;
  num_service_callbacks_finished = 0;
  num_client_response_callbacks_finished = 0;

  service_callback_time_ms = 100;

  auto start = std::chrono::steady_clock::now();
  for (auto i = 0; i < calls; ++i)
  {
    client.CallWithCallbackAsync(m1, r1, response_callback);
    num_service_calls++;
  }
  auto async_call_end = std::chrono::steady_clock::now();

  EXPECT_LT(async_call_end - start, std::chrono::milliseconds(100)); // The call should return immediately, as they are async.

  num_client_response_callbacks_finished.wait_for([num_service_calls](int v) { return num_service_calls == v; }, std::chrono::seconds(10));

  auto async_response_end = std::chrono::steady_clock::now();

  EXPECT_EQ(num_service_calls, num_service_callbacks_finished.get());
  EXPECT_EQ(num_service_calls, num_client_response_callbacks_finished.get());
  EXPECT_GT(async_response_end - start, std::chrono::milliseconds(service_callback_time_ms) * num_service_calls); // The response should take some time, as the service callback needs some time to finish.


  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* ClientServerBaseAsyncTest */

#if ClientServerBaseBlockingTest

TEST(core_cpp_clientserver, ClientServerBaseBlocking)
{
  const int num_services(2);
  const int num_clients(3);
  const int calls(1);

  // initialize eCAL API
  eCAL::Initialize("clientserver base blocking test");

  // create service servers
  ServiceVecT service_vec;
  for (auto s = 0; s < num_services; ++s)
  {
    service_vec.push_back(std::make_shared<eCAL::CServiceServer>("service"));
  }

  // method callback function
  std::atomic<int> methods_executed(0);
  auto method_callback = [&](const eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_) -> int
    {
      PrintRequest(method_info_, request_);
      response_ = "I answer on " + request_;
      methods_executed++;
      return 24;
    };

  // add method callback
  for (const auto& service : service_vec)
  {
    eCAL::SServiceMethodInformation method1_info{ "foo::method1", {"foo::req_type1", "", ""}, {"foo::resp_type1", "", ""} };
    eCAL::SServiceMethodInformation method2_info{ "foo::method2", {"foo::req_type2", "", ""}, {"foo::resp_type2", "", ""} };
    service->SetMethodCallback(method1_info, method_callback);
    service->SetMethodCallback(method2_info, method_callback);
  }

  // create service clients
  ClientVecT client_vec;
  for (auto s = 0; s < num_clients; ++s)
  {
    client_vec.push_back(std::make_shared<eCAL::CServiceClient>("service"));
  }

  // let's match them -> wait REGISTRATION_REFRESH_CYCLE (ecal_def.h)
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

  // call service
  std::atomic<int> methods_called(0);
  std::atomic<int> responses_executed(0);
  eCAL::ServiceResponseVecT service_response_vec;
  for (auto i = 0; i < calls; ++i)
  {
    // call methods
    for (const auto& client : client_vec)
    {
      // call method 1
      if (client->CallWithResponse("foo::method1", "my request for method 1", service_response_vec))
      {
        ASSERT_EQ(2, service_response_vec.size());

        PrintResponse(service_response_vec[0]);
        responses_executed++;

        PrintResponse(service_response_vec[1]);
        responses_executed++;

        methods_called++;
      }

      // call method 2
      if (client->CallWithResponse("foo::method2", "my request for method 2", service_response_vec))
      {
        ASSERT_EQ(2, service_response_vec.size());

        PrintResponse(service_response_vec[0]);
        responses_executed++;

        PrintResponse(service_response_vec[1]);
        responses_executed++;

        methods_called++;
      }
    }
  }

  EXPECT_EQ(methods_called * num_services, methods_executed);
  EXPECT_EQ(methods_called * num_services, responses_executed);

  // remove method callback
  for (const auto& service : service_vec)
  {
    service->RemoveMethodCallback("foo::method1");
    service->RemoveMethodCallback("foo::method2");
  }

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* ClientServerBaseBlockingTest */

#if NestedRPCCallTest

TEST(core_cpp_clientserver, NestedRPCCall)
{
  const int calls(1);
  const int sleep(0);

  // initialize eCAL API
  eCAL::Initialize("nested rpc call test");

  // create service server
  eCAL::CServiceServer server("service");

  // request callback function
  std::atomic<int> methods_executed(0);
  auto method_callback = [&](const eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_) -> int
    {
      PrintRequest(method_info_, request_);
      response_ = "I answer on " + request_;
      methods_executed++;
      return 42;
    };

  // add callback for client request
  eCAL::SServiceMethodInformation method1_info{ "foo::method1", {"foo::req_type1", "", ""}, {"foo::resp_type1", "", ""} };
  eCAL::SServiceMethodInformation method2_info{ "foo::method2", {"foo::req_type2", "", ""}, {"foo::resp_type2", "", ""} };
  server.SetMethodCallback(method1_info, method_callback);
  server.SetMethodCallback(method2_info, method_callback);

  // create service client
  eCAL::CServiceClient client1("service");
  eCAL::CServiceClient client2("service");

  // response callback function
  std::atomic<int> methods_called(0);
  std::atomic<int> responses_executed(0);
  bool success(true);
  auto response_callback2 = [&](const struct eCAL::SServiceResponse& service_response_)
    {
      PrintResponse(service_response_);
      responses_executed++;
    };
  auto response_callback1 = [&](const struct eCAL::SServiceResponse& service_response_)
    {
      PrintResponse(service_response_);
      success &= client2.CallWithCallback("foo::method2", "my request for method 2", response_callback2);
      methods_called++;
      responses_executed++;
    };

  // let's match them -> wait REGISTRATION_REFRESH_CYCLE (ecal_def.h)
  eCAL::Process::SleepMS(2000);

  // call service
  for (auto i = 0; i < calls; ++i)
  {
    // call method 1
    success &= client1.CallWithCallback("foo::method1", "my request for method 1", response_callback1);
    eCAL::Process::SleepMS(sleep);
    methods_called++;
  }

  EXPECT_EQ(true, success);
  EXPECT_EQ(methods_called, methods_executed);
  EXPECT_EQ(methods_called, responses_executed);

  // remove method callback
  server.RemoveMethodCallback("foo::method1");
  server.RemoveMethodCallback("foo::method2");

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* NestedRPCCallTest */

#if NestedBlockingCallFromServerCallbackTest

TEST(core_cpp_clientserver, NestedBlockingCallFromServerCallback)
{
  constexpr size_t depth = 2;
  
  // validat input, just to make sure we don't break the test with wrong input
  assert(depth >= 1);

  // initialize eCAL API
  eCAL::Initialize("nested blocking call from server callback test");

  // Create servers and clients. We build a chain of depth amount of servers with each having a client that can call it.
  std::vector<eCAL::CServiceServer> servers;
  std::vector<eCAL::CServiceClient> clients;
  servers.reserve(depth);
  clients.reserve(depth);
  for (size_t i = 0; i < depth; i++)
  {
    servers.emplace_back("service#" + std::to_string(i));
    clients.emplace_back("service#" + std::to_string(i));

    // Add a callback to the server that blocking-calls the next service
    servers.back().SetMethodCallback(eCAL::SServiceMethodInformation{ "my_method", {}, {} },
          [&clients, service_index = i](const eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_) -> int
          {
#if DO_LOGGING
            PrintRequest(method_info_, request_);
#endif // DO_LOGGING
            EXPECT_EQ(request_, std::to_string(service_index));

            // Use according client to call the next server
            if ((service_index + 1) < clients.size())
            {
              eCAL::ServiceResponseVecT service_response_vec;
              clients[service_index + 1].CallWithResponse("my_method", std::to_string(service_index + 1), service_response_vec);

              EXPECT_EQ(service_response_vec.size(), 1);
              EXPECT_EQ(service_response_vec.begin()->response, "Response from " + std::to_string(service_index + 1));
            }

            response_ = "Response from " + std::to_string(service_index);
            return 0;
          }
    );
  }

  // Wait for servers and clients to match
  {
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() < (start + std::chrono::seconds(2)))
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

      for (const auto& client : clients)
      {
        if (client.GetClientInstances().size() == 0)
        {
          continue;
        }
      }
      break;
    }
  }

  // Call the first client
  eCAL::ServiceResponseVecT service_response_vec;
  clients.front().CallWithResponse("my_method", "0", service_response_vec);

  ASSERT_EQ(service_response_vec.size(), 1);
  ASSERT_EQ(service_response_vec.begin()->response, "Response from 0");

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* NestedBlockingCallFromServerCallbackTest */
