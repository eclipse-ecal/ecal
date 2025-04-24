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
  std::cout << "----------------------------" << "\n";
  std::cout << " C++: HELLO WORLD RECEIVER"   << "\n";
  std::cout << "----------------------------" << "\n";

  /* 
    Initialize eCAL. You always have to initialize eCAL before using its API.
    The name of our eCAL Process will be "hello receive". 
    This name will be visible in the eCAL Monitor, once the process is running.
  */
  eCAL::Initialize("hello receive");

  /*
    Print some eCAL version information.
  */
  std::cout << "eCAL " << eCAL::GetVersionString() << " (" << eCAL::GetVersionDateString() << ")" << "\n";

  /*
    Set the state for the program.
    You can vary between different states like healthy, warning, critical ...
    This can be used to communicate the application state to applications like eCAL Monitor/Sys.
  */
  eCAL::Process::SetState(eCAL::Process::eSeverity::healthy, eCAL::Process::eSeverityLevel::level1, "I feel good!");

  /*
    Creating the eCAL Subscriber. An eCAL Process can create multiple subscribers (and publishers).
    The topic we are going to receive is called "hello".
  */
  eCAL::string::CSubscriber subscriber("hello");

  /*
    Creating a receive callback. The callback will be called whenever a new message is received.
  */
  auto message_callback = [](const eCAL::STopicId& publisher_id_, const std::string& message_, long long time_, long long clock_) { 
    std::cout << "---------------------------------------------------"                                << "\n";
    std::cout << " Received string message from topic \"" << publisher_id_.topic_name << "\" in C++ " << "\n";
    std::cout << "---------------------------------------------------"                                << "\n";
    std::cout << " Size    : " << message_.size()                                                     << "\n";
    std::cout << " Time    : " << time_                                                               << "\n";
    std::cout << " Clock   : " << clock_                                                              << "\n";
    std::cout << " Message : " << message_                                                            << "\n";
    std::cout << "\n";
  };
  
  /*
    Register the callback with the subscriber so it can be called.
  */
  subscriber.SetReceiveCallback(message_callback);

  /*
    Creating an infinite loop.
    eCAL Supports a stop signal; when an eCAL Process is stopped, eCAL::Ok() will return false.
  */
  while (eCAL::Ok())
  {
    /*
      Sleep for 500ms to avoid busy waiting.
      You can use eCAL::Process::SleepMS() to sleep in milliseconds.
    */
    eCAL::Process::SleepMS(500);
  }

  /*
    Deinitialize eCAL.
    You should always do that before your application exits.
  */
  eCAL::Finalize();

  return(0);
}
