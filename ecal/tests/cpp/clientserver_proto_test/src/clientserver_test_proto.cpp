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

#define ClientServerProtoCallbackTest 1
#define ClientServerProtoBlockingTest 1

#if ClientServerProtoCallbackTest

///////////////////////////////////////////////
// Math Service
///////////////////////////////////////////////
class MathServiceImpl : public MathService
{
public:
  void Add(::google::protobuf::RpcController* /* controller_ */, const ::SFloatTuple* request_, ::SFloat* response_, ::google::protobuf::Closure* /* done_ */) override
  {
    // print request and
    std::cout << "Received request MathService / Add      : " << request_->inp1() << " and " << request_->inp2() << std::endl << std::endl;
    // create response
    response_->set_out(request_->inp1() + request_->inp2());
  }

  void Multiply(::google::protobuf::RpcController* /* controller_ */, const ::SFloatTuple* request_, ::SFloat* response_, ::google::protobuf::Closure* /* done_ */) override
  {
    // print request and
    std::cout << "Received request MathService / Multiply : " << request_->inp1() << " and " << request_->inp2() << std::endl << std::endl;
    // create response
    response_->set_out(request_->inp1() * request_->inp2());
  }

  void Divide(::google::protobuf::RpcController* /* controller_ */, const ::SFloatTuple* request_, ::SFloat* response_, ::google::protobuf::Closure* /* done_ */) override
  {
    // print request and
    std::cout << "Received request MathService / Divide   : " << request_->inp1() << " and " << request_->inp2() << std::endl << std::endl;
    // create response
    if (std::fabs(request_->inp2()) > DBL_EPSILON) response_->set_out(request_->inp1() / request_->inp2());
    else                                           response_->set_out(0.0);
  }
};

TEST(core_cpp_clientserver_proto, ProtoCallback)
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
  void Ping(::google::protobuf::RpcController* /* controller_ */, const ::PingRequest* request_, ::PingResponse* response_, ::google::protobuf::Closure* /* done_ */) override
  {
    // print request and
    std::cout << "Received request PingService / Ping     : " << request_->message() << std::endl << std::endl;
    // create response
    response_->set_answer("PONG");
  }
};

TEST(core_cpp_clientserver_proto, ProtoBlocking)
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
  for (const auto& service_response : service_response_vec)
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
