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

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/client.h>
#include <ecal/msg/protobuf/server.h>

#include <cmath>
#include <iostream>

#include <gtest/gtest.h>

#include "math.pb.h"
#include "ping.pb.h"

#define ClientConnectEventTest                    1
#define ServerConnectEventTest                    1

#define ClientServerBaseCallbackTest              1
#define ClientServerBaseCallbackTimeoutTest       1

#define ClientServerBaseAsyncCallbackTest         1
#define ClientServerBaseAsyncCallbackTimeoutTest  1

#define ClientServerBaseBlockingTest              1

#define ClientServerProtoCallbackTest             1
#define ClientServerProtoBlockingTest             1

#define NestedRPCCallTest                         1

namespace
{
  typedef std::vector<std::shared_ptr<eCAL::CServiceServer>> ServiceVecT;
  typedef std::vector<std::shared_ptr<eCAL::CServiceClient>> ClientVecT;

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
  };
}

#if ClientConnectEventTest

TEST(IO, ClientConnectEvent)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "clientserver base connect event callback");

  // create client
  eCAL::CServiceClient client("service");

  // add client event callback for connect event
  int event_connected_fired(0);
  int event_disconnected_fired(0);
  auto event_callback = [&](const struct eCAL::SClientEventCallbackData* data_) -> void
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
  // attach event
  client.AddEventCallback(client_event_connected,    std::bind(event_callback, std::placeholders::_2));
  client.AddEventCallback(client_event_disconnected, std::bind(event_callback, std::placeholders::_2));

  // check events
  eCAL::Process::SleepMS(1000);
  EXPECT_EQ(0, event_connected_fired);
  EXPECT_EQ(0, event_disconnected_fired);

  // create server
  {
    eCAL::CServiceServer server1("service");

    eCAL::Process::SleepMS(2000);
    EXPECT_EQ(1, event_connected_fired);
    EXPECT_EQ(0, event_disconnected_fired);

    eCAL::CServiceServer server2("service");

    eCAL::Process::SleepMS(2000);
    EXPECT_EQ(2, event_connected_fired);
    EXPECT_EQ(0, event_disconnected_fired);
  }

  // do a dummy call
  // after that disconnect events should fire for now
  // this needs to be improved, clients needs to be
  // informed about disconnection without calling a service method
  client.Call("foo", "");
  eCAL::Process::SleepMS(1000);
  EXPECT_EQ(1, event_disconnected_fired);
  client.Call("foo", "");
  eCAL::Process::SleepMS(1000);
  EXPECT_EQ(2, event_disconnected_fired);
  client.Call("foo", "");
  eCAL::Process::SleepMS(1000);
  EXPECT_EQ(2, event_disconnected_fired);

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* ServerConnectEventTest */

#if ServerConnectEventTest

TEST(IO, ServerConnectEvent)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "clientserver base connect event callback");

  // create server
  eCAL::CServiceServer server("service");

  // add server event callback for connect event
  int event_connected_fired(0);
  int event_disconnected_fired(0);
  auto event_callback = [&](const struct eCAL::SServerEventCallbackData* data_) -> void
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
  // attach event
  server.AddEventCallback(server_event_connected,    std::bind(event_callback, std::placeholders::_2));
  server.AddEventCallback(server_event_disconnected, std::bind(event_callback, std::placeholders::_2));

  // check events
  eCAL::Process::SleepMS(1000);
  EXPECT_EQ(0, event_connected_fired);
  EXPECT_EQ(0, event_disconnected_fired);

  // create server
  {
    eCAL::CServiceClient client1("service");

    eCAL::Process::SleepMS(2000);
    EXPECT_EQ(1, event_connected_fired);
    EXPECT_EQ(0, event_disconnected_fired);

    eCAL::CServiceClient client2("service");

    eCAL::Process::SleepMS(2000);
    EXPECT_EQ(1, event_connected_fired);
    EXPECT_EQ(0, event_disconnected_fired);
  }
  eCAL::Process::SleepMS(2000);
  EXPECT_EQ(1, event_connected_fired);
  EXPECT_EQ(1, event_disconnected_fired);

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* ServerConnectEventTest */

#if ClientServerBaseCallbackTest

TEST(IO, ClientServerBaseCallback)
{
  const int num_services(2);
  const int num_clients(3);
  const int calls(1);
  const int sleep(0);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "clientserver base callback test");

  // create service servers
  ServiceVecT service_vec;
  for (auto s = 0; s < num_services; ++s)
  {
    service_vec.push_back(std::make_shared<eCAL::CServiceServer>("service"));
  }

  // method callback function
  int methods_executed(0);
  int method_process_time(0);
  auto method_callback = [&](const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_) -> int
  {
    eCAL::Process::SleepMS(method_process_time);
    PrintRequest(method_, req_type_, resp_type_, request_);
    response_ = "I answer on " + request_;
    methods_executed++;
    return 42;
  };

  // add method callbacks
  for (auto service : service_vec)
  {
    service->AddMethodCallback("foo::method1", "foo::req_type1", "foo::resp_type1", method_callback);
    service->AddMethodCallback("foo::method2", "foo::req_type2", "foo::resp_type2", method_callback);
  }

  // create service clients
  ClientVecT client_vec;
  for (auto s = 0; s < num_clients; ++s)
  {
    client_vec.push_back(std::make_shared<eCAL::CServiceClient>("service"));
  }

  // response callback function
  int responses_executed(0);
  auto response_callback = [&](const struct eCAL::SServiceResponse& service_response_)
  {
    PrintResponse(service_response_);
    responses_executed++;
  };

  // add callback for server response
  for (auto client : client_vec)
  {
    client->AddResponseCallback(response_callback);
  }

  // let's match them -> wait REGISTRATION_REFRESH_CYCLE (ecal_def.h)
  eCAL::Process::SleepMS(2000);

  // call service
  int methods_called(0);
  bool success(true);

  // some calls with no sleep in the method callback
  for (auto i = 0; i < calls; ++i)
  {
    // call methods
    for (auto client : client_vec)
    {
      // call method 1
      success &= client->Call("foo::method1", "my request for method 1");
      eCAL::Process::SleepMS(sleep);
      methods_called++;

      // call method 2
      success &= client->Call("foo::method2", "my request for method 2");
      eCAL::Process::SleepMS(sleep);
      methods_called++;
    }
  }

  // some calls with method_process_time sleep in the method callback
  method_process_time = 100;
  for (auto i = 0; i < calls; ++i)
  {
    // call methods
    for (auto client : client_vec)
    {
      // call method 1
      success &= client->Call("foo::method1", "my request for method 1");
      eCAL::Process::SleepMS(sleep);
      methods_called++;

      // call method 2
      success &= client->Call("foo::method2", "my request for method 2");
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

TEST(IO, ClientServerBaseCallbackTimeout)
{
  const int num_services(2);
  const int num_clients(3);
  const int calls(1);
  const int sleep(0);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "clientserver base callback test with timeout");

  // create service servers
  ServiceVecT service_vec;
  for (auto s = 0; s < num_services; ++s)
  {
    service_vec.push_back(std::make_shared<eCAL::CServiceServer>("service"));
  }

  // method callback function
  int methods_executed(0);
  int method_process_time(0);
  auto method_callback = [&](const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_) -> int
  {
    eCAL::Process::SleepMS(method_process_time);
    PrintRequest(method_, req_type_, resp_type_, request_);
    response_ = "I answer on " + request_;
    methods_executed++;
    return 42;
  };

  // add method callbacks
  for (auto service : service_vec)
  {
    service->AddMethodCallback("foo::method1", "foo::req_type1", "foo::resp_type1", method_callback);
    service->AddMethodCallback("foo::method2", "foo::req_type2", "foo::resp_type2", method_callback);
  }

  // create service clients
  ClientVecT client_vec;
  for (auto s = 0; s < num_clients; ++s)
  {
    client_vec.push_back(std::make_shared<eCAL::CServiceClient>("service"));
  }

  // response callback function
  int responses_executed(0);
  auto response_callback = [&](const struct eCAL::SServiceResponse& service_response_)
  {
    PrintResponse(service_response_);
    responses_executed++;
  };

  // add callback for server response
  for (auto client : client_vec)
  {
    client->AddResponseCallback(response_callback);
  }

  // add event callback for timeout event
  int timeout_fired = 0;
  auto event_callback = [&](const struct eCAL::SClientEventCallbackData* /*data_*/) -> void
  {
    timeout_fired++;
  };
  for (auto client : client_vec)
  {
    // catch events
    client->AddEventCallback(client_event_timeout, std::bind(event_callback, std::placeholders::_2));
  }

  // let's match them -> wait REGISTRATION_REFRESH_CYCLE (ecal_def.h)
  eCAL::Process::SleepMS(2000);

  // call service
  int methods_called(0);
  bool success(true);

  // some calls with method_process_time sleep in the method callback
  method_process_time = 100;
  for (auto i = 0; i < calls; ++i)
  {
    // call methods
    for (auto client : client_vec)
    {
      // call method 1
      success &= client->Call("foo::method1", "my request for method 1");
      eCAL::Process::SleepMS(sleep);
      methods_called++;

      // call method 2
      success &= client->Call("foo::method2", "my request for method 2");
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

  // some calls with method_process_time sleep in the method callback and a proper timeout parameter
  method_process_time = 100;
  for (auto i = 0; i < calls; ++i)
  {
    // call methods
    for (auto client : client_vec)
    {
      // call method 1
      success &= client->Call("foo::method1", "my request for method 1", method_process_time * 2);
      eCAL::Process::SleepMS(sleep);
      methods_called++;

      // call method 2
      success &= client->Call("foo::method2", "my request for method 2", method_process_time * 2);
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

  // some calls with method_process_time sleep in the method callback and to small timeout parameter
  method_process_time = 100;
  for (auto i = 0; i < calls; ++i)
  {
    // call methods
    for (auto client : client_vec)
    {
      // call method 1
      success &= client->Call("foo::method1", "my request for method 1", method_process_time / 10);
      eCAL::Process::SleepMS(method_process_time * 2);
      methods_called++;

      // call method 2
      success &= client->Call("foo::method2", "my request for method 2", method_process_time / 10);
      eCAL::Process::SleepMS(method_process_time * 2);
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

#if ClientServerBaseAsyncCallbackTest

TEST(IO, ClientServerBaseAsyncCallback)
{
  const int calls(1);
  const int sleep(100);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "clientserver base async callback test");

  // create service server
  eCAL::CServiceServer server("service");

  // method callback function
  int methods_executed(0);
  auto method_callback = [&](const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_) -> int
  {
    PrintRequest(method_, req_type_, resp_type_, request_);
    response_ = "I answered on " + request_;
    methods_executed++;
    return 42;
  };

  // add callback for client request
  server.AddMethodCallback("foo::method1", "foo::req_type1", "foo::resp_type1", method_callback);
  server.AddMethodCallback("foo::method2", "foo::req_type2", "foo::resp_type2", method_callback);

  // create service client
  eCAL::CServiceClient client("service");

  // response callback function
  int responses_executed(0);
  auto response_callback = [&](const struct eCAL::SServiceResponse& service_response_)
  {
    PrintResponse(service_response_);
    responses_executed++;
  };

  // add callback for server response
  client.AddResponseCallback(response_callback);

  // let's match them -> wait REGISTRATION_REFRESH_CYCLE (ecal_def.h)
  eCAL::Process::SleepMS(2000);

  // call service
  int methods_called(0);
  std::string m1("foo::method1");
  std::string m2("foo::method2");
  std::string r1("my request for method 1");
  std::string r2("my request for method 2");

  // some calls with no sleep in the method callback
  for (auto i = 0; i < calls; ++i)
  {
    // call method 1
    client.CallAsync(m1, r1);
    eCAL::Process::SleepMS(sleep);
    methods_called++;

    // call method 2
    client.CallAsync(m2, r2);
    eCAL::Process::SleepMS(sleep);
    methods_called++;
  }

  EXPECT_EQ(methods_called, methods_executed);
  EXPECT_EQ(methods_called, responses_executed);

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* ClientServerBaseAsyncCallbackTest */

#if ClientServerBaseAsyncCallbackTimeoutTest

TEST(IO, ClientServerBaseAsyncCallbackTimeout)
{
  const int calls(1);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "clientserver base async callback test with timeout");

  // create service server
  eCAL::CServiceServer server("service");

  // method callback function
  int methods_executed(0);
  int method_process_time(0);
  auto method_callback = [&](const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_) -> int
  {
    eCAL::Process::SleepMS(method_process_time);
    PrintRequest(method_, req_type_, resp_type_, request_);
    response_ = "I answered on " + request_;
    methods_executed++;
    return 42;
  };

  // add callback for client request
  server.AddMethodCallback("foo::method1", "foo::req_type1", "foo::resp_type1", method_callback);
  server.AddMethodCallback("foo::method2", "foo::req_type2", "foo::resp_type2", method_callback);

  // create service client
  eCAL::CServiceClient client("service");

  // response callback function
  int responses_executed(0);
  auto response_callback = [&](const struct eCAL::SServiceResponse& service_response_)
  {
    PrintResponse(service_response_);
    responses_executed++;
  };

  // add callback for server response
  client.AddResponseCallback(response_callback);

  // let's match them -> wait REGISTRATION_REFRESH_CYCLE (ecal_def.h)
  eCAL::Process::SleepMS(2000);

  // call service
  int methods_called(0);
  std::string m1("foo::method1");
  std::string m2("foo::method2");
  std::string r1("my request for method 1");
  std::string r2("my request for method 2");

  // some calls with method_process_time sleep in the method callback
  method_process_time = 100;
  for (auto i = 0; i < calls; ++i)
  {
    // call method 1
    client.CallAsync(m1, r1);
    eCAL::Process::SleepMS(method_process_time * 2);
    methods_called++;

    // call method 2
    client.CallAsync(m2, r2);
    eCAL::Process::SleepMS(method_process_time * 2);
    methods_called++;
  }

  eCAL::Process::SleepMS(1000);

  EXPECT_EQ(methods_called, methods_executed);
  EXPECT_EQ(methods_called, responses_executed);

  // call the same method with method_process_time sleep in the method callback
  // and less than method_process_time sleep time between
  // the calls -> second call should be blocked and fail
  methods_called      = 0;
  methods_executed    = 0;
  responses_executed  = 0;

  method_process_time = 100;
  for (auto i = 0; i < calls; ++i)
  {
    // call method 1
    client.CallAsync(m1, r1);
    eCAL::Process::SleepMS(method_process_time / 2);
    methods_called++;

    // call method 1 
    // this one should fail that means:
    //   server method is not called
    //   response is called with error message
    client.CallAsync(m1, r1);
    eCAL::Process::SleepMS(method_process_time / 2);
    methods_called++;
  }

  eCAL::Process::SleepMS(1000);

  EXPECT_EQ(methods_called/2, methods_executed);
  EXPECT_EQ(methods_called,   responses_executed);

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* ClientServerBaseAsyncCallbackTimeoutTest */

#if ClientServerBaseBlockingTest

TEST(IO, ClientServerBaseBlocking)
{
  const int num_services(2);
  const int num_clients(3);
  const int calls(1);
  const int sleep(0);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "clientserver base blocking test");

  // create service servers
  ServiceVecT service_vec;
  for (auto s = 0; s < num_services; ++s)
  {
    service_vec.push_back(std::make_shared<eCAL::CServiceServer>("service"));
  }

  // method callback function
  int methods_executed(0);
  auto method_callback = [&](const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_) -> int
  {
    PrintRequest(method_, req_type_, resp_type_, request_);
    response_ = "I answer on " + request_;
    methods_executed++;
    return 24;
  };

  // add method callback
  for (auto service : service_vec)
  {
    service->AddMethodCallback("foo::method1", "foo::req_type1", "foo::resp_type1", method_callback);
    service->AddMethodCallback("foo::method2", "foo::req_type2", "foo::resp_type2", method_callback);
  }

  // create service clients
  ClientVecT client_vec;
  for (auto s = 0; s < num_clients; ++s)
  {
    client_vec.push_back(std::make_shared<eCAL::CServiceClient>("service"));
  }

  // let's match them -> wait REGISTRATION_REFRESH_CYCLE (ecal_def.h)
  eCAL::Process::SleepMS(2000);

  // call service
  int methods_called(0);
  int responses_executed(0);
  eCAL::ServiceResponseVecT service_response_vec;
  for (auto i = 0; i < calls; ++i)
  {
    // call methods
    for (auto client : client_vec)
    {
      // call method 1
      if (client->Call("foo::method1", "my request for method 1", -1, &service_response_vec))
      {
        ASSERT_EQ(2, service_response_vec.size());

        PrintResponse(service_response_vec[0]);
        responses_executed++;

        PrintResponse(service_response_vec[1]);
        responses_executed++;

        eCAL::Process::SleepMS(sleep);
        methods_called++;
      }

      // call method 2
      if (client->Call("foo::method2", "my request for method 2", -1, &service_response_vec))
      {
        ASSERT_EQ(2, service_response_vec.size());

        PrintResponse(service_response_vec[0]);
        responses_executed++;

        PrintResponse(service_response_vec[1]);
        responses_executed++;

        eCAL::Process::SleepMS(sleep);
        methods_called++;
      }
    }
  }

  EXPECT_EQ(methods_called * num_services, methods_executed);
  EXPECT_EQ(methods_called * num_services, responses_executed);

  // remove method callback
  for (auto service : service_vec)
  {
    service->RemMethodCallback("foo::method1");
    service->RemMethodCallback("foo::method2");
  }

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* ClientServerBaseBlockingTest */

#if ClientServerProtoCallbackTest

///////////////////////////////////////////////
// Math Service
///////////////////////////////////////////////
class MathServiceImpl : public MathService
{
public:
  virtual void Add(::google::protobuf::RpcController* /* controller_ */, const ::SFloatTuple* request_, ::SFloat* response_, ::google::protobuf::Closure* /* done_ */)
  {
    // print request and
    std::cout << "Received request MathService / Add      : " << request_->inp1() << " and " << request_->inp2() << std::endl << std::endl;
    // create response
    response_->set_out(request_->inp1() + request_->inp2());
  }

  virtual void Multiply(::google::protobuf::RpcController* /* controller_ */, const ::SFloatTuple* request_, ::SFloat* response_, ::google::protobuf::Closure* /* done_ */)
  {
    // print request and
    std::cout << "Received request MathService / Multiply : " << request_->inp1() << " and " << request_->inp2() << std::endl << std::endl;
    // create response
    response_->set_out(request_->inp1() * request_->inp2());
  }

  virtual void Divide(::google::protobuf::RpcController* /* controller_ */, const ::SFloatTuple* request_, ::SFloat* response_, ::google::protobuf::Closure* /* done_ */)
  {
    // print request and
    std::cout << "Received request MathService / Divide   : " << request_->inp1() << " and " << request_->inp2() << std::endl << std::endl;
    // create response
    if (std::fabs(request_->inp2()) > DBL_EPSILON) response_->set_out(request_->inp1() / request_->inp2());
    else                                           response_->set_out(0.0);
  }
};

TEST(IO, ClientServerProtoCallback)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "clientserver proto callback test");

  // create MathService server
  std::shared_ptr<MathServiceImpl> math_service_impl = std::make_shared<MathServiceImpl>();
  eCAL::protobuf::CServiceServer<MathService> math_server(math_service_impl);

  // create MathService client
  eCAL::protobuf::CServiceClient<MathService> math_client;

  // response callback function
  double math_response(0.0);
  auto response_callback = [&](const struct eCAL::SServiceResponse& service_response_)
  {
    math_response = 0.0;
    switch (service_response_.call_state)
    {
    // service successful executed
    case call_state_executed:
    {
      SFloat response;
      response.ParseFromString(service_response_.response);
      std::cout << "Received response MathService / " << service_response_.method_name << " : " << response.out() << " from host " << service_response_.host_name << std::endl;
      math_response = response.out();
    }
    break;
    // service execution failed
    case call_state_failed:
      std::cout << "Received error MathService / " << service_response_.method_name << " : " << service_response_.error_msg << " from host " << service_response_.host_name << std::endl;
      break;
    default:
      break;
    }
  };

  // add callback for server response
  math_client.AddResponseCallback(response_callback);

  // let's match them -> wait REGISTRATION_REFRESH_CYCLE (ecal_def.h)
  eCAL::Process::SleepMS(2000);

  // test math service
  SFloatTuple math_request;
  double inp1 = 20.0;
  double inp2 = 22.0;
  math_request.set_inp1(inp1);
  math_request.set_inp2(inp2);
  std::string math_request_s = math_request.SerializeAsString();

  math_response = 0.0;
  math_client.Call("Add", math_request_s);
  std::cout << std::endl << "Add method called with : " << math_request.inp1() << " and " << math_request.inp1() << std::endl;
  EXPECT_EQ(inp1 + inp2, static_cast<double>(math_response));

  math_response = 0.0;
  math_client.Call("Multiply", math_request_s);
  std::cout << std::endl << "Multiply method called with : " << math_request.inp1() << " and " << math_request.inp1() << std::endl;
  EXPECT_EQ(inp1 * inp2, static_cast<double>(math_response));

  math_response = 0.0;
  math_client.Call("Divide", math_request_s);
  std::cout << std::endl << "Divide method called with : " << math_request.inp1() << " and " << math_request.inp1() << std::endl;
  EXPECT_EQ(inp1 / inp2, static_cast<double>(math_response));

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* ClientServerProtoCallbackTest */

#if ClientServerProtoBlockingTest

///////////////////////////////////////////////
// Ping Service
///////////////////////////////////////////////
class PingServiceImpl : public PingService
{
public:
  void Ping(::google::protobuf::RpcController* /* controller_ */, const ::PingRequest* request_, ::PingResponse* response_, ::google::protobuf::Closure* /* done_ */)
  {
    // print request and
    std::cout << "Received request PingService / Ping     : " << request_->message() << std::endl << std::endl;
    // create response
    response_->set_answer("PONG");
  }
};

TEST(IO, ClientServerProtoBlocking)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "clientserver proto blocking test");

  // create PingService server
  std::shared_ptr<PingServiceImpl> ping_service_impl = std::make_shared<PingServiceImpl>();
  eCAL::protobuf::CServiceServer<PingService> ping_server(ping_service_impl);

  // create PingService client
  eCAL::protobuf::CServiceClient<PingService> ping_client;

  // let's match them -> wait REGISTRATION_REFRESH_CYCLE (ecal_def.h)
  eCAL::Process::SleepMS(2000);

  // test ping service
  eCAL::ServiceResponseVecT service_response_vec;
  PingRequest ping_request;
  ping_request.set_message("PING");
  ping_client.Call("Ping", ping_request, -1, &service_response_vec);
  std::cout << std::endl << "Ping method called with message : " << ping_request.message() << std::endl;
  for (auto service_response : service_response_vec)
  {
    EXPECT_EQ(call_state_executed, service_response.call_state);
    switch (service_response.call_state)
    {
      // service successful executed
    case call_state_executed:
    {
      PingResponse response;
      response.ParseFromString(service_response.response);
      std::cout << "Received response PingService / Ping : " << response.answer() << " from host " << service_response.host_name << std::endl;
      EXPECT_STREQ(response.answer().c_str(), "PONG");
    }
    break;
    // service execution failed
    case call_state_failed:
      std::cout << "Received error PingService / Ping : " << service_response.error_msg << " from host " << service_response.host_name << std::endl;
      break;
    default:
      break;
    }
  }

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* ClientServerProtoBlockingTest */

#if NestedRPCCallTest

TEST(IO, NestedRPCCall)
{
  const int calls(1);
  const int sleep(0);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "nested rpc call test");

  // create service server
  eCAL::CServiceServer server("service");

  // request callback function
  int methods_executed(0);
  auto method_callback = [&](const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_) -> int
  {
    PrintRequest(method_, req_type_, resp_type_, request_);
    response_ = "I answer on " + request_;
    methods_executed++;
    return 42;
  };

  // add callback for client request
  server.AddMethodCallback("foo::method1", "foo::req_type1", "foo::resp_type1", method_callback);
  server.AddMethodCallback("foo::method2", "foo::req_type2", "foo::resp_type2", method_callback);

  // create service client
  eCAL::CServiceClient client1("service");
  eCAL::CServiceClient client2("service");

  // response callback function
  int methods_called(0);
  int responses_executed(0);
  bool success(true);
  auto response_callback1 = [&](const struct eCAL::SServiceResponse& service_response_)
  {
    PrintResponse(service_response_);
    success &= client2.Call("foo::method2", "my request for method 2");
    methods_called++;
    responses_executed++;
  };
  auto response_callback2 = [&](const struct eCAL::SServiceResponse& service_response_)
  {
    PrintResponse(service_response_);
    responses_executed++;
  };

  // add callback for server response
  client1.AddResponseCallback(response_callback1);
  client2.AddResponseCallback(response_callback2);

  // let's match them -> wait REGISTRATION_REFRESH_CYCLE (ecal_def.h)
  eCAL::Process::SleepMS(2000);

  // call service
  for (auto i = 0; i < calls; ++i)
  {
    // call method 1
    success &= client1.Call("foo::method1", "my request for method 1");
    eCAL::Process::SleepMS(sleep);
    methods_called++;
  }

  EXPECT_EQ(true, success);
  EXPECT_EQ(methods_called, methods_executed);
  EXPECT_EQ(methods_called, responses_executed);

  // remove method callback
  server.RemMethodCallback("foo::method1");
  server.RemMethodCallback("foo::method2");

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* NestedRPCCallTest */
