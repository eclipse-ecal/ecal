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

int main()
{
  // initialize eCAL API
  eCAL::Initialize("orchestrator");

  const eCAL::protobuf::CServiceClientTyped<orchestrator::ComponentService> component1("component1");
  const eCAL::protobuf::CServiceClientTyped<orchestrator::ComponentService> component2("component2");

  // sleep 2 seconds
  eCAL::Process::SleepMS(2000);

  // prepare service request and response vector
  orchestrator::request srv_request;

  // call components 1 and 2
  uint64_t cycle = 0;
  while (eCAL::Ok())
  {
    srv_request.set_id(cycle);

    std::cout << "call component 1" << std::endl;
    component1.CallWithResponse<orchestrator::request, orchestrator::response>("execute", srv_request);

    std::cout << "call component 2" << std::endl << std::endl;
    component2.CallWithResponse<orchestrator::request, orchestrator::response>("execute", srv_request);

    ++cycle;
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
