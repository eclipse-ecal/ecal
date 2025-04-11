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

#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

/*
  Here we create the subscriber callback function that is called everytime,
  when a new message arrived from a publisher.
*/
void OnReceive(const eCAL::STopicId& /*topic_id_*/, const eCAL::SDataTypeInformation& /*data_type_info_*/, const eCAL::SReceiveCallbackData& data_)
{
  if (data_.buffer_size < 1) return;

  int content(static_cast<int>(static_cast<const unsigned char*>(data_.buffer)[0]));
  std::cout << "----------------------------------------------" << std::endl;
  std::cout << " Received binary buffer " << content            << std::endl;
  std::cout << "----------------------------------------------" << std::endl;
  std::cout << " Size         : " << data_.buffer_size          << std::endl;
  std::cout << " Time         : " << data_.send_timestamp       << std::endl;
  std::cout << " Clock        : " << data_.send_clock           << std::endl;
  std::cout                                                     << std::endl;
}

int main()
{
  std::cout << "---------------------" << std::endl;
  std::cout << " C++: BLOB RECEIVER"   << std::endl;
  std::cout << "---------------------" << std::endl;

  /*
    Initialize eCAL. You always have to initialize eCAL before using its API.
    The name of our eCAL Process will be "blob_receive". 
    This name will be visible in the eCAL Monitor, once the process is running.
  */
  eCAL::Initialize("blob_receive");

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
    Creating the eCAL Subscriber. An eCAL Process can create multiple subscribers (and publishers).
    The topic we are going to receive is called "blob".
  */
  eCAL::CSubscriber sub("blob");

  /*
    Register a receive callback. The callback will be called whenever a new message is received.
  */
  sub.SetReceiveCallback(OnReceive);

  /*
    Creating an infinite loop.
    eCAL Supports a stop signal; when an eCAL Process is stopped, eCAL::Ok() will return false.
  */
  while (eCAL::Ok())
  {
    /*
      Sleep for 500ms to avoid busy waiting.
    */
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  /*
    Finalize eCAL. This will stop all eCAL processes and free all resources.
    You should always finalize eCAL before exiting your application.
  */
  eCAL::Finalize();

  return(0);
}
