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

#include "ping.pb.h"
#include <iostream>

class PingServiceImpl : public PingService
{
public:
  // Implements the "Ping" RPC.
  void Ping(::google::protobuf::RpcController* /*controller*/,
            const PingRequest* /*request*/,
            PingResponse* response,
            ::google::protobuf::Closure* done) override
  {
    // For example, simply reply with "PONG".
    response->set_answer("PONG");
    if (done)
      done->Run();
  }
};
