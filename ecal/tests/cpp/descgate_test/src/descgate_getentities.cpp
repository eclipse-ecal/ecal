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

#include "ecal_descgate.h"

#include <cstdint>
#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <thread>

#define DESCGATE_EXPIRATION_MS 500

namespace
{
  eCAL::Registration::Sample CreatePublisher(const std::string& topic_name_, std::uint64_t topic_id_)
  {
    eCAL::Registration::Sample reg_sample;
    reg_sample.cmd_type                              = eCAL::bct_reg_publisher;
    reg_sample.identifier.entity_id                  = topic_id_;
    reg_sample.topic.topic_name                      = topic_name_;
    reg_sample.topic.datatype_information.name       = topic_name_ + "-datatype_information.name";
    reg_sample.topic.datatype_information.encoding   = topic_name_ + "-datatype_information.encoding";
    reg_sample.topic.datatype_information.descriptor = topic_name_ + "-datatype_information.descriptor";
    return reg_sample;
  }

  eCAL::Registration::Sample DestroyPublisher(const std::string& topic_name_, std::uint64_t topic_id_)
  {
    eCAL::Registration::Sample reg_sample = CreatePublisher(topic_name_, topic_id_);
    reg_sample.cmd_type = eCAL::bct_unreg_publisher;
    return reg_sample;
  }

  eCAL::Registration::Sample CreateSubscriber(const std::string& topic_name_, std::uint64_t topic_id_)
  {
    eCAL::Registration::Sample reg_sample;
    reg_sample.cmd_type                              = eCAL::bct_reg_subscriber;
    reg_sample.identifier.entity_id                  = topic_id_;
    reg_sample.topic.topic_name                      = topic_name_;
    reg_sample.topic.datatype_information.name       = topic_name_ + "-datatype_information.name";
    reg_sample.topic.datatype_information.encoding   = topic_name_ + "-datatype_information.encoding";
    reg_sample.topic.datatype_information.descriptor = topic_name_ + "-datatype_information.descriptor";
    return reg_sample;
  }

  eCAL::Registration::Sample DestroySubscriber(const std::string& topic_name_, std::uint64_t topic_id_)
  {
    eCAL::Registration::Sample reg_sample = CreateSubscriber(topic_name_, topic_id_);
    reg_sample.cmd_type = eCAL::bct_unreg_subscriber;
    return reg_sample;
  }

  eCAL::Registration::Sample CreateService(const std::string& service_name_, std::uint64_t service_id_)
  {
    eCAL::Registration::Sample reg_sample;
    reg_sample.cmd_type             = eCAL::bct_reg_service;
    reg_sample.service.service_name = service_name_;
    reg_sample.identifier.entity_id = service_id_;

    eCAL::Service::Method method;
    method.method_name = "method_name";
    reg_sample.service.methods.push_back(method);
    return reg_sample;
  }

  eCAL::Registration::Sample DestroyService(const std::string& service_name_, std::uint64_t service_id_)
  {
    eCAL::Registration::Sample reg_sample = CreateService(service_name_, service_id_);
    reg_sample.cmd_type = eCAL::bct_unreg_service;
    return reg_sample;
  }

  eCAL::Registration::Sample CreateClient(const std::string& client_name_, std::uint64_t service_id_)
  {
    eCAL::Registration::Sample reg_sample;
    reg_sample.cmd_type               = eCAL::bct_reg_client;
    reg_sample.client.service_name    = client_name_;
    reg_sample.identifier.entity_id   = service_id_;

    eCAL::Service::Method method;
    method.method_name = "method_name";
    reg_sample.client.methods.push_back(method);
    return reg_sample;
  }

  eCAL::Registration::Sample DestroyClient(const std::string& client_name_, std::uint64_t service_id_)
  {
    eCAL::Registration::Sample reg_sample = CreateClient(client_name_, service_id_);
    reg_sample.cmd_type = eCAL::bct_unreg_client;
    return reg_sample;
  }
}

TEST(core_cpp_descgate, PublisherExpiration)
{
  eCAL::CDescGate desc_gate;

  // apply sample 5 times, sample should not expire
  auto runs(5);
  while ((runs--) != 0)
  {
    desc_gate.ApplySample(CreatePublisher("pub1", 1), eCAL::tl_none);
    EXPECT_EQ(1, desc_gate.GetPublisherIDs().size());
  }

  // now let the sample expire
  desc_gate.ApplySample(DestroyPublisher("pub1", 1), eCAL::tl_none);

  // sample should be expired
  EXPECT_EQ(0, desc_gate.GetPublisherIDs().size());
}

TEST(core_cpp_descgate, PublisherQualities)
{
  eCAL::CDescGate desc_gate;

  // create and apply publisher pub1
  desc_gate.ApplySample(CreatePublisher("pub1", 1), eCAL::tl_none);

  // create and apply publisher pub2
  desc_gate.ApplySample(CreatePublisher("pub2", 2), eCAL::tl_none);

  // check size
  auto id_set = desc_gate.GetPublisherIDs();
  EXPECT_EQ(2, id_set.size());

  // check publisher qualities
  {
    for (const auto& id : id_set)
    {
      eCAL::SDataTypeInformation topic_info;
      bool found = desc_gate.GetPublisherInfo(id, topic_info);
      EXPECT_TRUE(found);
      if (found)
      {
        std::string topic_name = id.topic_name;
        EXPECT_EQ(topic_name + "-datatype_information.name",       topic_info.name);
        EXPECT_EQ(topic_name + "-datatype_information.encoding",   topic_info.encoding);
        EXPECT_EQ(topic_name + "-datatype_information.descriptor", topic_info.descriptor);
      }
    }
  }

  // now let the sample expire
  desc_gate.ApplySample(DestroyPublisher("pub1", 1), eCAL::tl_none);
  desc_gate.ApplySample(DestroyPublisher("pub2", 2), eCAL::tl_none);

  // sample should be expired
  EXPECT_EQ(0, desc_gate.GetPublisherIDs().size());
}

TEST(core_cpp_descgate, ManyPublisher)
{
  eCAL::CDescGate desc_gate;

  constexpr int num_pub(1000);
  for (auto pub = 0; pub < num_pub; ++pub)
  {
    // create registration sample for pub-xx
    desc_gate.ApplySample(CreatePublisher("pub" + std::to_string(pub), pub), eCAL::tl_none);
  }

  // map should contain num_pub samples
  EXPECT_EQ(num_pub, desc_gate.GetPublisherIDs().size());

  // now let the samples expire
  for (auto pub = 0; pub < num_pub; ++pub)
  {
    // create registration sample for pub-xx
    desc_gate.ApplySample(DestroyPublisher("pub" + std::to_string(pub), pub), eCAL::tl_none);
  }

  // samples should be expired
  EXPECT_EQ(0, desc_gate.GetPublisherIDs().size());
}

TEST(core_cpp_descgate, SubscriberExpiration)
{
  eCAL::CDescGate desc_gate;

  // apply sample 5 times, sample should not expire
  auto runs(5);
  while ((runs--) != 0)
  {
    desc_gate.ApplySample(CreateSubscriber("sub1", 1), eCAL::tl_none);
    EXPECT_EQ(1, desc_gate.GetSubscriberIDs().size());
  }

  // now let the sample expire
  desc_gate.ApplySample(DestroySubscriber("sub1", 1), eCAL::tl_none);

  // sample should be expired
  EXPECT_EQ(0, desc_gate.GetSubscriberIDs().size());
}

TEST(core_cpp_descgate, SubscriberQualities)
{
  eCAL::CDescGate desc_gate;

  // create and apply subscriber sub1
  desc_gate.ApplySample(CreateSubscriber("sub1", 1), eCAL::tl_none);

  // create and apply subscriber sub2
  desc_gate.ApplySample(CreateSubscriber("sub2", 2), eCAL::tl_none);

  // check size
  auto id_set = desc_gate.GetSubscriberIDs();
  EXPECT_EQ(2, id_set.size());

  // check subscriber qualities
  {
    for (const auto& id : id_set)
    {
      eCAL::SDataTypeInformation topic_info;
      bool found = desc_gate.GetSubscriberInfo(id, topic_info);
      EXPECT_TRUE(found);
      if (found)
      {
        std::string topic_name = id.topic_name;
        EXPECT_EQ(topic_name + "-datatype_information.name",       topic_info.name);
        EXPECT_EQ(topic_name + "-datatype_information.encoding",   topic_info.encoding);
        EXPECT_EQ(topic_name + "-datatype_information.descriptor", topic_info.descriptor);
      }
    }
  }

  // now let the sample expire
  desc_gate.ApplySample(DestroySubscriber("sub1", 1), eCAL::tl_none);
  desc_gate.ApplySample(DestroySubscriber("sub2", 2), eCAL::tl_none);

  // sample should be expired
  EXPECT_EQ(0, desc_gate.GetSubscriberIDs().size());
}

TEST(core_cpp_descgate, ManySubscriber)
{
  eCAL::CDescGate desc_gate;

  constexpr int num_sub(1000);
  for (auto sub = 0; sub < num_sub; ++sub)
  {
    // create registration sample for sub-xx
    desc_gate.ApplySample(CreateSubscriber("sub" + std::to_string(sub), sub), eCAL::tl_none);
  }

  // map should contain num_sub samples
  EXPECT_EQ(num_sub, desc_gate.GetSubscriberIDs().size());

  // now let the samples expire
  for (auto sub = 0; sub < num_sub; ++sub)
  {
    // create registration sample for sub-xx
    desc_gate.ApplySample(DestroySubscriber("sub" + std::to_string(sub), sub), eCAL::tl_none);
  }

  // samples should be expired
  EXPECT_EQ(0, desc_gate.GetSubscriberIDs().size());
}

TEST(core_cpp_descgate, ServiceExpiration)
{
  eCAL::CDescGate desc_gate;

  // apply sample 5 times, sample should not expire
  auto runs(5);
  while ((runs--) != 0)
  {
    desc_gate.ApplySample(CreateService("service1", 1), eCAL::tl_none);

    EXPECT_EQ(1, desc_gate.GetServerIDs().size());
  }

  // now let the sample expire
  desc_gate.ApplySample(DestroyService("service1", 1), eCAL::tl_none);

  // sample should be expired
  EXPECT_EQ(0, desc_gate.GetServerIDs().size());
}

TEST(core_cpp_descgate, ManyService)
{
  eCAL::CDescGate desc_gate;

  constexpr int num_service(1000);
  for (auto service = 0; service < num_service; ++service)
  {
    // create registration sample for service-xx
    desc_gate.ApplySample(CreateService("service" + std::to_string(service), service), eCAL::tl_none);
  }

  // map should contain num_service samples
  EXPECT_EQ(num_service, desc_gate.GetServerIDs().size());

  // now let the samples expire
  for (auto service = 0; service < num_service; ++service)
  {
    // create registration sample for service-xx
    desc_gate.ApplySample(DestroyService("service" + std::to_string(service), service), eCAL::tl_none);
  }

  // samples should be expired
  EXPECT_EQ(0, desc_gate.GetServerIDs().size());
}

TEST(core_cpp_descgate, ClientExpiration)
{
  eCAL::CDescGate desc_gate;

  // apply sample 5 times, sample should not expire
  auto runs(5);
  while ((runs--) != 0)
  {
    desc_gate.ApplySample(CreateClient("client1", 1), eCAL::tl_none);
    EXPECT_EQ(1, desc_gate.GetClientIDs().size());
  }

  // now let the sample expire
  desc_gate.ApplySample(DestroyClient("client1", 1), eCAL::tl_none);

  // sample should be expired
  EXPECT_EQ(0, desc_gate.GetClientIDs().size());
}

TEST(core_cpp_descgate, ManyClient)
{
  eCAL::CDescGate desc_gate;

  constexpr int num_client(1000);
  for (auto client = 0; client < num_client; ++client)
  {
    // create registration sample for client-xx
    desc_gate.ApplySample(CreateClient("client" + std::to_string(client), client), eCAL::tl_none);
  }

  // map should contain num_client samples
  EXPECT_EQ(num_client, desc_gate.GetClientIDs().size());

  // now let the samples expire
  for (auto client = 0; client < num_client; ++client)
  {
    // create registration sample for client-xx
    desc_gate.ApplySample(DestroyClient("client" + std::to_string(client), client), eCAL::tl_none);
  }

  // samples should be expired
  EXPECT_EQ(0, desc_gate.GetClientIDs().size());
}
