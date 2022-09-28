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

#include <iostream>

int main(int argc, char **argv)
{
  std::cout << "-------------------------------" << std::endl;
  std::cout << " BINARY RECEIVER               " << std::endl;
  std::cout << "-------------------------------" << std::endl;

  // initialize eCAL API
  eCAL::Initialize(argc, argv, "binary_rec");

  // subscriber for topic "blob"
  eCAL::CSubscriber sub("blob");

  // receive updates
  std::string bin_buffer;
  while (eCAL::Ok())
  {
    // receive buffer
    if (sub.ReceiveBuffer(bin_buffer) && bin_buffer.size() > 0)
    {
      std::cout << "Received buffer filled with " << static_cast<int>(bin_buffer[0]) << std::endl;
    }
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
