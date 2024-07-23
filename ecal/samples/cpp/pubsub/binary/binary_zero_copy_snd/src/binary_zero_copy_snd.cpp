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

#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

// a simple struct to demonstrate
// zero copy modifications
struct alignas(4) SSimpleStruct
{
  uint32_t version      = 1;
  uint16_t rows         = 5;
  uint16_t cols         = 3;
  uint32_t clock        = 0;
  uint8_t  bytes[5 * 3] = { 0 };
};

// SSimpleStruct logging
void log_struct(const char* action_name, const SSimpleStruct& s)
{
  std::cout << "------------------------------------" << std::endl;
  std::cout << action_name << std::endl;
  std::cout << "------------------------------------" << std::endl;

  std::cout << "Version : " << s.version << std::endl;
  std::cout << "Rows    : " << s.rows    << std::endl;
  std::cout << "Cols    : " << s.cols    << std::endl;
  std::cout << "Clock   : " << s.clock   << std::endl;

  std::cout << "Bytes   : " << std::endl;
  for (int i = 0; i < s.rows; ++i) {
    for (int j = 0; j < s.cols; ++j) {
      std::cout << static_cast<int>(s.bytes[i * s.cols + j]) << " ";
    }
    std::cout << std::endl;
  }
}

// a binary payload object that handles
// SSimpleStruct WriteFull and WriteModified functionality
class CStructPayload : public eCAL::CPayloadWriter
{
public:
  // Write the complete SSimpleStruct to the shared memory
  bool WriteFull(void* buf_, size_t len_) override
  {
    // check available size and pointer
    if (len_ < GetSize() || buf_ == nullptr) return false;

    // create a new struct and update its content
    SSimpleStruct simple_struct;
    UpdateStruct(&simple_struct);

    // copy complete struct into the memory
    *static_cast<SSimpleStruct*>(buf_) = simple_struct;

    // log action
    log_struct("WriteFull SSimpleStruct :", simple_struct);

    return true;
  };

  // Modify the SSimpleStruct in the shared memory
  bool WriteModified(void* buf_, size_t len_) override
  {
    // check available size and pointer
    if (len_ < GetSize() || buf_ == nullptr) return false;

    // update the struct in memory
    UpdateStruct(static_cast<SSimpleStruct*>(buf_));

    // log action
    log_struct("WriteModified SSimpleStruct :", *static_cast<SSimpleStruct*>(buf_));

    return true;
  };

  size_t GetSize() override { return sizeof(SSimpleStruct); };

private:
  void UpdateStruct(SSimpleStruct* simple_struct)
  {
    // modify the simple_struct
    simple_struct->clock = clock;
    for (auto i = 0; i < (simple_struct->rows * simple_struct->cols); ++i)
    {
      simple_struct->bytes[i] = static_cast<char>(simple_struct->clock);
    }

    // increase internal state clock
    clock++;
  };

  uint32_t clock = 0;
};

int main(int argc, char** argv)
{
  const char* nodeName       = "binary_zero_copy_snd";
  const char* topicName      = "simple_struct";
  const char* structTypeName = "SSimpleStruct";

  // initialize eCAL API
  eCAL::Initialize(argc, argv, nodeName);

  // create publisher config
  eCAL::Publisher::Configuration pub_config;

  // turn zero copy mode on
  pub_config.layer.shm.zero_copy_mode = true;

  // create the publisher
  eCAL::CPublisher pub(topicName, { "custom", structTypeName, "" }, pub_config);

  // create the simple struct payload
  CStructPayload struct_payload;

  // send updates every 100 ms
  while (eCAL::Ok())
  {
    pub.Send(struct_payload);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // finalize eCAL API
  eCAL::Finalize();

  return 0;
}
