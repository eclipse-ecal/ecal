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
#include <vector>

const int subscriber_number                    (5000);

const int publisher_number                     (5000);
const int publisher_type_encoding_size_bytes   (10*1024);
const int publisher_type_descriptor_size_bytes (10*1024);

const int in_between_sleep_sec                 (5);
const int final_sleep_sec                      (5);

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

int main()
{
  // set eCAL configuration
  eCAL::Configuration configuration;
  configuration.registration.registration_timeout = 10000;    // registration timeout == 10 sec
  configuration.registration.layer.shm.enable     = true;     // switch shm registration on and
  configuration.registration.layer.udp.enable     = false;    // switch udp registration off

  // initialize eCAL API
  eCAL::Initialize(configuration, "massive_pub_sub");

  // publisher registration event callback
  size_t created_publisher_num(0);
  size_t deleted_publisher_num(0);
  std::set<eCAL::STopicId> created_publisher_ids;
  std::set<eCAL::STopicId> deleted_publisher_ids;
  eCAL::Registration::AddPublisherEventCallback(
    [&](const eCAL::STopicId& id_, eCAL::Registration::RegistrationEventType event_type_)
    {
      switch (event_type_)
      {
      case eCAL::Registration::RegistrationEventType::new_entity:
        created_publisher_num++;
        created_publisher_ids.insert(id_);
        //std::cout << "Publisher created" << std::endl;
        break;
      case eCAL::Registration::RegistrationEventType::deleted_entity:
        deleted_publisher_num++;
        deleted_publisher_ids.insert(id_);
        //std::cout << "Publisher deleted" << std::endl;
        break;
      }
    }
  );

  // create subscriber
  std::vector<eCAL::CSubscriber> vector_of_subscriber;
  std::cout << "Subscriber creation started. (" << subscriber_number << ")" << std::endl;
  {
    // start time measurement
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < subscriber_number; i++)
    {
      // publisher topic name
      std::stringstream topic_name;
      topic_name << "TOPIC_" << i;

      // create subscriber
      vector_of_subscriber.emplace_back(topic_name.str());
    }
    // stop time measurement
    auto end_time = std::chrono::high_resolution_clock::now();

    // calculate the duration
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Time taken for subscriber creation: " << duration << " milliseconds" << std::endl << std::endl;
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
      std::stringstream topic_name;
      topic_name << "TOPIC_" << i;

      // create publisher
      vector_of_publisher.emplace_back(topic_name.str(), data_type_info);
    }
    // stop time measurement
    auto end_time = std::chrono::high_resolution_clock::now();

    // calculate the duration
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Time taken for publisher creation: " << duration << " milliseconds" << std::endl << std::endl;
  }

  // sleep for a few seconds
  std::this_thread::sleep_for(std::chrono::seconds(in_between_sleep_sec));

  // wait for full registration
  std::cout << "Wait for publisher/subscriber registration." << std::endl;
  {
    // start time measurement
    auto start_time = std::chrono::high_resolution_clock::now();

    size_t num_pub(0);
    size_t num_sub(0);
    while ((num_pub < publisher_number) || (num_sub < subscriber_number))
    {
      std::set<eCAL::STopicId> publisher_ids;
      std::set<eCAL::STopicId> subscriber_ids;

      eCAL::Registration::GetPublisherIDs(publisher_ids);
      eCAL::Registration::GetSubscriberIDs(subscriber_ids);

      num_pub = publisher_ids.size();
      num_sub = subscriber_ids.size();

      std::cout << "Registered publisher : " << num_pub << std::endl;
      std::cout << "Registered subscriber: " << num_sub << std::endl;

      // sleep for 1000 milliseconds
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    // stop time measurement
    auto end_time = std::chrono::high_resolution_clock::now();

    // calculate the duration
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Time taken to get all registered: " << duration << " milliseconds" << std::endl << std::endl;
  }

  // get publisher information
  std::cout << "Get publisher information. (";
  size_t num_pub(0);
  {
    // start time measurement
    auto start_time = std::chrono::high_resolution_clock::now();

    std::set<eCAL::STopicId> pub_ids;
    eCAL::Registration::GetPublisherIDs(pub_ids);
    num_pub = pub_ids.size();
    for (const auto& id : pub_ids)
    {
      eCAL::SDataTypeInformation topic_info;
      eCAL::Registration::GetPublisherInfo(id, topic_info);
    }

    // stop time measurement
    auto end_time = std::chrono::high_resolution_clock::now();

    // calculate the duration
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << num_pub << ")" << std::endl << "Time taken to get publisher information: " << duration << " milliseconds" << std::endl << std::endl;
  }

  // check creation events
  std::set<eCAL::STopicId> publisher_ids;
  eCAL::Registration::GetPublisherIDs(publisher_ids);
  std::cout << "Number of publisher creation events   " << created_publisher_num << std::endl;
  std::cout << "Size   of publisher creation id set   " << created_publisher_ids.size() << std::endl;
  //std::cout << "Publisher creation id sets are equal  " << (publisher_ids == created_publisher_ids) << std::endl;
  std::cout << std::endl;
  
  // delete all publisher
  std::cout << "Delete all publisher .." << std::endl;
  vector_of_publisher.clear();
  std::cout << "Deletion done." << std::endl;
  std::cout << std::endl;

  // sleep for a few seconds
  std::this_thread::sleep_for(std::chrono::seconds(in_between_sleep_sec));

  // check deletion events
  std::cout << "Number of publisher deletion events   " << deleted_publisher_num << std::endl;
  std::cout << "Size   of publisher deletion id set   " << deleted_publisher_ids.size() << std::endl;
  //std::cout << "Publisher deleteion id sets are equal " << (publisher_ids == deleted_publisher_ids) << std::endl;

  // sleep final seconds
  std::this_thread::sleep_for(std::chrono::seconds(final_sleep_sec));

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
