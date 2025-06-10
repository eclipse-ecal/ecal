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

#include <ecal_c/ecal.h>

#include <gtest/gtest.h>


class pubsub_test_c : public ::testing::Test {

  protected:
    eCAL_Subscriber* subscriber;
    eCAL_Publisher* publisher;
    struct eCAL_SDataTypeInformation data_type_information;
    const char *topic_name = "C-Binding-Test";

    void SetUp() override {
      // initialize eCAL API
      eCAL_Initialize("pubsub_test_c", NULL, NULL);
    
      // create subscriber "hello"
      memset(&data_type_information, 0, sizeof(struct eCAL_SDataTypeInformation));
      data_type_information.name = "string";
      data_type_information.encoding = "utf-8";
      
      subscriber = eCAL_Subscriber_New(topic_name, &data_type_information, NULL, NULL);

      // create publisher "hello"
      publisher = eCAL_Publisher_New(topic_name, &data_type_information, NULL, NULL);
    }

    void TearDown() override {
      eCAL_Subscriber_Delete(subscriber);
      eCAL_Publisher_Delete(publisher);
      eCAL_Finalize();
    }
};

void OnReceive(const struct eCAL_STopicId* topic_id_, const struct eCAL_SDataTypeInformation* data_type_information_, const struct eCAL_SReceiveCallbackData* callback_data_, void* user_argument_)
{
  // unused arguments
  (void)data_type_information_;
  (void)user_argument_;
  int* cnt = (int*)user_argument_;
  (*cnt)++;
}

int DummyWriteFull(void* buffer, size_t size) 
{
  memset(buffer, 0x42, size);
  return 0;
}

size_t DummyGetSize()
{
  return 64;
}
  
TEST_F(pubsub_test_c, subscriber)
{
  // test subscriber instantiation
  EXPECT_NE(nullptr, subscriber);

  // test setting receivecallback
  eCAL_Subscriber_SetReceiveCallback(subscriber, OnReceive, NULL);

  // test GetTopicId
  EXPECT_STREQ(topic_name, eCAL_Subscriber_GetTopicId(subscriber)->topic_name);

  // test GetTopicName
  EXPECT_STREQ(topic_name, eCAL_Subscriber_GetTopicName(subscriber));

  // test GetDataTypeInformation
  EXPECT_STREQ("string", eCAL_Subscriber_GetDataTypeInformation(subscriber)->name);

  EXPECT_STREQ("utf-8", eCAL_Subscriber_GetDataTypeInformation(subscriber)->encoding);
}

TEST_F(pubsub_test_c, publisher)
{
  // test publisher instantiation
  EXPECT_NE(nullptr, publisher);
  
  // test GetTopicId
  EXPECT_STREQ(topic_name, eCAL_Publisher_GetTopicId(publisher)->topic_name);

  // test GetTopicName
  EXPECT_STREQ(topic_name, eCAL_Publisher_GetTopicName(publisher));

  // test GetDataTypeInformation
  EXPECT_STREQ("string", eCAL_Publisher_GetDataTypeInformation(publisher)->name);

  EXPECT_STREQ("utf-8", eCAL_Publisher_GetDataTypeInformation(publisher)->encoding);
}

TEST_F(pubsub_test_c, pubsub) 
{
  const char* snd_s = "HELLO WORLD FROM C";
  int cnt = 0;
  int callback_count = 0;

  // add callback
  eCAL_Subscriber_SetReceiveCallback(subscriber, OnReceive, &callback_count);

  // sleep here because otherwise it takes to long to set the receive-callback
  eCAL_Process_SleepMS(2000);

  // send messages
  for(int i = 0; i < 10; i++) 
  {
    // send content
    EXPECT_EQ(0, eCAL_Publisher_Send(publisher, snd_s, strlen(snd_s), NULL));
    cnt++;

    // sleep 500 ms
    eCAL_Process_SleepMS(10);
  }

  // checking amout of send messages and called callbacks
  EXPECT_EQ(cnt, callback_count);
}

TEST_F(pubsub_test_c, sub_RemoveReceiveCallback) 
{
  // add callback
  eCAL_Subscriber_SetReceiveCallback(subscriber, OnReceive, NULL);

  // actual test to remove it again
  eCAL_Subscriber_RemoveReceiveCallback(subscriber); 
}

TEST_F(pubsub_test_c, sub_GetPublisherCount) 
{
  eCAL_Publisher* publisher_1;
  eCAL_Publisher* publisher_2;
  eCAL_Publisher* publisher_3;

  publisher_1 = eCAL_Publisher_New(topic_name, &data_type_information, NULL, NULL);
  EXPECT_NE(nullptr, publisher_1);
  publisher_2 = eCAL_Publisher_New(topic_name, &data_type_information, NULL, NULL);
  EXPECT_NE(nullptr, publisher_2);
  publisher_3 = eCAL_Publisher_New(topic_name, &data_type_information, NULL, NULL);
  EXPECT_NE(nullptr, publisher_3);

  eCAL_Process_SleepMS(5000);

  // actual test 3 publishers + 1 from SetUp()
  EXPECT_EQ(4, eCAL_Subscriber_GetPublisherCount(subscriber));

  eCAL_Publisher_Delete(publisher_1);
  eCAL_Publisher_Delete(publisher_2);
  eCAL_Publisher_Delete(publisher_3);
}

TEST_F(pubsub_test_c, pub_GetSubscriberCount) 
{
  eCAL_Subscriber *subscriber_1;
  eCAL_Subscriber *subscriber_2;

  subscriber_1 = eCAL_Subscriber_New(topic_name, &data_type_information, NULL, NULL);
  EXPECT_NE(nullptr, subscriber_1);
  subscriber_2 = eCAL_Subscriber_New(topic_name, &data_type_information, NULL, NULL);
  EXPECT_NE(nullptr, subscriber_2);

  publisher = eCAL_Publisher_New(topic_name, &data_type_information, NULL, NULL);
  EXPECT_NE(nullptr, publisher);

  eCAL_Process_SleepMS(2000);

  // actual test 2 subscribers + 1 from SetUp()
  EXPECT_EQ(3, eCAL_Publisher_GetSubscriberCount(publisher));
}

TEST_F(pubsub_test_c, pub_SendPayloadWriter) 
{
  eCAL_PayloadWriter writer = {};
  writer.WriteFull = DummyWriteFull;
  writer.GetSize = DummyGetSize;
  
  eCAL_Process_SleepMS(2000);

  EXPECT_EQ(0, eCAL_Publisher_SendPayloadWriter(publisher, &writer, nullptr));
}