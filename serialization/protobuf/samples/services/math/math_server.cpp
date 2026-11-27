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
#include <ecal/msg/protobuf/server.h>

#include <iostream>
#include <chrono>
#include <thread>
#include <math.h>
#include <cfloat>
#include <cmath>

#include "math.pb.h"

/*
  Here we derive our service implementation from the generated protobuf service class.abort
  We override the methods Add, Multiplay and Divide which we defined in the protobuf file.
*/
class MathServiceImpl : public MathService
{
public:
  void Add(::google::protobuf::RpcController* /* controller_ */, const ::SFloatTuple* request_, ::SFloat* response_, ::google::protobuf::Closure* /* done_ */) override
  {
    std::cout << "Received request for MathService in C++: Add" << "\n";
    std::cout << "Input1 : " << request_->inp1() << "\n";
    std::cout << "Input2 : " << request_->inp2() << "\n";
    std::cout << "\n";

    /*
      The request is a pointer to the protobuf message SFloatTuple.
      We can access the input values inp1 and inp2 using the generated getter methods.
      The response is a pointer to the protobuf message SFloat.
      We set the output value using the generated setter method set_out.

      This is very convenient, because we can simply work with the protobuf messages without additional function calls.
    */
    response_->set_out(request_->inp1() + request_->inp2());
  }

  void Multiply(::google::protobuf::RpcController* /* controller_ */, const ::SFloatTuple* request_, ::SFloat* response_, ::google::protobuf::Closure* /* done_ */) override
  {
    std::cout << "Received request for MathService in C++: Multiply" << "\n";
    std::cout << "Input1 : " << request_->inp1() << "\n";
    std::cout << "Input2 : " << request_->inp2() << "\n";
    std::cout << "\n";

    response_->set_out(request_->inp1() * request_->inp2());
  }

  void Divide(::google::protobuf::RpcController* /* controller_ */, const ::SFloatTuple* request_, ::SFloat* response_, ::google::protobuf::Closure* /* done_ */) override
  {
    std::cout << "Received request for MathService in C++: Divide" << "\n";
    std::cout << "Input1 : " << request_->inp1() << "\n";
    std::cout << "Input2 : " << request_->inp2() << "\n";
    std::cout << "\n";

    if(std::fabs(request_->inp2()) > DBL_EPSILON) response_->set_out(request_->inp1() / request_->inp2());
    else                                          response_->set_out(0.0);
  }
};

/*
  We define a callback function that will be called when the an event happens on the server, like connected or disconnected.
*/
void OnServerEvent(const eCAL::SServiceId& /*service_id_*/, const struct eCAL::SServerEventCallbackData& data_)
{
  switch (data_.type)
  {
  case eCAL::eServerEvent::connected:
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Server connected                   " << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    break;
  case eCAL::eServerEvent::disconnected:
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Server disconnected                " << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    break;
  default:
    std::cout << "Unknown server event." << std::endl;
    break;
  }
}

int main()
{
  /*
    As always: initialize the eCAL API and give your process a name.
  */
  eCAL::Initialize("math server");

  /*
    Here we need to create a shared pointer to our service implementation class.
  */
  std::shared_ptr<MathService> math_service = std::make_shared<MathServiceImpl>();
  
  /*
    Now we create the math server and give as parameter the service shared pointer.
    Additionally we want to listen to server events, so we pass the callback function OnServerEvent.
  */
  eCAL::protobuf::CServiceServer<MathService> math_server(math_service, OnServerEvent);

  while(eCAL::Ok())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  /*
    As always, when we are done, finalize the eCAL API.
  */
  eCAL::Finalize();

  return(0);
}
