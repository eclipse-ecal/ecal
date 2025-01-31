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
#include <sstream>
#include <chrono>
#include <thread>

// subscriber callback function
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
  // initialize eCAL API
  eCAL::Initialize("binary_rec");

  // subscriber for topic "blob"
  eCAL::CSubscriber sub("blob");

  // assign callback
  sub.SetReceiveCallback(OnReceive);

  // idle main loop
  while (eCAL::Ok()) std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
