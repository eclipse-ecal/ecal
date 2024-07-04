/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
#include <vector>

const int subscriber_number                    (5000);

const int publisher_number                     (5000);
const int publisher_type_encoding_size_bytes   (10*1024);
const int publisher_type_descriptor_size_bytes (10*1024);

const int in_between_sleep_sec                 (5);
const int final_sleep_sec                      (120);

std::string GenerateSizedString(const std::string& name, size_t totalSize)
{
  if (name.empty() || totalSize == 0) {
    return "";
  }

  std::string result;
  result.reserve(totalSize);

  while (result.size() + name.size() <= totalSize) {
    result += name;
  }

  if (result.size() < totalSize) {
    result += name.substr(0, totalSize - result.size());
  }

  return result;
}

int main(int argc, char** argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "massive_pub_sub");

  eCAL::Util::EnableLoopback(true);

  // create subscriber
  std::vector<eCAL::CSubscriber> vector_of_subscriber;
  std::cout << "Subscriber creation started. (" << subscriber_number << ")" << std::endl;
  {
    // start time measurement
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < subscriber_number; i++)
    {
      // publisher topic name
      std::stringstream tname;
      tname << "TOPIC_" << i;

      // create subscriber
      vector_of_subscriber.emplace_back(tname.str());
    }
    // stop time measurement
    auto end_time = std::chrono::high_resolution_clock::now();

    // calculate the duration
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Time taken for subscriber creation: " << duration << " milliseconds" << std::endl;
  }

  // sleep for a few seconds
  std::this_thread::sleep_for(std::chrono::seconds(in_between_sleep_sec));

  // create publisher
  std::vector<eCAL::CPublisher> vector_of_publisher;
  std::cout << "Publisher creation started. (" << publisher_number << ")" << std::endl;
  {
    // start time measurement
    auto start_time = std::chrono::high_resolution_clock::now();

    eCAL::SDataTypeInformation data_type_info;
    data_type_info.name       = "TOPIC_TYPE_NAME";
    data_type_info.encoding   = GenerateSizedString("TOPIC_TYPE_ENCODING",   publisher_type_encoding_size_bytes);
    data_type_info.descriptor = GenerateSizedString("TOPIC_TYPE_DESCRIPTOR", publisher_type_descriptor_size_bytes);

    for (int i = 0; i < publisher_number; i++)
    {
      // publisher topic name
      std::stringstream tname;
      tname << "TOPIC_" << i;

      // create publisher
      vector_of_publisher.emplace_back(tname.str(), data_type_info);
    }
    // stop time measurement
    auto end_time = std::chrono::high_resolution_clock::now();

    // calculate the duration
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Time taken for publisher creation: " << duration << " milliseconds" << std::endl;
  }
  std::cout << std::endl;

  // sleep for a few seconds
  std::this_thread::sleep_for(std::chrono::seconds(final_sleep_sec));

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
