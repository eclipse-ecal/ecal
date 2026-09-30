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

// Include the eCAL convenience header
#include <ecal/ecal.h>

#include <algorithm>
#include <iostream>
#include <chrono>
#include <random>

/*
  Some helper function to generate binary data into a buffer.
  Clears the vector, resizes it to a specified size and fills it with random printable ascii characters.
*/
void fillBinaryBuffer(std::vector<unsigned char>& buffer) { 
  constexpr unsigned int buffer_size = 16;

  static std::random_device random_device;
  static std::mt19937 generator(random_device());
  // Useful random characters are in the range [32, 126]
  static std::uniform_int_distribution<> printable_ascii_char(32, 126);
  
  buffer.clear();
  buffer.resize(buffer_size);

  for (unsigned int i = 0; i < buffer_size; ++i) {
    buffer[i] = static_cast<char>(printable_ascii_char(generator));
  }
}

int main()
{
  std::cout << "-------------------" << "\n";
  std::cout << " C++: BLOB SENDER"   << "\n";
  std::cout << "-------------------" << "\n";

  /*
    Initialize eCAL. You always have to initialize eCAL before using its API.
    The name of our eCAL Process will be "blob send". 
    This name will be visible in the eCAL Monitor, once the process is running.
  */
  eCAL::Initialize("blob send");

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
    Now we create a new publisher that will publish the topic "blob".
  */
  eCAL::CPublisher pub("blob");

  /*
    Construct a message. As we are sending binary data, we just create a buffer of unsigned characters.
  */
  std::vector<unsigned char> binary_buffer;

  /*
    Creating an infinite publish-loop.
    eCAL Supports a stop signal; when an eCAL Process is stopped, eCAL_Ok() will return false.
  */
  while (eCAL::Ok())
  {
    /*
      Fill the buffer with the character that is defined by the counter variable.
    */
    fillBinaryBuffer(binary_buffer);

    /*
      Send the message. The message is sent to all subscribers that are currently connected to the topic "blob".
      For binary data you need to set a buffer pointer and the size of the buffer.
    */
    if (pub.Send(binary_buffer.data(), binary_buffer.size()))
      std::cout << "Sent binary data in C++: " << std::string(binary_buffer.begin(), binary_buffer.end()) << "\n";
    else
      std::cout << "Sending binary data in C++ failed!" << "\n";

    /*
      Sleep for 500ms to send in a frequency of 2 hz.
    */
    eCAL::Process::SleepMS(500);
  }

  /*
    Finalize eCAL. This will stop all eCAL processes and free all resources.
    You should always finalize eCAL when you are done using it.
  */
  eCAL::Finalize();

  return(0);
}
