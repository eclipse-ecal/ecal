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

#pragma once

#include "math.pb.h"
#include <cmath>
#include <iostream>

class MathServiceImpl : public MathService
{
public:
  // Implements the "Add" RPC.
  void Add(::google::protobuf::RpcController* /*controller*/,
           const SFloatTuple* request,
           SFloat* response,
           ::google::protobuf::Closure* done) override
  {
    response->set_out(request->inp1() + request->inp2());

    // do some delay to simulate a slow service
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (done)
      done->Run();
  }

  // Implements the "Multiply" RPC.
  void Multiply(::google::protobuf::RpcController* /*controller*/,
                const SFloatTuple* request,
                SFloat* response,
                ::google::protobuf::Closure* done) override
  {
    response->set_out(request->inp1() * request->inp2());

    // do some delay to simulate a slow service
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (done)
      done->Run();
  }

  // Implements the "Divide" RPC.
  void Divide(::google::protobuf::RpcController* /*controller*/,
              const SFloatTuple* request,
              SFloat* response,
              ::google::protobuf::Closure* done) override
  {
    if (std::fabs(request->inp2()) > DBL_EPSILON)
      response->set_out(request->inp1() / request->inp2());
    else
      response->set_out(0.0);
      
    // do some delay to simulate a slow service
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    if (done)
      done->Run();
  }
};
