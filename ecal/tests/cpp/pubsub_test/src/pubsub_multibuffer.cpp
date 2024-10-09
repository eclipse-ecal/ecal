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

enum {
  CMN_REGISTRATION_REFRESH_MS = 1000,
  DATA_FLOW_TIME_MS = 50,
  PAYLOAD_SIZE_BYTE = 1024
};

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

std::vector<char> multibuffer_pub_sub_test(int buffer_count, bool zero_copy, int publications, int bytes_to_read)
{
  // create payload
  CBinaryPayload binary_payload(PAYLOAD_SIZE_BYTE);

  // create subscriber for topic "A"
  eCAL::CSubscriber sub("A");

  // create publisher config
  eCAL::Publisher::Configuration pub_config;
  // set transport layer
  pub_config.layer.shm.enable = true;
  pub_config.layer.udp.enable = false;
  pub_config.layer.tcp.enable = false;
  // set zero copy mode
  pub_config.layer.shm.zero_copy_mode = zero_copy;
  // set number of memory buffer
  pub_config.layer.shm.memfile_buffer_count = buffer_count;

  // create publisher for topic "A"
  eCAL::CPublisher pub("A", pub_config);

  std::atomic<size_t> received_count{ 0 };
  std::atomic<size_t> received_bytes{ 0 };
  std::vector<char>   received_content;

  // add callback
  auto lambda = [&](const char* /*topic_name_*/, const eCAL::SReceiveCallbackData* data_) {
    received_bytes += data_->size;
    ++received_count;
    for (auto i = 0; i < bytes_to_read; ++i)
    {
      const char rec_char(static_cast<const char*>(data_->buf)[i]);
      received_content.push_back(rec_char);
      std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(rec_char) << " ";
    }
    std::cout << std::endl;
    };
  EXPECT_EQ(true, sub.AddReceiveCallback(lambda));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH_MS);

  // run publications
  for (int i = 0; i < publications; ++i)
  {
    EXPECT_EQ(PAYLOAD_SIZE_BYTE, pub.Send(binary_payload));
    eCAL::Process::SleepMS(DATA_FLOW_TIME_MS);
  }

  return received_content;
}

TEST(core_cpp_pubsub, MultibufferPubSub)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "pubsub_test");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // number of iterations
  const int publications(11);
  const int bytes_to_read(10);

  {
    std::cout << std::endl << "Buffer = 1, Zero Copy Off -> partial writing disabled" << std::endl;
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
    auto received_content = multibuffer_pub_sub_test(1, false, publications, bytes_to_read);
    // check receive content
    // one buffer, no zero copy
    // we expect 11 full writes == 11 * 10 * 42 == 4620
    auto rec_char_sum = std::accumulate(received_content.begin(), received_content.end(), 0);
    EXPECT_EQ(publications * bytes_to_read * 42, rec_char_sum);
  }

  {
    std::cout << std::endl << "Buffer = 2, Zero Copy Off -> partial writing disabled" << std::endl;
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
    auto received_content = multibuffer_pub_sub_test(2, false, publications, bytes_to_read);
    // check receive content
    // two buffer, no zero copy
    // we expect 11 full writes == 11 * 10 * 42 == 4620
    auto rec_char_sum = std::accumulate(received_content.begin(), received_content.end(), 0);
    EXPECT_EQ(publications * bytes_to_read * 42, rec_char_sum);
  }

  {
    std::cout << std::endl << "Buffer = 2, Zero Copy On -> partial writing disabled" << std::endl;
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
    auto received_content = multibuffer_pub_sub_test(2, true, publications, bytes_to_read);
    // check receive content
    // two buffer, zero copy on
    // we expect 11 full writes == 11 * 10 * 42 == 4620
    auto rec_char_sum = std::accumulate(received_content.begin(), received_content.end(), 0);
    EXPECT_EQ(11 * 10 * 42, rec_char_sum);
  }

  {
    std::cout << std::endl << "Buffer = 1, Zero Copy On -> partial writing enabled" << std::endl;
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
    auto received_content = multibuffer_pub_sub_test(1, true, publications, bytes_to_read);
    // check receive content
    // one buffer, zero copy on
    // we expect 1 full write + 10 updates == 2475
    auto rec_char_sum = std::accumulate(received_content.begin(), received_content.end(), 0);
    EXPECT_EQ(2475, rec_char_sum);
  }
  std::cout << std::endl;

  // finalize eCAL API
  eCAL::Finalize();
}
