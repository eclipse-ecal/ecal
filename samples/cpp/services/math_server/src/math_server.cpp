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
#include <ecal/msg/protobuf/server.h>

#include <iostream>
#include <chrono>
#include <thread>
#include <math.h>
#include <cfloat>
#include <cmath>

#include "math.pb.h"

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
    if(std::fabs(request_->inp2()) > DBL_EPSILON) response_->set_out(request_->inp1() / request_->inp2());
    else                                          response_->set_out(0.0);
  }
};

// main entry
int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "math server");

  // create Math service server
  std::shared_ptr<MathService> math_service = std::make_shared<MathServiceImpl>();
  eCAL::protobuf::CServiceServer<MathService> math_server(math_service);

  while(eCAL::Ok())
  {
    // sleep 100 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
