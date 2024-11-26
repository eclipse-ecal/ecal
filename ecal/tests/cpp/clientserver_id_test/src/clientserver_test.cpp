/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

#include <cmath>
#include <iostream>

#include <gtest/gtest.h>

#include "atomic_signalable.h"

#define ClientConnectEventTest                    1
#define ServerConnectEventTest                    1

#define ClientServerBaseCallbackTest              1
#define ClientServerBaseCallbackTimeoutTest       1

#define ClientServerBaseAsyncCallbackTest         0
#define ClientServerBaseAsyncTest                 0

#define ClientServerBaseBlockingTest              0

#define NestedRPCCallTest                         0

enum {
  CMN_REGISTRATION_REFRESH_MS = 1000
};

namespace
{
  typedef std::vector<std::shared_ptr<eCAL::CServiceServer>>   ServiceVecT;
  typedef std::vector<std::shared_ptr<eCAL::CServiceClientID>> ClientVecT;

#if 0
  void PrintRequest(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_)
  {
    std::cout << "------ REQUEST -------" << std::endl;
    std::cout << "Method name           : " << method_    << std::endl;
    std::cout << "Method request type   : " << req_type_  << std::endl;
    std::cout << "Method response type  : " << resp_type_ << std::endl;
    std::cout << "Method request type   : " << resp_type_ << std::endl;
    std::cout << "Method request        : " << request_   << std::endl;
    std::cout << std::endl;
  }
#else
  void PrintRequest(const std::string& /*method_*/, const std::string& /*req_type_*/, const std::string& /*resp_type_*/, const std::string& /*request_*/)
  {
  }
#endif

#if 0
  void PrintResponse(const struct eCAL::SServiceResponse& service_response_)
  {
    std::cout << "------ RESPONSE ------" << std::endl;
    std::cout << "Executed on host name : " << service_response_.host_name    << std::endl;
    std::cout << "Executed service name : " << service_response_.service_name << std::endl;
    std::cout << "Executed method name  : " << service_response_.method_name  << std::endl;
    std::cout << "Return value          : " << service_response_.ret_state    << std::endl;
    std::cout << "Execution state       : ";
    switch (service_response_.call_state)
    {
    case call_state_none:
      std::cout << "call_state_none";
      break;
    case call_state_executed:
      std::cout << "call_state_executed";
      break;
    case call_state_failed:
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

TEST(core_cpp_clientserver_id, ClientConnectEvent)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "clientserver base connect event callback");

  // enable loop back communication in the same thread
  eCAL::Util::EnableLoopback(true);

  // client event callback for connect events
  atomic_signalable<int> event_connected_fired   (0);
  atomic_signalable<int> event_disconnected_fired(0);
  auto event_callback = [&](const eCAL::Registration::SServiceId& service_id_, const struct eCAL::SClientEventCallbackData* data_)
    {
      switch (data_->type)
      {
      case client_event_connected:
        std::cout << "event connected fired" << std::endl;
        event_connected_fired++;
        break;
      case client_event_disconnected:
        std::cout << "event disconnected fired" << std::endl;
        event_disconnected_fired++;
        break;
      default:
        break;
      }
    };

  // create client
  eCAL::CServiceClientID client("service", eCAL::ServiceMethodInformationMapT(), event_callback);

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

TEST(core_cpp_clientserver_id, ServerConnectEvent)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "clientserver base connect event callback");

  // enable loop back communication in the same thread
  eCAL::Util::EnableLoopback(true);

  // create server
  eCAL::CServiceServer server("service");

  // server event callback for connect events
  atomic_signalable<int> event_connected_fired   (0);
  atomic_signalable<int> event_disconnected_fired(0);
  auto event_callback = [&](const struct eCAL::SServerEventCallbackData* data_) -> void
    {
      switch (data_->type)
      {
      case server_event_connected:
        std::cout << "event connected fired" << std::endl;
        event_connected_fired++;
        break;
      case server_event_disconnected:
        std::cout << "event disconnected fired" << std::endl;
        event_disconnected_fired++;
        break;
      default:
        break;
      }
    };
  // attach event
  server.AddEventCallback(server_event_connected, std::bind(event_callback, std::placeholders::_2));
  server.AddEventCallback(server_event_disconnected, std::bind(event_callback, std::placeholders::_2));

  // check events
  eCAL::Process::SleepMS(1000);
  EXPECT_EQ(0, event_connected_fired.get());
  EXPECT_EQ(0, event_disconnected_fired.get());

  // create clients
  {
    eCAL::CServiceClientID client1("service");

    event_connected_fired.wait_for([](int v) { return v >= 1; }, std::chrono::milliseconds(3 * CMN_REGISTRATION_REFRESH_MS));
    EXPECT_EQ(1, event_connected_fired.get());
    EXPECT_EQ(0, event_disconnected_fired.get());

    eCAL::CServiceClientID client2("service");

    // TODO: Service API should trigger connect event with every new connection (client side is acting this way!)
    //event_disconnected_fired.wait_for([](int v) { return v >= 2; }, std::chrono::milliseconds(3 * CMN_REGISTRATION_REFRESH_MS));
    //EXPECT_EQ(2, event_connected_fired.get());
    //EXPECT_EQ(0, event_disconnected_fired.get());

    eCAL::Process::SleepMS(2000);                       
    EXPECT_EQ(1, event_connected_fired.get());
    EXPECT_EQ(0, event_disconnected_fired.get());
  }

  // TODO: Service API should trigger disconnect event with every single disconnection (client side is acting this way!)
  //event_disconnected_fired.wait_for([](int v) { return v >= 2; }, std::chrono::milliseconds(3 * CMN_REGISTRATION_REFRESH_MS));
  //EXPECT_EQ(2, event_connected_fired.get());
  //EXPECT_EQ(2, event_disconnected_fired.get());

  // wait for disconnection
  event_disconnected_fired.wait_for([](int v) { return v >= 1; }, std::chrono::milliseconds(3 * CMN_REGISTRATION_REFRESH_MS));
  EXPECT_EQ(1, event_connected_fired.get());
  EXPECT_EQ(1, event_disconnected_fired.get());

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* ServerConnectEventTest */

#if ClientServerBaseCallbackTest

TEST(core_cpp_clientserver, BaseCallback)
{
  const int num_services(2);
  const int num_clients(3);
  const int calls(1);
  const int sleep(0);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "clientserver base callback test");

  // enable loop back communication in the same thread
  eCAL::Util::EnableLoopback(true);

  // create service servers
  ServiceVecT service_vec;
  for (auto s = 0; s < num_services; ++s)
  {
    service_vec.push_back(std::make_shared<eCAL::CServiceServer>("service"));
  }

  // method callback function
  std::atomic<int> methods_executed(0);
  std::atomic<int> method_process_time(0);
  auto method_callback = [&](const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_) -> int
    {
      eCAL::Process::SleepMS(method_process_time);
      PrintRequest(method_, req_type_, resp_type_, request_);
      response_ = "I answer on " + request_;
      methods_executed++;
      return 42;
    };

  // add method callbacks
  for (const auto& service : service_vec)
  {
    service->AddMethodCallback("foo::method1", "foo::req_type1", "foo::resp_type1", method_callback);
    service->AddMethodCallback("foo::method2", "foo::req_type2", "foo::resp_type2", method_callback);
  }

  // create service clients
  ClientVecT client_vec;
  for (auto s = 0; s < num_clients; ++s)
  {
    client_vec.push_back(std::make_shared<eCAL::CServiceClientID>("service"));
  }

  // response callback function
  std::atomic<int> responses_executed(0);
  auto response_callback = [&](const eCAL::Registration::SEntityId& /*entity_id_*/, const struct eCAL::SServiceResponse& service_response_)
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
      success &= client->CallWithCallback("foo::method1", "my request for method 1", -1, response_callback);
      eCAL::Process::SleepMS(sleep);
      methods_called++;

      // call method 2
      success &= client->CallWithCallback("foo::method2", "my request for method 2", -1, response_callback);
      eCAL::Process::SleepMS(sleep);
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
      success &= client->CallWithCallback("foo::method1", "my request for method 1", -1, response_callback);
      eCAL::Process::SleepMS(sleep);
      methods_called++;

      // call method 2
      success &= client->CallWithCallback("foo::method2", "my request for method 2", -1, response_callback);
      eCAL::Process::SleepMS(sleep);
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

TEST(core_cpp_clientserver, BaseCallbackTimeout)
{
  const int num_services(2);
  const int num_clients(3);
  const int calls(1);
  const int sleep(0);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "clientserver base callback test with timeout");

  // enable loop back communication in the same thread
  eCAL::Util::EnableLoopback(true);

  // create service servers
  ServiceVecT service_vec;
  for (auto s = 0; s < num_services; ++s)
  {
    service_vec.push_back(std::make_shared<eCAL::CServiceServer>("service"));
  }

  // method callback function
  std::atomic<int> methods_executed(0);
  std::atomic<int> method_process_time(0);
  auto method_callback = [&](const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_) -> int
    {
      eCAL::Process::SleepMS(method_process_time);
      PrintRequest(method_, req_type_, resp_type_, request_);
      response_ = "I answer on " + request_;
      methods_executed++;
      return 42;
    };

  // add method callbacks
  for (const auto& service : service_vec)
  {
    service->AddMethodCallback("foo::method1", "foo::req_type1", "foo::resp_type1", method_callback);
    service->AddMethodCallback("foo::method2", "foo::req_type2", "foo::resp_type2", method_callback);
  }

  // event callback for timeout event
  std::atomic<int> timeout_fired(0);
  auto event_callback = [&](const eCAL::Registration::SServiceId& /*service_id_*/, const struct eCAL::SClientEventCallbackData* data_) -> void
    {
      if ((data_ != nullptr) && (data_->type == client_event_timeout))
      {
        std::cout << "event timeouted fired" << std::endl;
        timeout_fired++;
      }
    };

  // create service clients
  ClientVecT client_vec;
  for (auto s = 0; s < num_clients; ++s)
  {
    client_vec.push_back(std::make_shared<eCAL::CServiceClientID>("service", eCAL::ServiceMethodInformationMapT(), event_callback));
  }

  // response callback function
  std::atomic<int> responses_executed(0);
  auto response_callback = [&](const eCAL::Registration::SEntityId& /*entity_id_*/, const struct eCAL::SServiceResponse& service_response_)
    {
      PrintResponse(service_response_);
      responses_executed++;
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
      success &= client->CallWithCallback("foo::method1", "my request for method 1", -1, response_callback);
      eCAL::Process::SleepMS(sleep);
      methods_called++;

      // call method 2
      success &= client->CallWithCallback("foo::method2", "my request for method 2", -1, response_callback);
      eCAL::Process::SleepMS(sleep);
      methods_called++;
    }
  }

  EXPECT_EQ(true, success);
  EXPECT_EQ(methods_called * num_services, methods_executed);
  EXPECT_EQ(methods_called * num_services, responses_executed);
  EXPECT_EQ(0, timeout_fired);

  // reset all
  success = true;
  methods_called = 0;
  methods_executed = 0;
  responses_executed = 0;
  timeout_fired = 0;

  // some calls with service_callback_time_ms sleep in the method callback and a proper timeout parameter
  method_process_time = 50;
  for (auto i = 0; i < calls; ++i)
  {
    // call methods
    for (const auto& client : client_vec)
    {
      // call method 1
      success &= client->CallWithCallback("foo::method1", "my request for method 1", method_process_time * 4, response_callback);
      eCAL::Process::SleepMS(sleep);
      methods_called++;

      // call method 2
      success &= client->CallWithCallback("foo::method2", "my request for method 2", method_process_time * 4, response_callback);
      eCAL::Process::SleepMS(sleep);
      methods_called++;
    }
  }

  EXPECT_EQ(true, success);
  EXPECT_EQ(methods_called * num_services, methods_executed);
  EXPECT_EQ(methods_called * num_services, responses_executed);
  EXPECT_EQ(0, timeout_fired);

  // reset all
  success = true;
  methods_called = 0;
  methods_executed = 0;
  responses_executed = 0;
  timeout_fired = 0;

  // some calls with service_callback_time_ms sleep in the method callback and to small timeout parameter
  method_process_time = 50;
  for (auto i = 0; i < calls; ++i)
  {
    // call methods
    for (const auto& client : client_vec)
    {
      // call method 1
      success &= client->CallWithCallback("foo::method1", "my request for method 1", method_process_time / 10, response_callback);
      eCAL::Process::SleepMS(method_process_time * 4);
      methods_called++;

      // call method 2
      success &= client->CallWithCallback("foo::method2", "my request for method 2", method_process_time / 10, response_callback);
      eCAL::Process::SleepMS(method_process_time * 4);
      methods_called++;
    }
  }
  eCAL::Process::SleepMS(1000);

  EXPECT_EQ(false, success);
  EXPECT_EQ(0, responses_executed);
  EXPECT_EQ(methods_executed, timeout_fired);

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* ClientServerBaseCallbackTimeoutTest */
