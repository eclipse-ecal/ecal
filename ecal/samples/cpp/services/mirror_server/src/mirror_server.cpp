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

#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>

// Callback function that will echo back the data sent by the user
int OnEchoCallback(const eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_)
{
  response_ = request_;

  std::cout << "Method   : '" << method_info_.method_name << "' called" << std::endl;
  std::cout << "Request  : " << request_ << std::endl;
  std::cout << "Response : " << response_ << std::endl;
  std::cout << std::endl;
  
  // The return value here has no actual meaning. It's made available to the caller.
  return 0;
}

// Callback function that will reverse the data sent by the user
int OnReverseCallback(const eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_)
{
  response_.resize(request_.size());
  std::copy(request_.rbegin(), request_.rend(), response_.begin());

  std::cout << "Method   : '" << method_info_.method_name << "' called" << std::endl;
  std::cout << "Request  : " << request_ << std::endl;
  std::cout << "Response : " << response_ << std::endl;
  std::cout << std::endl;

  // The return value here has no actual meaning. It's made available to the caller.
  return 0;
}

// main entry
int main()
{
  // initialize eCAL API
  eCAL::Initialize("mirror server c++");

  // create binary service server
  eCAL::CServiceServer mirror_server("mirror");

  // this server offers two methods, 'echo' and 'reverse'
  mirror_server.SetMethodCallback({ "echo", {}, {} }, OnEchoCallback);
  mirror_server.SetMethodCallback({ "reverse", {}, {} }, OnReverseCallback);

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
