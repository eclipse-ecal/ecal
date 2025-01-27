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
#include <ecal/msg/string/subscriber.h>

#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

int main()
{
  std::cout << "-------------------------------" << std::endl;
  std::cout << " HELLO WORLD RECEIVER"           << std::endl;
  std::cout << "-------------------------------" << std::endl;

  // initialize eCAL API
  eCAL::Initialize("minimal_rec_cb");

  // subscriber for topic "Hello"
  eCAL::string::CSubscriber<std::string> sub("Hello");

  // receive updates in a callback (lambda function)
  auto msg_cb = [](const std::string& msg_) { std::cout << "Received \"" << msg_ << "\"" << std::endl; };
  sub.SetReceiveCallback(std::bind(msg_cb, std::placeholders::_2));

  // idle main loop
  while (eCAL::Ok()) std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
