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
#include <ecal/ecal_payload_writer.h>

#include <atomic>
#include <iostream>
#include <numeric>

#include <gtest/gtest.h>

#define CMN_REGISTRATION_REFRESH   1000
#define DATA_FLOW_TIME               50
#define PAYLOAD_SIZE               1024

// a binary payload object for testing
// full (WriteFull) and partial (WriteModified) writing
class CBinaryPayload : public eCAL::CPayloadWriter
{
public:
  CBinaryPayload(size_t size_) : size(size_) {}

  bool WriteFull(void* buf_, size_t len_) override
  {
    // write complete content to the shared memory file
    if (len_ < size) return false;
    memset(buf_, 42, size);
    return true;
  };

  bool WriteModified(void* buf_, size_t len_) override
  {
    // modify content of the shared memory file
    if (len_ < size) return false;
    const size_t write_idx(clock % len_);
    const char write_chr(clock % 100);
    static_cast<char*>(buf_)[write_idx] = write_chr;
    clock++;
    return true;
  };

  size_t GetSize() override { return size; };

  void ResetClock() { clock = 0; };

private:
  size_t size = 0;
  int    clock = 0;
};

TEST(PubSub, MultibufferPubSub)
{ 
  // create payload
  CBinaryPayload binary_payload(PAYLOAD_SIZE);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create subscriber for topic "A"
  eCAL::CSubscriber sub("A");

  // create publisher for topic "A"
  eCAL::CPublisher pub("A");
  pub.SetLayerMode(eCAL::TLayer::tlayer_all, eCAL::TLayer::smode_off);
  pub.SetLayerMode(eCAL::TLayer::tlayer_shm, eCAL::TLayer::smode_on);

  std::atomic<size_t> received_count{ 0 };
  std::atomic<size_t> received_bytes{ 0 };
  std::vector<char>   received_content;

  // add callback
  auto lambda = [&received_count, &received_bytes, &received_content](const char* /*topic_name_*/, const eCAL::SReceiveCallbackData* data_) {
    received_bytes += data_->size;
    ++received_count;
    for (auto i = 0; i < 10; ++i)
    {
      const char rec_char(static_cast<const char*>(data_->buf)[i]);
      received_content.push_back(rec_char);
      std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(rec_char) << " ";
    }
    std::cout << std::endl;
  };
  EXPECT_EQ(true, sub.AddReceiveCallback(lambda));

  const int iterations(11);
  int rec_char_sum(0);

  //////////////////////////////////////////////////////////
  // one buffer, no zero copy / PARTIAL WRITING DISABLED
  //////////////////////////////////////////////////////////
  // expected output:
  // 42 42 42 42 42 42 42 42 42 42    < full initial write
  // 42 42 42 42 42 42 42 42 42 42    < full write
  // 42 42 42 42 42 42 42 42 42 42    < ..
  // 42 42 42 42 42 42 42 42 42 42
  // 42 42 42 42 42 42 42 42 42 42
  // 42 42 42 42 42 42 42 42 42 42
  // 42 42 42 42 42 42 42 42 42 42
  // 42 42 42 42 42 42 42 42 42 42
  // 42 42 42 42 42 42 42 42 42 42
  // 42 42 42 42 42 42 42 42 42 42
  // 42 42 42 42 42 42 42 42 42 42

  std::cout << std::endl << "Buffer = 1, Zero Copy Off -> partial writing disabled" << std::endl;
  binary_payload.ResetClock();
  received_content.clear();
  pub.ShmSetBufferCount(1);
  pub.ShmEnableZeroCopy(false);

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  // run 11 iterations (11 full writes)
  for (int i = 0; i < iterations; ++i)
  {
    EXPECT_EQ(PAYLOAD_SIZE, pub.Send(binary_payload));
    eCAL::Process::SleepMS(DATA_FLOW_TIME);
  }
  std::cout << std::endl;

  // check receive content
  // one buffer, no zero copy
  // we expect 11 full writes == 11 * 10 * 42 == 4620
  rec_char_sum = std::accumulate(received_content.begin(), received_content.end(), 0);
  EXPECT_EQ(11 * 10 * 42, rec_char_sum);

  //////////////////////////////////////////////////////////
  // two buffer, no zero copy / PARTIAL WRITING DISABLED
  //////////////////////////////////////////////////////////
  // expected output:
  // 42 42 42 42 42 42 42 42 42 42    < full initial write
  // 42 42 42 42 42 42 42 42 42 42    < full write
  // 42 42 42 42 42 42 42 42 42 42    < ..
  // 42 42 42 42 42 42 42 42 42 42
  // 42 42 42 42 42 42 42 42 42 42
  // 42 42 42 42 42 42 42 42 42 42
  // 42 42 42 42 42 42 42 42 42 42
  // 42 42 42 42 42 42 42 42 42 42
  // 42 42 42 42 42 42 42 42 42 42
  // 42 42 42 42 42 42 42 42 42 42
  // 42 42 42 42 42 42 42 42 42 42

  std::cout << std::endl << "Buffer = 2, Zero Copy Off -> partial writing disabled" << std::endl;
  binary_payload.ResetClock();
  received_content.clear();
  pub.ShmSetBufferCount(2);
  pub.ShmEnableZeroCopy(false);

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  // run 11 iterations (11 full writes)
  for (int i = 0; i < iterations; ++i)
  {
    EXPECT_EQ(PAYLOAD_SIZE, pub.Send(binary_payload));
    eCAL::Process::SleepMS(DATA_FLOW_TIME);
  }
  std::cout << std::endl;

  // check receive content
  // two buffer, no zero copy
  // we expect 11 full writes == 11 * 10 * 42 == 4620
  rec_char_sum = std::accumulate(received_content.begin(), received_content.end(), 0);
  EXPECT_EQ(11 * 10 * 42, rec_char_sum);

  //////////////////////////////////////////////////////////
  // two buffer, zero copy on / PARTIAL WRITING DISABLED
  //////////////////////////////////////////////////////////
  // expected output:
  // 42 42 42 42 42 42 42 42 42 42    < full initial write
  // 42 42 42 42 42 42 42 42 42 42    < full write
  // 42 42 42 42 42 42 42 42 42 42    < ..
  // 42 42 42 42 42 42 42 42 42 42
  // 42 42 42 42 42 42 42 42 42 42
  // 42 42 42 42 42 42 42 42 42 42
  // 42 42 42 42 42 42 42 42 42 42
  // 42 42 42 42 42 42 42 42 42 42
  // 42 42 42 42 42 42 42 42 42 42
  // 42 42 42 42 42 42 42 42 42 42
  // 42 42 42 42 42 42 42 42 42 42

  std::cout << std::endl << "Buffer = 2, Zero Copy On -> partial writing disabled" << std::endl;
  binary_payload.ResetClock();
  received_content.clear();
  pub.ShmSetBufferCount(2);
  pub.ShmEnableZeroCopy(true);

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  // run 11 iterations (11 full writes)
  for (int i = 0; i < iterations; ++i)
  {
    EXPECT_EQ(PAYLOAD_SIZE, pub.Send(binary_payload));
    eCAL::Process::SleepMS(DATA_FLOW_TIME);
  }
  std::cout << std::endl;

  // check receive content
  // two buffer, zero copy on
  // we expect 11 full writes == 11 * 10 * 42 == 4620
  rec_char_sum = std::accumulate(received_content.begin(), received_content.end(), 0);
  EXPECT_EQ(11 * 10 * 42, rec_char_sum);

  //////////////////////////////////////////////////////////
  // one buffer, zero copy on / PARTIAL WRITING ENABLED
  //////////////////////////////////////////////////////////
  // expected output:
  // 42 42 42 42 42 42 42 42 42 42    < full initial write
  // 00 42 42 42 42 42 42 42 42 42    < 1. partial write
  // 00 01 42 42 42 42 42 42 42 42    < 2. partial write
  // 00 01 02 42 42 42 42 42 42 42    < ..
  // 00 01 02 03 42 42 42 42 42 42
  // 00 01 02 03 04 42 42 42 42 42
  // 00 01 02 03 04 05 42 42 42 42
  // 00 01 02 03 04 05 06 42 42 42
  // 00 01 02 03 04 05 06 07 42 42
  // 00 01 02 03 04 05 06 07 08 42
  // 00 01 02 03 04 05 06 07 08 09

  std::cout << std::endl << "Buffer = 1, Zero Copy On -> partial writing enabled" << std::endl;
  binary_payload.ResetClock();
  received_content.clear();
  pub.ShmSetBufferCount(1);
  pub.ShmEnableZeroCopy(true);

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  // run 11 iterations (1 full write, 10 updates)
  for (int i = 0; i < iterations; ++i)
  {
    EXPECT_EQ(PAYLOAD_SIZE, pub.Send(binary_payload));
    eCAL::Process::SleepMS(DATA_FLOW_TIME);
  }
  std::cout << std::endl;

  // check receive content
  // one buffer, zero copy on
  // we expect 1 full write + 10 updates == 2475
  rec_char_sum = std::accumulate(received_content.begin(), received_content.end(), 0);
  EXPECT_EQ(2475, rec_char_sum);

  // finalize eCAL API
  eCAL::Finalize();
}
