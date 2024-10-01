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
#include <ecal/msg/string/publisher.h>
#include <ecal/msg/string/subscriber.h>
#include <ecal/ecal_defs.h>
#include <ecal/ecal_process.h>

#include <atomic>
#include <thread>
#include <fstream>
#include <ostream>

#include <gtest/gtest.h>

const std::string ECAL_DEFAULT_SUB_TOPIC_MAPPING = "sub_topic_mapping.yaml";
const std::string ECAL_DEFAULT_PUB_TOPIC_MAPPING = "pub_topic_mapping.yaml";

const std::string TOPIC_ORIGINAL = "original";
const std::string TOPIC_RENAMED  = "renamed";

const unsigned int REGISTRATION_REFRESH = 50;

bool writeYamlFile(const std::string& file_name_)
{
  std::ofstream file(file_name_);

  if (file.is_open())
  {
    file << TOPIC_ORIGINAL;
    file << ": ";
    file << TOPIC_RENAMED;
    file.close();
    return true;
  }
  else 
  {
    std::cerr << "Error opening file for ini writing" << "\n";
    return false;
  }
}


TEST(core_cpp_pubsub_topic_renaming, Subscriber)
{
  bool file_written = writeYamlFile(ECAL_DEFAULT_SUB_TOPIC_MAPPING);

  eCAL::Configuration config;
  config.registration.registration_refresh = REGISTRATION_REFRESH;
  config.subscriber.topic_renaming         = true;

  eCAL::Initialize(config, "topic_renaming_subscriber", eCAL::Init::Default);

  eCAL::string::CSubscriber<std::string> subscriber;
  subscriber.Create(TOPIC_ORIGINAL);

  EXPECT_EQ(subscriber.GetTopicName(), TOPIC_RENAMED);

  eCAL::string::CPublisher<std::string> publisher_original;
  publisher_original.Create(TOPIC_ORIGINAL);

  eCAL::string::CPublisher<std::string> publisher_renamed;
  publisher_renamed.Create(TOPIC_RENAMED);

  eCAL::Process::SleepMS(REGISTRATION_REFRESH * 2);

  EXPECT_EQ(publisher_original.IsSubscribed(), false);
  EXPECT_EQ(publisher_renamed.IsSubscribed(), true);

  eCAL::Finalize();

  if (file_written)
  {
    remove(ECAL_DEFAULT_SUB_TOPIC_MAPPING.data());
  }
}

TEST(core_cpp_pubsub_topic_renaming, Publisher)
{
  bool file_written = writeYamlFile(ECAL_DEFAULT_PUB_TOPIC_MAPPING);

  eCAL::Configuration config;
  config.registration.registration_refresh = REGISTRATION_REFRESH;
  config.publisher.topic_renaming          = true;

  eCAL::Initialize(config, "topic_renaming_publisher", eCAL::Init::Default);

  eCAL::string::CPublisher<std::string> publisher;
  publisher.Create(TOPIC_ORIGINAL);

  EXPECT_EQ(publisher.GetTopicName(), TOPIC_RENAMED);

  eCAL::string::CSubscriber<std::string> subscriber_original;
  subscriber_original.Create(TOPIC_ORIGINAL);

  eCAL::string::CSubscriber<std::string> subscriber_renamed;
  subscriber_renamed.Create(TOPIC_RENAMED);

  eCAL::Process::SleepMS(REGISTRATION_REFRESH * 2);

  EXPECT_EQ(subscriber_original.IsPublished(), false);
  EXPECT_EQ(subscriber_renamed.IsPublished(),  true);

  eCAL::Finalize();

  if (file_written)
  {
    remove(ECAL_DEFAULT_PUB_TOPIC_MAPPING.data());
  }
}

TEST(core_cpp_pubsub_topic_renaming, IndividualPublisher)
{
  bool file_written = writeYamlFile(ECAL_DEFAULT_PUB_TOPIC_MAPPING);

  eCAL::Configuration config;
  config.registration.registration_refresh = REGISTRATION_REFRESH;
  config.publisher.topic_renaming          = false;

  eCAL::Initialize(config, "topic_renaming_publisher", eCAL::Init::Default);

  auto pub_config = eCAL::GetPublisherConfiguration();
  pub_config.topic_renaming = true;

  eCAL::string::CPublisher<std::string> publisher_no_renaming;
  publisher_no_renaming.Create(TOPIC_ORIGINAL);

  eCAL::string::CPublisher<std::string> publisher_with_renaming;
  publisher_with_renaming.Create(TOPIC_ORIGINAL, pub_config);

  EXPECT_EQ(publisher_no_renaming.GetTopicName(), TOPIC_ORIGINAL);
  EXPECT_EQ(publisher_with_renaming.GetTopicName(), TOPIC_RENAMED);

  eCAL::string::CSubscriber<std::string> subscriber_original;
  subscriber_original.Create(TOPIC_ORIGINAL);

  eCAL::string::CSubscriber<std::string> subscriber_renamed;
  subscriber_renamed.Create(TOPIC_RENAMED);

  eCAL::Process::SleepMS(REGISTRATION_REFRESH * 2);

  EXPECT_EQ(subscriber_original.GetPublisherCount(), 1);
  EXPECT_EQ(subscriber_renamed.GetPublisherCount(),  1);

  eCAL::Finalize();

  if (file_written)
  {
    remove(ECAL_DEFAULT_PUB_TOPIC_MAPPING.data());
  }
}