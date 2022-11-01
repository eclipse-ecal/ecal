/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2022 Eclipse Foundation
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

#include "orchestrator.pb.h"

#include <iostream>
#include <thread>
#include <chrono>

void main(int argc, char** argv)
{
  eCAL::Initialize(argc, argv, "orchestrator");

  eCAL::protobuf::CServiceClient<orchestrator::ComponentService> component1("component1");
  eCAL::protobuf::CServiceClient<orchestrator::ComponentService> component2("component2");

  std::this_thread::sleep_for(std::chrono::seconds(2));

  eCAL::ServiceResponseVecT srv_response_vec;
  orchestrator::request     request;

  uint64_t cycle = 0;
  while (eCAL::Ok())
  {
    request.set_id(cycle);

    std::cout << "call component 1" << std::endl;
    component1.Call("execute", request, -1, &srv_response_vec);

    std::cout << "call component 2" << std::endl << std::endl;
    component2.Call("execute", request, -1, &srv_response_vec);

    ++cycle;
  }

  eCAL::Finalize();
}
