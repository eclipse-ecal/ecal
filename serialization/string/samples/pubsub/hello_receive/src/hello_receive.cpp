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

// Include the basic eCAL header
#include <ecal/ecal.h>
/* 
  We want to receive raw strings, so wie include the string subscriber header.
  eCAL supports multiple message formats.
*/
#include <ecal/msg/string/subscriber.h>

#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

int main()
{
  std::cout << "-------------------------------" << std::endl;
  std::cout << " C++: HELLO WORLD RECEIVER"      << std::endl;
  std::cout << "-------------------------------" << std::endl;

  /* 
    Initialize eCAL. You always have to initialize eCAL beforee using its API.
    The name of our eCAL Process will be "hello_receive". 
    This name will be visible in the eCAL Monitor, once the process is running.
  */
  eCAL::Initialize("hello_receive");

  /*
    Creating the eCAL Subscriber. An eCAL Process can create multiple subscribers (and publishers).
    The topic we are going to receive is called "hello".
  */
  eCAL::string::CSubscriber sub("hello");

  /*
    Creating a receive callback. The callback will be called whenever a new message is received.
  */
  auto msg_cb = [](const std::string& msg_) { std::cout << "Received \"" << msg_ << "\"" << std::endl; };
  
  /*
    Register the callback with the subscriber so it can be called.
  */
  sub.SetReceiveCallback(std::bind(msg_cb, std::placeholders::_2));

  /*
    Creating an infinite loop.
    eCAL Supports a stop signal; when an eCAL Process is stopped, eCAL::Ok() will return false.
  */
  while (eCAL::Ok()) std::this_thread::sleep_for(std::chrono::milliseconds(500));

  /*
    Deinitialize eCAL.
    You should always do that before your application exits.
  */
  eCAL::Finalize();

  return(0);
}
