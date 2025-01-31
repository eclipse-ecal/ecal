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
#include <chrono>
#include <sstream>
#include <thread>

// a simple struct to demonstrate
// zero copy modifications
struct alignas(4) SSimpleStruct
{
  uint32_t version      = 1;
  uint16_t rows         = 5;
  uint16_t cols         = 3;
  uint32_t clock        = 0;
  uint8_t  bytes[5 * 3] = { 0 };
};

// SSimpleStruct logging
std::ostream& operator<<(std::ostream& os, const SSimpleStruct& s)
{
  os << "Version : " << s.version << std::endl;
  os << "Rows    : " << s.rows    << std::endl;
  os << "Cols    : " << s.cols    << std::endl;
  os << "Clock   : " << s.clock   << std::endl;

  os << "Bytes   : " << std::endl;
  for (int i = 0; i < s.rows; ++i) {
    for (int j = 0; j < s.cols; ++j) {
      os << static_cast<int>(s.bytes[i * s.cols + j]) << " ";
    }
    os << std::endl;
  }
  return os;
}

// subscriber callback function
void OnReceive(const eCAL::STopicId& /*topic_id_*/, const eCAL::SDataTypeInformation& /*data_type_info_*/, const eCAL::SReceiveCallbackData& data_)
{
  if (data_.buffer_size < 1) return;

  std::cout << "------------------------------------"   << std::endl;
  std::cout << "Binary buffer header :"                 << std::endl;
  std::cout << "------------------------------------"   << std::endl;
  std::cout << " Size    : " << data_.buffer_size       << std::endl;
  std::cout << " Time    : " << data_.send_timestamp    << std::endl;
  std::cout << " Clock   : " << data_.send_clock        << std::endl;
  std::cout                                             << std::endl;
  std::cout << "------------------------------------"   << std::endl;
  std::cout << "SSimpleStruct :"                        << std::endl;
  std::cout << "------------------------------------"   << std::endl;
  std::cout << *static_cast<const SSimpleStruct*>(data_.buffer) << std::endl;
}

int main()
{
  const char* nodeName  = "binary_zero_copy_rec";
  const char* topicName = "simple_struct";

  // initialize eCAL API
  eCAL::Initialize(nodeName);

  // create the subscriber
  eCAL::CSubscriber sub(topicName);

  // assign callback
  sub.SetReceiveCallback(OnReceive);

  // idle main loop
  while (eCAL::Ok()) std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // finalize eCAL API
  eCAL::Finalize();

  return 0;
}
