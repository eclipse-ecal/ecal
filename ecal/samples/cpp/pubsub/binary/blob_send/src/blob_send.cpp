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

int main()
{
  std::cout << "-------------------" << std::endl;
  std::cout << " C++: BLOB SENDER"   << std::endl;
  std::cout << "-------------------" << std::endl;

  /*
    Initialize eCAL. You always have to initialize eCAL before using its API.
    The name of our eCAL Process will be "blob_send". 
    This name will be visible in the eCAL Monitor, once the process is running.
  */
  eCAL::Initialize("blob_send");

  /*
    Print some eCAL version information.
  */
  std::cout << "eCAL " << eCAL::GetVersionString() << " (" << eCAL::GetVersionDateString() << ")" << "\n";

  /*
    Set the state for the program.
    You can vary between different states like healthy, warning, critical ...
    This can be used to communicate the application state to applications like eCAL Monitor/Sys.
  */
  eCAL::Process::SetState(eCAL::Process::eSeverity::healthy, eCAL::Process::eSeverityLevel::level1, "I feel good !");

  /*
    Now we create a new publisher that will publish the topic "blob".
  */
  eCAL::CPublisher pub("blob");

  /*
    Construct a message. As we are sending binary data, we just create a buffer of unsigned characters.
  */
  std::vector<unsigned char> bin_buffer(1024);

  /*
    Creating an infinite publish-loop.
    eCAL Supports a stop signal; when an eCAL Process is stopped, eCAL_Ok() will return false.
  */
  unsigned char loop_count = 0;
  while (eCAL::Ok())
  {
    /*
      Fill the buffer with the character that is defined by the counter variable.
    */
    std::fill(bin_buffer.begin(), bin_buffer.end(), loop_count++);

    /*
      Send the message. The message is sent to all subscribers that are currently connected to the topic "blob".
    */
    pub.Send(bin_buffer.data(), bin_buffer.size());
    std::cout << "Sent buffer filled with " << static_cast<int>(loop_count) << std::endl;

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
