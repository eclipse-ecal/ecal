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

void printCallbackInformation(const eCAL::SServiceMethodInformation& method_info_, const std::string& request_, const std::string& response_)
{
  std::cout << "Method   : '" << method_info_.method_name << "' called in C++" << "\n";
  std::cout << "Request  : " << request_ << "\n";
  std::cout << "Response : " << response_ << "\n";
  std::cout << "\n";
}

/*
  We define the callback function that will be called when a client calls the service method "echo".
  This callback will simply return the request as the response.
*/
int OnEchoCallback(const eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_)
{
  response_ = request_;

  printCallbackInformation(method_info_, request_, response_);
  
  // The return value here has no actual meaning. It's made available to the caller.
  return 0;
}

/* 
  This callback will be called when a client calls the service method "reverse".
  It will return the request in reverse order as the response.
*/
int OnReverseCallback(const eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_)
{
  response_.resize(request_.size());
  std::copy(request_.rbegin(), request_.rend(), response_.begin());

  printCallbackInformation(method_info_, request_, response_);

  // The return value here has no actual meaning. It's made available to the caller.
  return 0;
}

int main()
{
  std::cout << "--------------------" << "\n";
  std::cout << " C++: MIRROR SERVER"  << "\n";
  std::cout << "--------------------" << "\n";

  /*
    As always: initialize the eCAL API and give your process a name.
  */
  eCAL::Initialize("mirror server");

  std::cout << "eCAL " << eCAL::GetVersionString() << " (" << eCAL::GetVersionDateString() << ")" << "\n";
  eCAL::Process::SetState(eCAL::Process::eSeverity::healthy, eCAL::Process::eSeverityLevel::level1, "I feel good!");

  /*
    Now we create the mirror server and give it the name "mirror".
  */
  eCAL::CServiceServer mirror_server("mirror");

  /*
    The server will have two methods: "echo" and "reverse".
    To set a callback, we need to set a ServiceMethodInformation struct as well as the callback function.
    We simplify the struct creation of ServiceMethodInformationStruct and the other two
    fields can be left empty for our example.
  */
  mirror_server.SetMethodCallback({ "echo", {}, {} }, OnEchoCallback);
  mirror_server.SetMethodCallback({ "reverse", {}, {} }, OnReverseCallback);

  /*
    Now we will go in an infinite loop, to wait for incoming service calls that will be handled with the callbacks.
  */
  while(eCAL::Ok())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  /*
    After we are done, as always, finalize the eCAL API.
  */
  eCAL::Finalize();

  return(0);
}
