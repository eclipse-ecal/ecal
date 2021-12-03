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

#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>

#include <gtest/gtest.h>

#include "math.pb.h"
#include "ping.pb.h"

#define ClientServerBaseCallbackTest       1
#define ClientServerBaseAsyncCallbackTest  0   // still crashing

#define ClientServerBaseBlockingTest       1
#define ClientServerProtoCallbackTest      1
#define ClientServerProtoBlockingTest      1

#define MultipleServerTest                 1
#define NestedRPCCallTest                  1

namespace
{
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

#if ClientServerBaseCallbackTest

TEST(IO, ClientServerBaseCallback)
{
  const int calls(1);
  const int sleep(0);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "clientserver base callback test");

  // create service server
  eCAL::CServiceServer server("service1");

  // request callback function
  int requests_executed(0);
  auto request_callback = [&](const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_) -> int
  {
    PrintRequest(method_, req_type_, resp_type_, request_);
    response_ = "I answer on " + request_;
    requests_executed++;
    return 42;
  };

  // add callback for client request
  server.AddMethodCallback("foo::method1", "foo::req_type1", "foo::resp_type1", request_callback);
  server.AddMethodCallback("foo::method2", "foo::req_type2", "foo::resp_type2", request_callback);

  // create service client
  eCAL::CServiceClient client("service1");

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
  int requests_called(0);
  bool success(true);
  for (auto i = 0; i < calls; ++i)
  {
    // call a method1
    success &= client.Call("foo::method1", "my request for method 1");
    eCAL::Process::SleepMS(sleep);
    requests_called++;

    // call a method2
    success &= client.Call("foo::method2", "my request for method 2");
    eCAL::Process::SleepMS(sleep);
    requests_called++;
  }

  EXPECT_EQ(true, success);
  EXPECT_EQ(requests_called, requests_executed);
  EXPECT_EQ(requests_called, responses_executed);

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* ClientServerBaseCallbackTest */

#if ClientServerBaseAsyncCallbackTest

TEST(IO, ClientServerBaseAsyncCallback)
{
  const int calls(1);
  const int sleep(10);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "clientserver base async callback test");

  // create service server
  eCAL::CServiceServer server("service1");

  // request callback function
  int requests_executed(0);
  auto request_callback = [&](const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_) -> int
  {
    PrintRequest(method_, req_type_, resp_type_, request_);
    response_ = "I answer on " + request_;
    requests_executed++;
    return 42;
  };

  // add callback for client request
  server.AddMethodCallback("foo::method1", "foo::req_type1", "foo::resp_type1", request_callback);
  server.AddMethodCallback("foo::method2", "foo::req_type2", "foo::resp_type2", request_callback);

  // create service client
  eCAL::CServiceClient client("service1");

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
  int requests_called(0);
  for (auto i = 0; i < calls; ++i)
  {
    // call a method1
    client.CallAsync("foo::method1", "my request for method 1", -1);
    eCAL::Process::SleepMS(sleep);
    requests_called++;

    // call a method2
    client.CallAsync("foo::method2", "my request for method 2", -1);
    eCAL::Process::SleepMS(sleep);
    requests_called++;
  }

  EXPECT_EQ(requests_called, requests_executed);
  EXPECT_EQ(requests_called, responses_executed);

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* ClientServerBaseAsyncCallbackTest */

#if ClientServerBaseBlockingTest

TEST(IO, ClientServerBaseBlocking)
{
  const int calls(1);
  const int sleep(0);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "clientserver base blocking test");

  // create service server
  eCAL::CServiceServer server("service1");

  // request callback function
  int requests_executed(0);
  auto request_callback = [&](const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_) -> int
  {
    PrintRequest(method_, req_type_, resp_type_, request_);
    response_ = "I answer on " + request_;
    requests_executed++;
    return 24;
  };

  // add callback for client request
  server.AddMethodCallback("foo::method1", "foo::req_type1", "foo::resp_type1", request_callback);
  server.AddMethodCallback("foo::method2", "foo::req_type2", "foo::resp_type2", request_callback);

  // create service client
  eCAL::CServiceClient client("service1");

  // let's match them -> wait REGISTRATION_REFRESH_CYCLE (ecal_def.h)
  eCAL::Process::SleepMS(2000);

  // call service
  int requests_called(0);
  int responses_executed(0);
  for (auto i = 0; i < calls; ++i)
  {
    eCAL::ServiceResponseVecT service_response_vec;
    // call a method1
    if (client.Call("foo::method1", "my request for method 1", -1, &service_response_vec))
    {
      ASSERT_EQ(1, service_response_vec.size());
      PrintResponse(service_response_vec[0]);
      responses_executed++;
    }
    eCAL::Process::SleepMS(sleep);
    requests_called++;

    // call a method2
    if (client.Call("foo::method2", "my request for method 2", -1, &service_response_vec))
    {
      ASSERT_EQ(1, service_response_vec.size());
      PrintResponse(service_response_vec[0]);
      responses_executed++;
    }
    eCAL::Process::SleepMS(sleep);
    requests_called++;
  }

  EXPECT_EQ(requests_called, requests_executed);
  EXPECT_EQ(requests_called, responses_executed);

  // remove server request callback
  server.RemMethodCallback("foo::method1");
  server.RemMethodCallback("foo::method2");

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
  EXPECT_EQ(inp1*inp2, static_cast<double>(math_response));

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
  std::string ping_request_s = ping_request.SerializeAsString();
  std::cout << std::endl << "Ping method called with message : " << ping_request.message() << std::endl;
  ping_client.Call("Ping", ping_request_s, -1, &service_response_vec);
  ASSERT_EQ(1, service_response_vec.size());
  EXPECT_EQ(call_state_executed, service_response_vec[0].call_state);
  switch (service_response_vec[0].call_state)
  {
  // service successful executed
  case call_state_executed:
  {
    PingResponse response;
    response.ParseFromString(service_response_vec[0].response);
    std::cout << "Received response PingService / Ping : " << response.answer() << " from host " << service_response_vec[0].host_name << std::endl;
    EXPECT_STREQ(response.answer().c_str(), "PONG");
  }
  break;
  // service execution failed
  case call_state_failed:
    std::cout << "Received error PingService / Ping : " << service_response_vec[0].error_msg << " from host " << service_response_vec[0].host_name << std::endl;
    break;
  default:
    break;
  }

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* ClientServerProtoBlockingTest */

#if MultipleServerTest

TEST(IO, MultipleServerTest)
{
  const int calls(1);
  const int sleep(0);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "multiple server test");

  // create service server
  eCAL::CServiceServer server1("service1");
  eCAL::CServiceServer server2("service2");

  // request callback function
  int requests_executed(0);
  auto request_callback = [&](const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_) -> int
  {
    PrintRequest(method_, req_type_, resp_type_, request_);
    response_ = "I answer on " + request_;
    requests_executed++;
    return 42;
  };

  // add callback for client request
  server1.AddMethodCallback("foo::method1", "foo::req_type1", "foo::resp_type1", request_callback);
  server2.AddMethodCallback("foo::method2", "foo::req_type2", "foo::resp_type2", request_callback);

  // create service clients
  eCAL::CServiceClient client1("service1");
  eCAL::CServiceClient client2("service2");

  // response callback function
  int responses_executed(0);
  auto response_callback = [&](const struct eCAL::SServiceResponse& service_response_)
  {
    PrintResponse(service_response_);
    responses_executed++;
  };

  // add callback for server response
  client1.AddResponseCallback(response_callback);
  client2.AddResponseCallback(response_callback);

  // let's match them -> wait REGISTRATION_REFRESH_CYCLE (ecal_def.h)
  eCAL::Process::SleepMS(2000);

  // call service
  int requests_called(0);
  bool success(true);
  for (auto i = 0; i < calls; ++i)
  {
    // call a method1
    success &= client1.Call("foo::method1", "my request for method 1");
    eCAL::Process::SleepMS(sleep);
    requests_called++;

    // call a method2
    success &= client2.Call("foo::method2", "my request for method 2");
    eCAL::Process::SleepMS(sleep);
    requests_called++;
  }

  EXPECT_EQ(true, success);
  EXPECT_EQ(requests_called, requests_executed);
  EXPECT_EQ(requests_called, responses_executed);

  // remove server request callback
  server1.RemMethodCallback("foo::method1");
  server2.RemMethodCallback("foo::method2");

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* MultipleServerTest */

#if NestedRPCCallTest

TEST(IO, NestedRPCCall)
{
  const int calls(1);
  const int sleep(0);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "nested rpc call test");

  // create service server
  eCAL::CServiceServer server("service1");

  // request callback function
  int requests_executed(0);
  auto request_callback = [&](const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_) -> int
  {
    PrintRequest(method_, req_type_, resp_type_, request_);
    response_ = "I answer on " + request_;
    requests_executed++;
    return 42;
  };

  // add callback for client request
  server.AddMethodCallback("foo::method1", "foo::req_type1", "foo::resp_type1", request_callback);
  server.AddMethodCallback("foo::method2", "foo::req_type2", "foo::resp_type2", request_callback);

  // create service client
  eCAL::CServiceClient client1("service1");
  eCAL::CServiceClient client2("service1");

  // response callback function
  int requests_called(0);
  int responses_executed(0);
  bool success(true);
  auto response_callback1 = [&](const struct eCAL::SServiceResponse& service_response_)
  {
    PrintResponse(service_response_);
    success &= client2.Call("foo::method2", "my request for method 2");
    requests_called++;
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
    // call a method1
    success &= client1.Call("foo::method1", "my request for method 1");
    eCAL::Process::SleepMS(sleep);
    requests_called++;
  }

  EXPECT_EQ(true, success);
  EXPECT_EQ(requests_called, requests_executed);
  EXPECT_EQ(requests_called, responses_executed);

  // remove server request callback
  server.RemMethodCallback("foo::method1");
  server.RemMethodCallback("foo::method2");

  // finalize eCAL API
  eCAL::Finalize();
}

#endif /* NestedRPCCallTest */
