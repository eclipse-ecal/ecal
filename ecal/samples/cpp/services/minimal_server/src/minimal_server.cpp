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

#include <iostream>
#include <chrono>
#include <thread>

// method callback
int OnMethodCallback(const eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_)
{
  std::cout << "Method called : " << method_info_.method_name << std::endl;
  std::cout << "Request       : " << request_ << std::endl << std::endl;
  response_ = request_;
  return 42;
}

// main entry
int main()
{
  // initialize eCAL API
  eCAL::Initialize("minimal server");

  // create minimal service server
  eCAL::CServiceServer minimal_server("service1");

  // add method callback
  minimal_server.SetMethodCallback({ "echo", {}, {} }, OnMethodCallback);

  // idle
  while(eCAL::Ok())
  {
    // sleep 100 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
