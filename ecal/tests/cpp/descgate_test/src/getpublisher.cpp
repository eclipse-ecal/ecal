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

#include "ecal_descgate.h"

#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#define DESCGATE_EXPIRATION_MS 500

namespace
{
  eCAL::Registration::Sample CreatePublisher(const std::string& topic_name_, std::uint64_t topic_id_)
  {
    eCAL::Registration::Sample reg_sample;
    reg_sample.cmd_type                   = eCAL::bct_reg_publisher;
    reg_sample.topic.tname                = topic_name_;
    reg_sample.topic.tid                  = std::to_string(topic_id_);
    reg_sample.topic.tdatatype.name       = topic_name_ + "-tdatatype.name";
    reg_sample.topic.tdatatype.encoding   = topic_name_ + "-tdatatype.encoding";
    reg_sample.topic.tdatatype.descriptor = topic_name_ + "-tdatatype.descriptor";
    return reg_sample;
  }

  eCAL::Registration::Sample CreateSubscriber(const std::string& topic_name_, std::uint64_t topic_id_)
  {
    eCAL::Registration::Sample reg_sample;
    reg_sample.cmd_type                   = eCAL::bct_reg_subscriber;
    reg_sample.topic.tname                = topic_name_;
    reg_sample.topic.tid                  = std::to_string(topic_id_);
    reg_sample.topic.tdatatype.name       = topic_name_ + "-tdatatype.name";
    reg_sample.topic.tdatatype.encoding   = topic_name_ + "-tdatatype.encoding";
    reg_sample.topic.tdatatype.descriptor = topic_name_ + "-tdatatype.descriptor";
    return reg_sample;
  }

  eCAL::Registration::Sample CreateService(const std::string& service_name_, std::uint64_t service_id_)
  {
    eCAL::Registration::Sample reg_sample;
    reg_sample.cmd_type      = eCAL::bct_reg_service;
    reg_sample.service.sname = service_name_;
    reg_sample.service.sid   = std::to_string(service_id_);

    eCAL::Service::Method method;
    method.mname = "method_name";
    reg_sample.service.methods.push_back(method);
    return reg_sample;
  }

  eCAL::Registration::Sample CreateClient(const std::string& client_name_, std::uint64_t service_id_)
  {
    eCAL::Registration::Sample reg_sample;
    reg_sample.cmd_type     = eCAL::bct_reg_client;
    reg_sample.client.sname = client_name_;
    reg_sample.client.sid   = std::to_string(service_id_);

    eCAL::Service::Method method;
    method.mname = "method_name";
    reg_sample.client.methods.push_back(method);
    return reg_sample;
  }
}

TEST(core_cpp_descgate, PublisherExpiration)
{
  eCAL::CDescGate desc_gate(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS));

  // apply sample 5 times, sample should not expire
  auto runs(5);
  while (runs--)
  {
    desc_gate.ApplySample(CreatePublisher("pub1", 1), eCAL::tl_none);
    std::this_thread::sleep_for(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS / 2));

    EXPECT_EQ(1, desc_gate.GetPublisher().size());
  }

  // now let the sample expire
  std::this_thread::sleep_for(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS));

  // sample should be expired
  EXPECT_EQ(0, desc_gate.GetPublisher().size());
}

TEST(core_cpp_descgate, PublisherQualities)
{
  eCAL::CDescGate desc_gate(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS));

  // create and apply publisher pub1
  desc_gate.ApplySample(CreatePublisher("pub1", 1), eCAL::tl_none);

  // create and apply publisher pub2
  desc_gate.ApplySample(CreatePublisher("pub2", 2), eCAL::tl_none);

  // check gate size
  auto sample_map = desc_gate.GetPublisher();
  EXPECT_EQ(2, sample_map.size());

  // check pub1 quality
  {
    auto pub_it = sample_map.find({ "pub1", 1 });
    EXPECT_NE(pub_it, sample_map.end());
    if (pub_it != sample_map.end())
    {
      EXPECT_EQ(1,                           pub_it->second.id);
      EXPECT_EQ("pub1-tdatatype.name",       pub_it->second.info.name);
      EXPECT_EQ("pub1-tdatatype.encoding",   pub_it->second.info.encoding);
      EXPECT_EQ("pub1-tdatatype.descriptor", pub_it->second.info.descriptor);
    }
  }

  // check pub2 quality
  {
    auto pub_it = sample_map.find({ "pub2", 2 });
    EXPECT_NE(pub_it, sample_map.end());
    if (pub_it != sample_map.end())
    {
      EXPECT_EQ(2,                           pub_it->second.id);
      EXPECT_EQ("pub2-tdatatype.name",       pub_it->second.info.name);
      EXPECT_EQ("pub2-tdatatype.encoding",   pub_it->second.info.encoding);
      EXPECT_EQ("pub2-tdatatype.descriptor", pub_it->second.info.descriptor);
    }
  }

  // now let the sample expire
  std::this_thread::sleep_for(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS));

  // sample should be expired
  EXPECT_EQ(0, desc_gate.GetPublisher().size());
}

TEST(core_cpp_descgate, ManyPublisher)
{
  eCAL::CDescGate desc_gate(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS));

  constexpr int num_pub(1000);
  for (auto pub = 0; pub < num_pub; ++pub)
  {
    // create registration sample for pub-xx
    desc_gate.ApplySample(CreatePublisher("pub" + std::to_string(pub), pub), eCAL::tl_none);
  }

  // map should contain num_pub samples
  EXPECT_EQ(num_pub, desc_gate.GetPublisher().size());

  // now let the samples expire
  std::this_thread::sleep_for(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS));

  // samples should be expired
  EXPECT_EQ(0, desc_gate.GetPublisher().size());
}

TEST(core_cpp_descgate, SubscriberExpiration)
{
  eCAL::CDescGate desc_gate(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS));

  // apply sample 5 times, sample should not expire
  auto runs(5);
  while (runs--)
  {
    desc_gate.ApplySample(CreateSubscriber("sub1", 1), eCAL::tl_none);
    std::this_thread::sleep_for(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS / 2));

    EXPECT_EQ(1, desc_gate.GetSubscriber().size());
  }

  // now let the sample expire
  std::this_thread::sleep_for(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS));

  // sample should be expired
  EXPECT_EQ(0, desc_gate.GetSubscriber().size());
}

TEST(core_cpp_descgate, SubscriberQualities)
{
  eCAL::CDescGate desc_gate(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS));

  // create and apply subscriber sub1
  desc_gate.ApplySample(CreateSubscriber("sub1", 1), eCAL::tl_none);

  // create and apply subscriber sub2
  desc_gate.ApplySample(CreateSubscriber("sub2", 2), eCAL::tl_none);

  // check gate size
  auto sample_map = desc_gate.GetSubscriber();
  EXPECT_EQ(2, sample_map.size());

  // check sub1 quality
  {
    auto sub_it = sample_map.find({ "sub1", 1 });
    EXPECT_NE(sub_it, sample_map.end());
    if (sub_it != sample_map.end())
    {
      EXPECT_EQ(1,                           sub_it->second.id);
      EXPECT_EQ("sub1-tdatatype.name",       sub_it->second.info.name);
      EXPECT_EQ("sub1-tdatatype.encoding",   sub_it->second.info.encoding);
      EXPECT_EQ("sub1-tdatatype.descriptor", sub_it->second.info.descriptor);
    }
  }

  // check sub2 quality
  {
    auto sub_it = sample_map.find({ "sub2", 2 });
    EXPECT_NE(sub_it, sample_map.end());
    if (sub_it != sample_map.end())
    {
      EXPECT_EQ(2,                           sub_it->second.id);
      EXPECT_EQ("sub2-tdatatype.name",       sub_it->second.info.name);
      EXPECT_EQ("sub2-tdatatype.encoding",   sub_it->second.info.encoding);
      EXPECT_EQ("sub2-tdatatype.descriptor", sub_it->second.info.descriptor);
    }
  }

  // now let the sample expire
  std::this_thread::sleep_for(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS));

  // sample should be expired
  EXPECT_EQ(0, desc_gate.GetSubscriber().size());
}

TEST(core_cpp_descgate, ManySubscriber)
{
  eCAL::CDescGate desc_gate(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS));

  constexpr int num_sub(1000);
  for (auto sub = 0; sub < num_sub; ++sub)
  {
    // create registration sample for sub-xx
    desc_gate.ApplySample(CreateSubscriber("sub" + std::to_string(sub), sub), eCAL::tl_none);
  }

  // map should contain num_sub samples
  EXPECT_EQ(num_sub, desc_gate.GetSubscriber().size());

  // now let the samples expire
  std::this_thread::sleep_for(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS));

  // samples should be expired
  EXPECT_EQ(0, desc_gate.GetSubscriber().size());
}

TEST(core_cpp_descgate, ServiceExpiration)
{
  eCAL::CDescGate desc_gate(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS));

  // apply sample 5 times, sample should not expire
  auto runs(5);
  while (runs--)
  {
    desc_gate.ApplySample(CreateService("service1", 1), eCAL::tl_none);
    std::this_thread::sleep_for(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS / 2));

    EXPECT_EQ(1, desc_gate.GetServices().size());
  }

  // now let the sample expire
  std::this_thread::sleep_for(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS));

  // sample should be expired
  EXPECT_EQ(0, desc_gate.GetServices().size());
}

TEST(core_cpp_descgate, ManyService)
{
  eCAL::CDescGate desc_gate(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS));

  constexpr int num_service(1000);
  for (auto service = 0; service < num_service; ++service)
  {
    // create registration sample for service-xx
    desc_gate.ApplySample(CreateService("service" + std::to_string(service), service), eCAL::tl_none);
  }

  // map should contain num_service samples
  EXPECT_EQ(num_service, desc_gate.GetServices().size());

  // now let the samples expire
  std::this_thread::sleep_for(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS));

  // samples should be expired
  EXPECT_EQ(0, desc_gate.GetServices().size());
}

TEST(core_cpp_descgate, ClientExpiration)
{
  eCAL::CDescGate desc_gate(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS));

  // apply sample 5 times, sample should not expire
  auto runs(5);
  while (runs--)
  {
    desc_gate.ApplySample(CreateClient("client1", 1), eCAL::tl_none);
    std::this_thread::sleep_for(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS / 2));

    EXPECT_EQ(1, desc_gate.GetClients().size());
  }

  // now let the sample expire
  std::this_thread::sleep_for(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS));

  // sample should be expired
  EXPECT_EQ(0, desc_gate.GetClients().size());
}

TEST(core_cpp_descgate, ManyClient)
{
  eCAL::CDescGate desc_gate(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS));

  constexpr int num_client(1000);
  for (auto client = 0; client < num_client; ++client)
  {
    // create registration sample for client-xx
    desc_gate.ApplySample(CreateClient("client" + std::to_string(client), client), eCAL::tl_none);
  }

  // map should contain num_client samples
  EXPECT_EQ(num_client, desc_gate.GetClients().size());

  // now let the samples expire
  std::this_thread::sleep_for(std::chrono::milliseconds(DESCGATE_EXPIRATION_MS));

  // samples should be expired
  EXPECT_EQ(0, desc_gate.GetClients().size());
}
