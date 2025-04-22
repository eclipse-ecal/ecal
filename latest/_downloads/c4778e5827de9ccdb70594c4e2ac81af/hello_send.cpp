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
  We want to publish raw strings, so wie include the string publisher header.
  eCAL supports multiple message formats.
*/
#include <ecal/msg/string/publisher.h>

#include <iostream>
#include <sstream>

int main()
{
  std::cout << "--------------------------" << std::endl;
  std::cout << " C++: HELLO WORLD SENDER"        << std::endl;
  std::cout << "--------------------------" << std::endl;

  /* 
    Initialize eCAL. You always have to initialize eCAL before using its API.
    The name of our eCAL Process will be "hello send". 
    This name will be visible in the eCAL Monitor, once the process is running.
  */
  eCAL::Initialize("hello send");

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
    Creating the eCAL Publisher. An eCAL Process can create multiple publishers (and subscribers).
    The topic we are going to publish is called "hello".
  */
  eCAL::string::CPublisher publisher("hello");

  /*
    Creating an inifite publish-loop.
    eCAL Supports a stop signal; when an eCAL Process is stopped, eCAL::Ok() will return false.
  */
  int loop_count = 0;
  while (eCAL::Ok())
  {
    /*
      Build the string you want to send, using a stringstream in this example.
    */
    std::stringstream message;
    message << "HELLO WORLD FROM C++" << " (" << ++loop_count << ")";

    /*
      Send the content to other eCAL Processes that have subscribed to the topic "hello".
    */
    if(publisher.Send(message.str()))
      std::cout << "Sent string message in C++ \"" << message.str() << "\"" << "\n";
    else
      std::cout << "Sending string message in C++ failed!" << "\n";

    /*
      Sleep for 500 ms so we send with a frequency of 2 Hz.
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
