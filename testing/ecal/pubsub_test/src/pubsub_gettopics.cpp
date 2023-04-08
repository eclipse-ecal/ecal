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

#include <unordered_map>

#include <gtest/gtest.h>

#define CMN_REGISTRATION_REFRESH   1000
#define CMN_MONITORING_TIMEOUT     5000

TEST(IO, GetTopics)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_gettopics");

  std::unordered_map<std::string, eCAL::Util::STopicInfo> topic_info_map;

  // create and check a few pub/sub entities
  {
    std::string ttype("type");
    std::string tdesc("desc");

    // create 3 publisher
    eCAL::CPublisher pub1("A1", ttype + "A1", tdesc + "A1");
    eCAL::CPublisher pub2("A2", ttype + "A2", tdesc + "A2");
    eCAL::CPublisher pub3("A3", ttype + "A3", tdesc + "A3");

    // create a missmatching publisher
    // this should trigger a warning but not increase map size
    eCAL::CPublisher pub12("A1", ttype + "A1.2", tdesc + "A1.2");

    // create 2 subscriber
    eCAL::CSubscriber sub1("B1", ttype + "B1", tdesc + "B1");
    eCAL::CSubscriber sub2("B2", ttype + "B2", tdesc + "B2");

    // create a missmatching subscriber
    // this should trigger a warning but not increase map size
    eCAL::CSubscriber sub12("B1", ttype + "B1.2", tdesc + "B1.2");

    // get all topics
    eCAL::Util::GetTopics(topic_info_map);

    // check size
    EXPECT_EQ(topic_info_map.size(), 5);

    // check types and descriptions
    for (auto& topic_info : topic_info_map)
    {
      EXPECT_EQ(eCAL::Util::GetTopicTypeName(topic_info.first), ttype + topic_info.first);
      EXPECT_EQ(eCAL::Util::GetTopicDescription(topic_info.first), tdesc + topic_info.first);
    }

    // wait a monitoring timeout long,
    eCAL::Process::SleepMS(CMN_MONITORING_TIMEOUT);

    // the topics should not be expired
    eCAL::Util::GetTopics(topic_info_map);

    // check size
    EXPECT_EQ(topic_info_map.size(), 5);

    // now destroy publisher pub1 and subscriber sub1
    // after map expiration time the entities p12 and sub12 
    // should replace them (by overwriting type name and description)
    pub1.Destroy();
    sub1.Destroy();

    // pub1 and sub1 still exists
    EXPECT_EQ(eCAL::Util::GetTopicTypeName("A1"), "typeA1");
    EXPECT_EQ(eCAL::Util::GetTopicDescription("A1"), "descA1");
    EXPECT_EQ(eCAL::Util::GetTopicTypeName("B1"), "typeB1");
    EXPECT_EQ(eCAL::Util::GetTopicDescription("B1"), "descB1");

    // wait a monitoring timeout long, and let pub1.2 and sub1.2 register
    eCAL::Process::SleepMS(CMN_MONITORING_TIMEOUT + CMN_REGISTRATION_REFRESH);

    // update map
    eCAL::Util::GetTopics(topic_info_map);

    // size should be 5 again (because of pub1.2 and sub1.2 should have replaced pub1 and sub1 attributes now)
    EXPECT_EQ(topic_info_map.size(), 5);

    // check overwritten attributes
    EXPECT_EQ(eCAL::Util::GetTopicTypeName("A1"), "typeA1.2");
    EXPECT_EQ(eCAL::Util::GetTopicDescription("A1"), "descA1.2");
    EXPECT_EQ(eCAL::Util::GetTopicTypeName("B1"), "typeB1.2");
    EXPECT_EQ(eCAL::Util::GetTopicDescription("B1"), "descB1.2");
  }

  // let's unregister them
  eCAL::Process::SleepMS(CMN_MONITORING_TIMEOUT + 1000);

  // get all topics again, now all topics 
  // should be removed from the map
  eCAL::Util::GetTopics(topic_info_map);

  // check size
  EXPECT_EQ(topic_info_map.size(), 0);

  // finalize eCAL API
  eCAL::Finalize();
}
